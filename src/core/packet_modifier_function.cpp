// packet_modifier_function.cpp
#include "core/types.h"
#include "core/queues.h"
#include "core/databus.h"
#include <cstring>
#include <chrono>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

#pragma pack(push, 1)
struct EthHeader {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;
};
struct Ipv4Header {
    uint8_t ver_ihl;
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t src_addr;
    uint32_t dst_addr;
};
struct UdpHeader {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
};
#pragma pack(pop)

static bool parsePacket(uint8_t* packet, uint32_t len,
                        uint8_t*& udp_payload, uint32_t& payload_len) {
    if (len < sizeof(EthHeader) + sizeof(Ipv4Header) + sizeof(UdpHeader))
        return false;

    EthHeader* eth = reinterpret_cast<EthHeader*>(packet);
    if (ntohs(eth->type) != 0x0800)   // только IPv4
        return false;

    Ipv4Header* ip = reinterpret_cast<Ipv4Header*>(packet + sizeof(EthHeader));
    uint8_t ip_header_len = (ip->ver_ihl & 0x0F) * 4;
    if (len < sizeof(EthHeader) + ip_header_len + sizeof(UdpHeader))
        return false;
    if (ip->protocol != 17)   // UDP
        return false;

    UdpHeader* udp = reinterpret_cast<UdpHeader*>(packet + sizeof(EthHeader) + ip_header_len);
    uint16_t dst_port = ntohs(udp->dst_port);
    uint16_t src_port = ntohs(udp->src_port);
    if (dst_port != 101 && src_port != 101)
        return false;

    udp_payload = reinterpret_cast<uint8_t*>(udp + 1);
    payload_len = len - (sizeof(EthHeader) + ip_header_len + sizeof(UdpHeader));
    return true;
}

bool packetModifierCallback(uint8_t* packet, uint32_t len) {
    uint8_t* udp_data;
    uint32_t data_len;
    if (!parsePacket(packet, len, udp_data, data_len))
        return false;

    if (data_len == sizeof(bkd::core::YVToYLSPacket)) {
        // Запрос от БЦВМ к ЯЛС
        auto* req = reinterpret_cast<bkd::core::YVToYLSPacket*>(udp_data);
        bool modified = false;

        bkd::core::GuiCommand cmd;
        while (bkd::core::g_guiToMaster.pop(cmd)) {
            switch (cmd.type) {
            case bkd::core::GuiCommand::SET_DRIVE_ANGLES:
                if (cmd.block < bkd::core::NUM_BLOCKS) {
                    req->data.ykp[cmd.block][0] = cmd.drive.angles[0];
                    req->data.ykp[cmd.block][1] = cmd.drive.angles[1];
                    modified = true;
                }
                break;
            case bkd::core::GuiCommand::SET_PYRO_MASK:
                if (cmd.block < bkd::core::NUM_BLOCKS) {
                    req->data.yps_bkd[cmd.block] = cmd.pyro_mask;
                    modified = true;
                }
                break;
            default:
                break;
            }
        }
        if (modified) {
            req->data.command = 8;
        }
        return modified;
    }
    else if (data_len == sizeof(bkd::core::YLSToYVPacket)) {
        // Ответ от ЯЛС к БЦВМ
        auto* resp = reinterpret_cast<bkd::core::YLSToYVPacket*>(udp_data);
        for (int block = 0; block < bkd::core::NUM_BLOCKS; ++block) {
            for (int i = 0; i < bkd::core::ANGLES_PER_BLOCK; ++i) {
                bkd::core::g_dataBus.setAngle(block, i, resp->data.ykp[block][0][i]);
            }
            bkd::core::g_dataBus.setPyroMask(block, resp->data.yps_bkd[block][0]);
        }
        bkd::core::TickData td;
        td.tick_time = std::chrono::duration_cast<std::chrono::microseconds>(
                           std::chrono::steady_clock::now().time_since_epoch()).count();
        td.incoming = *resp;
        td.response_received = true;
        bkd::core::g_masterToGui.push(td);
        return false;
    }
    return false;
}
