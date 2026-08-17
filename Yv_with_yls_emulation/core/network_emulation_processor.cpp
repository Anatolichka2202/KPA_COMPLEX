#include "network_emulation_processor.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <atomic>

namespace YAV {

NetworkEmulationProcessor::NetworkEmulationProcessor(
    std::unique_ptr<Network::IUdpClient> network_client,
    const Config& config)
    : config_(config)
    , network_client_(std::move(network_client)) {
}

bool NetworkEmulationProcessor::initialize() {
    std::cout << "NetworkEmulationProcessor: initialize" << std::endl;

    // Инициализируем сетевой клиент для локального адреса
    if (!network_client_->initialize("127.0.0.15", 1500)) {
        std::cerr << "Failed to initialize network client" << std::endl;
        return false;
    }

    return true;
}

void NetworkEmulationProcessor::shutdown() {
    std::cout << "NetworkEmulationProcessor: shutdown" << std::endl;
    if (network_client_) {
        network_client_->shutdown();
    }
}

void NetworkEmulationProcessor::processRequest(const BKD::Protocol::YVToYLSPacket& request) {
    if (!network_client_) return;

    // Сбрасываем состояние
    pending_requests_.clear();
    received_responses_.clear();

    auto start_time = std::chrono::steady_clock::now();

    // 1. Асинхронная отправка всех запросов
    for (uint8_t block = 0; block < 12; ++block) {
        if (request.isBlockActive(block)) {
            // Получаем запрос для этого блока
            BKD::Protocol::BKDRequest bkd_request = request.getBlockRequest(block);

            // Получаем адрес БКД из конфига
            std::string bkd_ip = config_.getBkdIp(block);
            uint16_t bkd_port = config_.getBkdPort(block);

            // Создаём структуру подключения
            BKDConnection conn;
            conn.ip = bkd_ip;
            conn.port = bkd_port;
            conn.timeout_ms = config_.emulated_bkd.bkd_timeout_ms;

            // Отправляем запрос БКД
            std::vector<uint8_t> request_data(sizeof(bkd_request));
            memcpy(request_data.data(), &bkd_request, sizeof(bkd_request));

            if (network_client_->sendTo(bkd_ip, bkd_port, request_data)) {
                stats_.packets_sent++;

                // Запоминаем отправленный запрос
                PendingRequest pending;
                pending.block = block;
                pending.sent_time = std::chrono::steady_clock::now();
                pending.connection = conn;
                pending_requests_.push_back(pending);

                std::cout << "Sent request to block " << (int)block
                          << " at " << bkd_ip << ":" << bkd_port << std::endl;
            } else {
                stats_.errors++;
                std::cerr << "Failed to send to block " << (int)block << std::endl;
            }
        }
    }

    if (pending_requests_.empty()) {
        return;
    }

    // 2. Асинхронное ожидание ответов с адаптивным таймаутом
    auto timeout = std::chrono::milliseconds(
        std::min(100, config_.emulated_bkd.bkd_timeout_ms * 2) // Увеличиваем таймаут
        );

    // Считаем полученные ответы
    size_t expected_responses = pending_requests_.size();
    size_t received_responses_count = 0;

    auto deadline = start_time + timeout;

    while (std::chrono::steady_clock::now() < deadline &&
           received_responses_count < expected_responses) {

        // Неблокирующее чтение с небольшим таймаутом
        std::string sender_addr;
        uint16_t sender_port;

        // Вычисляем оставшееся время
        auto remaining_time = deadline - std::chrono::steady_clock::now();
        int remaining_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                               remaining_time).count();

        // Устанавливаем разумный таймаут для чтения
        int read_timeout = std::max(1, std::min(remaining_ms, 10));

        auto response_data = network_client_->receiveFrom(
            sender_addr, sender_port, read_timeout);

        if (!response_data.empty() && response_data.size() == sizeof(BKD::Protocol::BKDResponse)) {
            BKD::Protocol::BKDResponse bkd_response;
            memcpy(&bkd_response, response_data.data(), sizeof(bkd_response));

            if (bkd_response.isValid()) {
                uint8_t block = bkd_response.block_number;

                // Находим этот блок в pending_requests_
                auto it = std::find_if(pending_requests_.begin(), pending_requests_.end(),
                                       [block](const PendingRequest& req) {
                                           return req.block == block;
                                       });

                if (it != pending_requests_.end()) {
                    // Проверяем, что ответ пришёл с правильного адреса
                    if (sender_addr == it->connection.ip && sender_port == it->connection.port) {
                        received_responses_[block] = bkd_response;
                        received_responses_count++;
                        stats_.packets_received++;

                        std::cout << "Received response from block " << (int)block << std::endl;

                        // Удаляем из ожидания
                        pending_requests_.erase(it);
                    }
                }
            }
        }

        // Проверяем таймауты для оставшихся запросов
        auto now = std::chrono::steady_clock::now();
        for (auto it = pending_requests_.begin(); it != pending_requests_.end();) {
            if (now - it->sent_time > std::chrono::milliseconds(it->connection.timeout_ms)) {
                std::cerr << "Timeout for block " << (int)it->block << std::endl;
                stats_.errors++;
                it = pending_requests_.erase(it);
            } else {
                ++it;
            }
        }

        // Небольшая пауза
        if (response_data.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    // 3. Формируем общий ответный пакет
    BKD::Protocol::YLSToYVPacket yls_response;
    yls_response.yls_index = 0;
    yls_response.command = BKD::Protocol::CMD_RESPONSE;

    for (const auto& [block, response] : received_responses_) {
        yls_response.setBlockResponse(block, response);
    }

    // 4. Вызываем коллбэк
    if (response_callback_) {
        response_callback_(yls_response);
    }
}

void NetworkEmulationProcessor::setResponseCallback(ResponseCallback callback) {
    response_callback_ = std::move(callback);
}

} // namespace YAV
