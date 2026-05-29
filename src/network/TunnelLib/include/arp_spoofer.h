#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include <cstdint>

namespace TunnelLib {

struct ArpConfig {
    std::string deviceName;     // имя сетевого интерфейса (например, "\Device\NPF_{...}")
    std::string targetA_IP;     // 192.168.17.246
    std::string targetB_IP;     // 192.168.17.230
    std::string spoofedIP_A;    // IP, который мы выдаём за targetA ( IP Б)
    std::string spoofedIP_B;    // IP, который выдаём за targetB ( IP А)
    int intervalMs = 10000;     // интервал отправки поддельных ARP-ответов (мс)
};

class ArpSpoofer {
public:
    explicit ArpSpoofer(const ArpConfig& config);
    ~ArpSpoofer();

    bool start();           // запустить фоновую отправку
    void stop();            // остановить поток

private:
    void spoofingLoop();

    // Вспомогательные функции с использованием pcap
    bool initPcap();
    void sendArpReply(const std::string& targetIP, const std::string& senderIP,
                      const uint8_t targetMAC[6], const uint8_t senderMAC[6]);

    ArpConfig config_;
    std::atomic<bool> running_{false};
    std::thread workerThread_;

    // pcap handle
    void* pcapHandle_ = nullptr;
    uint8_t myMAC_[6] = {0};
    uint8_t targetA_MAC_[6] = {0};
    uint8_t targetB_MAC_[6] = {0};
};

} // namespace TunnelLib
