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

#include "tunnel_lib.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#undef min
#undef max

#include <iphlpapi.h>
#include <pcap.h>

#include "arp_spoofer.h"
#include "packet_modifier.h"
#include <string>
#include <thread>
#include <atomic>
#include <iostream>
namespace TunnelLib {

// Fallback для констант, если не определены
#ifndef PCAP_IF_LOOPBACK
#define PCAP_IF_LOOPBACK 0x01
#endif
#ifndef PCAP_IF_ETHERNET
#define PCAP_IF_ETHERNET 0x02
#endif

struct Tunnel::Impl {
    TunnelConfig config;
    std::unique_ptr<ArpSpoofer> arp;
    std::unique_ptr<PacketModifier> modifier;
    std::atomic<bool> running{false};

    Impl(const TunnelConfig& cfg) : config(cfg) {}

    bool start() {
        ArpConfig arpCfg;
        arpCfg.deviceName = config.deviceName;
        arpCfg.targetA_IP = config.deviceA_ip;
        arpCfg.targetB_IP = config.deviceB_ip;
        arpCfg.spoofedIP_A = config.deviceB_ip;
        arpCfg.spoofedIP_B = config.deviceA_ip;
        arpCfg.intervalMs = 5000;

        arp = std::make_unique<ArpSpoofer>(arpCfg);
        if (!arp->start()) {
            std::cerr << "Failed to start ARP spoofer" << std::endl;
            return false;
        }

        std::string filter = "ip.DstAddr == " + config.deviceA_ip +
                             " or ip.SrcAddr == " + config.deviceA_ip +
                             " or ip.DstAddr == " + config.deviceB_ip +
                             " or ip.SrcAddr == " + config.deviceB_ip;
        DivertConfig divCfg;
        divCfg.filter = filter;
        divCfg.modifier = config.packetModifier;

        modifier = std::make_unique<PacketModifier>(divCfg);
        if (!modifier->start()) {
            std::cerr << "Failed to start packet modifier" << std::endl;
            arp->stop();
            return false;
        }

        running = true;
        return true;
    }

    void stop() {
        if (arp) arp->stop();
        if (modifier) modifier->stop();
        running = false;
    }
};

Tunnel::Tunnel(const TunnelConfig& config)
    : pImpl(std::make_unique<Impl>(config)) {}

Tunnel::~Tunnel() = default;

bool Tunnel::start() { return pImpl->start(); }
void Tunnel::stop() { pImpl->stop(); }

std::string getDefaultEthernetInterface() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t* alldevs;
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        std::cerr << "pcap_findalldevs failed: " << errbuf << std::endl;
        return "";
    }

    std::string selectedDevice;
    for (pcap_if_t* dev = alldevs; dev != nullptr; dev = dev->next) {
        if (dev->flags & PCAP_IF_LOOPBACK) continue;
        if (!(dev->flags & PCAP_IF_ETHERNET)) continue;
        bool hasIPv4 = false;
        for (pcap_addr_t* addr = dev->addresses; addr != nullptr; addr = addr->next) {
            if (addr->addr && addr->addr->sa_family == AF_INET) {
                hasIPv4 = true;
                break;
            }
        }
        if (!hasIPv4) continue;
        selectedDevice = dev->name;
        break;
    }

    pcap_freealldevs(alldevs);
    if (selectedDevice.empty()) {
        std::cerr << "No suitable Ethernet interface found" << std::endl;
    }
    return selectedDevice;
}

} // namespace TunnelLib
