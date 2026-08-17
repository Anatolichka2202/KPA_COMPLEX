#include "network.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace YAV {
namespace Network {

// Заглушка для тестов
class StubUdpClient : public IUdpClient {
public:
    StubUdpClient() {
        std::cout << "StubUdpClient created" << std::endl;
    }

    ~StubUdpClient() {
        std::cout << "StubUdpClient destroyed" << std::endl;
    }

    bool initialize(const std::string& local_address, uint16_t local_port) override {
        std::cout << "StubUdpClient: initialize on "
                  << local_address << ":" << local_port << std::endl;
        initialized_ = true;
        return true;
    }

    bool sendTo(const std::string& address, uint16_t port,
                const std::vector<uint8_t>& data) override {
        if (!initialized_) return false;

        std::cout << "StubUdpClient: send " << data.size() << " bytes to "
                  << address << ":" << port << std::endl;

        // Имитация задержки сети
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return true;
    }

    std::vector<uint8_t> receiveFrom(std::string& sender_address,
                                     uint16_t& sender_port,
                                     int timeout_ms = 100) override {
        if (!initialized_) return {};

        std::cout << "StubUdpClient: receiveFrom, timeout=" << timeout_ms << "ms" << std::endl;

        // Имитация получения данных
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        // Возвращаем пустой вектор (в реальности тут будут данные)
        sender_address = "127.0.0.1";
        sender_port = 15000;

        return std::vector<uint8_t>();
    }

    bool isInitialized() const override {
        return initialized_;
    }

    void shutdown() override {
        std::cout << "StubUdpClient: shutdown" << std::endl;
        initialized_ = false;
    }

private:
    bool initialized_ = false;
};

// Реализация фабричных методов
std::unique_ptr<IUdpClient> IUdpClient::createStub() {
    return std::make_unique<StubUdpClient>();
}

std::unique_ptr<IUdpClient> IUdpClient::createQt() {
    // Этот метод будет реализован в GUI части
    // Сейчас возвращаем заглушку
    return std::make_unique<StubUdpClient>();
}

} // namespace Network
} // namespace YAV
