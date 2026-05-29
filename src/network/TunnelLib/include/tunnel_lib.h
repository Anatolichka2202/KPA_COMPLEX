#pragma once
#include <string>
#include <functional>
#include <memory>

namespace TunnelLib {

struct TunnelConfig {
    std::string deviceName;   // сетевой интерфейс для pcap (например "\Device\NPF_{...}")
    std::string deviceA_ip;
    uint16_t    deviceA_port;
    std::string deviceB_ip;
    uint16_t    deviceB_port;

    // Функция для модификации пакетов (будет вызвана для каждого пакета между A и B)
    std::function<bool(uint8_t* data, uint32_t len)> packetModifier;
};

class Tunnel {
public:
    explicit Tunnel(const TunnelConfig& config);
    ~Tunnel();

    bool start();
    void stop();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
 std::string getDefaultEthernetInterface();
} // namespace TunnelLib
