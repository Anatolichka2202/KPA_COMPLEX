#include "real_network_processor.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace YAV {

RealNetworkProcessor::RealNetworkProcessor(
    std::unique_ptr<Network::IUdpClient> network_client,
    const Config& config)
    : config_(config)
    , network_client_(std::move(network_client)) {
}

bool RealNetworkProcessor::initialize() {
    std::cout << "RealNetworkProcessor: initialize" << std::endl;

    // Инициализируем для отправки на ЯЛС
    if (!network_client_->initialize("127.0.0.1", 15002)) {
        std::cerr << "Failed to initialize network client" << std::endl;
        return false;
    }

    return true;
}

void RealNetworkProcessor::shutdown() {
    std::cout << "RealNetworkProcessor: shutdown" << std::endl;
    if (network_client_) {
        network_client_->shutdown();
    }
}

void RealNetworkProcessor::processRequest(const BKD::Protocol::YVToYLSPacket& request) {
    if (!network_client_) return;

    // 1. Отправляем весь пакет на ЯЛС
    std::vector<uint8_t> request_data(sizeof(request));
    memcpy(request_data.data(), &request, sizeof(request));

    if (!network_client_->sendTo(config_.real_yls.ip,
                                 config_.real_yls.port,
                                 request_data)) {
        std::cerr << "Failed to send to YLS at "
                  << config_.real_yls.ip << ":" << config_.real_yls.port << std::endl;
        stats_.errors++;
        return;
    }

    stats_.packets_sent++;
    std::cout << "Sent to YLS: " << config_.real_yls.ip << ":" << config_.real_yls.port << std::endl;

    // 2. Ждём ответ от ЯЛС
    std::string sender_addr;
    uint16_t sender_port;
    auto response_data = network_client_->receiveFrom(sender_addr, sender_port,
                                                      config_.real_yls.timeout_ms);

    if (!response_data.empty() && response_data.size() == sizeof(BKD::Protocol::YLSToYVPacket)) {
        BKD::Protocol::YLSToYVPacket yls_response;
        memcpy(&yls_response, response_data.data(), sizeof(yls_response));

        stats_.packets_received++;

        // 3. Вызываем коллбэк
        if (response_callback_) {
            response_callback_(yls_response);
        }
    } else {
        std::cerr << "No response from YLS or invalid size" << std::endl;
        stats_.errors++;
    }
}

void RealNetworkProcessor::setResponseCallback(ResponseCallback callback) {
    response_callback_ = std::move(callback);
}

} // namespace YAV
