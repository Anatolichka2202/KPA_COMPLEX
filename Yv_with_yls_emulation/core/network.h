#ifndef NETWORK_H
#define NETWORK_H

#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

namespace YAV {
namespace Network {

class IUdpClient {
public:
    virtual ~IUdpClient() = default;

    virtual bool initialize(const std::string& local_address, uint16_t local_port) = 0;
    virtual bool sendTo(const std::string& address, uint16_t port,
                        const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> receiveFrom(std::string& sender_address,
                                             uint16_t& sender_port,
                                             int timeout_ms = 100) = 0;
    virtual bool isInitialized() const = 0;
    virtual void shutdown() = 0;

    // Фабричные методы
    static std::unique_ptr<IUdpClient> createStub();    // Заглушка для тестов
    static std::unique_ptr<IUdpClient> createQt();      // Qt реализация
    // static std::unique_ptr<IUdpClient> createBoost(); // Boost реализация (позже)
};

} // namespace Network
} // namespace YAV
#endif // NETWORK_H
