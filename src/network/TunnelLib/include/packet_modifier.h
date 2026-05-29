#pragma once
#include <functional>
#include <atomic>
#include <thread>
#include <string>
#include <cstdint>
#include <vector>

namespace TunnelLib {

// Тип функции-обработчика: получает буфер пакета, его длину,
// возвращает true, если пакет изменён (и false, если не менять).
using PacketCallback = std::function<bool(uint8_t* packetData, uint32_t packetLen)>;

struct DivertConfig {
    std::string filter;          // например "ip.DstAddr == 192.168.17.230 or ip.SrcAddr == 192.168.17.246"
    PacketCallback modifier;     // пользовательская функция модификации
};

class PacketModifier {
public:
    explicit PacketModifier(const DivertConfig& config);
    ~PacketModifier();

    bool start();                // запустить цикл перехвата
    void stop();

private:
    void divertLoop();

    DivertConfig config_;
    std::atomic<bool> running_{false};
    std::thread workerThread_;
    void* divertHandle_ = nullptr;   // HANDLE from WinDivert
};

} // namespace TunnelLib
