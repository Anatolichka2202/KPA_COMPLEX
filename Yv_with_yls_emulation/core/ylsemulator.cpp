#include "ylsemulator.h"

#include "network.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

#include <random>
#include <algorithm>

namespace YAV {

YLSEmulator::YLSEmulator(std::unique_ptr<Network::IUdpClient> network_client,
                         const Config& config)
    : config_(config)
    , network_client_(std::move(network_client)) {
}

YLSEmulator::~YLSEmulator() = default;

void YLSEmulator::setResponseCallback(ResponseCallback callback) {
    response_callback_ = std::move(callback);
}

void YLSEmulator::updateConfig(const Config& config) {
    config_ = config;
}

void YLSEmulator::processPacket(const BKD::Protocol::YVToYLSPacket& request_packet) {
    switch (config_.yls_mode) {
    case Config::YLSMode::FULL_EMULATE:
        processFullEmulation(request_packet);
        break;
    case Config::YLSMode::EMULATE_YLS:
        processEmulationWithRealBKD(request_packet);
        break;
    case Config::YLSMode::REAL:
        processReal(request_packet);
        break;
    }
}

void YLSEmulator::processFullEmulation(const BKD::Protocol::YVToYLSPacket& request) {
    BKD::Protocol::YLSToYVPacket response_packet;
    response_packet.address_mask = 0;
    response_packet.command = BKD::Protocol::CMD_RESPONSE;

    // Проходим по всем активным блокам
    for (uint8_t block = 0; block < BKD::Protocol::NUM_BLOCKS; ++block) {
        if (request.isBlockActive(block)) {
            // Получаем запрос для этого блока
            BKD::Protocol::BKDRequest block_request = request.getBlockRequest(block);

            // Генерируем фейковый ответ
            BKD::Protocol::BKDResponse block_response = generateFakeBKDResponse(block, block_request);

            if (block_response.isValid()) {
                // Добавляем ответ в общий пакет
                response_packet.setBlockResponse(block, block_response);
                stats_.packets_received++;
            } else {
                stats_.errors++;
            }
        }
    }

    // Вызываем callback
    if (response_callback_) {
        response_callback_(response_packet);
    }
}

void YLSEmulator::processEmulationWithRealBKD(const BKD::Protocol::YVToYLSPacket& request) {
    BKD::Protocol::YLSToYVPacket response_packet;
    response_packet.address_mask = 0; // Начнем с пустой маски
    response_packet.command = BKD::Protocol::CMD_RESPONSE;

    // Проходим по всем блокам
    for (uint8_t block = 0; block < BKD::Protocol::NUM_BLOCKS; ++block) {
        if (request.isBlockActive(block)) {
            // Получаем запрос для этого блока
            BKD::Protocol::BKDRequest block_request = request.getBlockRequest(block);

            // Отправляем запрос БКД
            BKD::Protocol::BKDResponse block_response = sendToBKD(block, block_request);

            if (block_response.isValid()) {
                // Добавляем ответ в общий пакет
                response_packet.setBlockResponse(block, block_response);
                stats_.packets_received++;
            } else {
                stats_.errors++;
            }
        }
    }

    // Вызываем callback с собранным ответом
    if (response_callback_) {
        response_callback_(response_packet);
    }
}

void YLSEmulator::processReal(const BKD::Protocol::YVToYLSPacket& request) {
    // В реальном режиме просто отправляем весь пакет на ЯЛС

    // Преобразуем пакет в сырые данные
    std::vector<uint8_t> request_data(sizeof(request));
    std::memcpy(request_data.data(), &request, sizeof(request));

    // Отправляем на ЯЛС
    if (network_client_->sendTo(config_.real_yls.ip,
                                config_.real_yls.port,
                                request_data)) {
        stats_.packets_sent++;

        // Получаем ответ (используем таймаут ЯЛС)
        std::string sender_ip;
        uint16_t sender_port;
        std::vector<uint8_t> response_data = network_client_->receiveFrom(
            sender_ip, sender_port, config_.real_yls.timeout_ms);

        if (response_data.size() == sizeof(BKD::Protocol::YLSToYVPacket)) {
            BKD::Protocol::YLSToYVPacket response_packet;
            std::memcpy(&response_packet, response_data.data(), sizeof(response_packet));

            if (response_callback_) {
                response_callback_(response_packet);
            }
            stats_.packets_received++;
        } else {
            stats_.errors++;
        }
    } else {
        stats_.errors++;
    }
}


BKD::Protocol::BKDResponse YLSEmulator::sendToBKD(uint8_t block_num,
                                                 const BKD::Protocol::BKDRequest& request) {
    // Проверяем, что сетевой клиент инициализирован
    if (!network_client_ || !network_client_->isInitialized()) {
       std::cerr << "YLSEmulator: сетевой клиент не инициализирован";
        return BKD::Protocol::BKDResponse{};
    }

    // Формируем данные для отправки
    std::vector<uint8_t> request_data(sizeof(request));
    std::memcpy(request_data.data(), &request, sizeof(request));

    // Получаем адрес БКД из конфига (ИСПРАВЛЕНО)
    std::string bkd_ip = config_.getBkdIp(block_num);  // ← ИСПРАВЛЕНО
    uint16_t bkd_port = config_.getBkdPort(block_num);

    // Логирование
    if (stats_.packets_sent < 3 || stats_.packets_sent % 30 == 0) {
       std::cout<< "YLSEmulator: отправка БКД" << block_num
                 << "на" << bkd_ip << ":" << bkd_port
                 << "размер:" << request_data.size() << "байт";
    }

    // Отправляем запрос
    if (network_client_->sendTo(bkd_ip, bkd_port, request_data)) {
        stats_.packets_sent++;

        // Ждем ответ с таймаутом (ИСПРАВЛЕНО)
        std::string sender_ip;
        uint16_t sender_port;

        // Таймаут для БКД берем из конфига
        int bkd_timeout = config_.emulated_bkd.bkd_timeout_ms;  // ← ИСПРАВЛЕНО

        std::vector<uint8_t> response_data = network_client_->receiveFrom(
            sender_ip, sender_port, bkd_timeout);  // ← ИСПРАВЛЕНО

        if (response_data.size() == sizeof(BKD::Protocol::BKDResponse)) {
            BKD::Protocol::BKDResponse response;
            std::memcpy(&response, response_data.data(), sizeof(response));

            if (response.isValid() && response.block_number == block_num) {
                stats_.packets_received++;

                if (stats_.packets_received < 3 || stats_.packets_received % 30 == 0) {
                    std::cout<< "YLSEmulator: получен ответ от БКД" << block_num
                             << "размер:" << response_data.size() << "байт"
                             << "от:" << sender_ip<< ":" << sender_port;
                }
                return response;
            } else {
               std::cerr<< "YLSEmulator: невалидный ответ от БКД" << block_num;
            }
        } else if (response_data.empty()) {
            std::cout<< "YLSEmulator: таймаут от БКД" << block_num;
        } else {
            std::cerr<< "YLSEmulator: неверный размер ответа от БКД" << block_num
                       << "ожидалось:" << sizeof(BKD::Protocol::BKDResponse)
                       << "получено:" << response_data.size();
        }
    } else {
       std::cerr<< "YLSEmulator: ошибка отправки БКД" << block_num;
    }

    stats_.errors++;
    return BKD::Protocol::BKDResponse{};
}

BKD::Protocol::BKDResponse YLSEmulator::generateFakeBKDResponse(
    uint8_t block_num,
    const BKD::Protocol::BKDRequest& request) {

    BKD::Protocol::BKDResponse response;

    // Базовые поля
    response.block_number = block_num;
    response.command = BKD::Protocol::CMD_RESPONSE;

    // Углы: добавляем старший бит (инвертируем?)
    // Ты сказал "добавляем старший бит" - я сделаю XOR с 0x8000 (старший бит)
    for (int drive = 0; drive < 3; drive++) {
        uint16_t input_angle = request.getDriveAngle(drive);
        uint16_t output_angle = input_angle ^ 0x8000; // Инвертируем старший бит
        response.setDriveAngle(drive, output_angle);
    }

    // Маска пиро: с вероятностью 50% оставляем как есть, 50% - изменяем
    uint8_t input_mask = request.pyro_mask;
    uint8_t output_mask = input_mask;

    // Простая "случайность" на основе времени
    static std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count());
    static std::uniform_int_distribution<int> dist(0, 1);

    if (dist(rng) == 0) { // 50% вероятность
        // Изменяем маску - инвертируем младшие 4 бита
        output_mask = input_mask ^ 0x0F;
    }

    response.pyro_mask = output_mask;

    // Генерируем случайные данные для подсистем
    std::generate(std::begin(response.yaz_data), std::end(response.yaz_data), []() {
        return rand() % 256;
    });

    std::generate(std::begin(response.ylk_data), std::end(response.ylk_data), []() {
        return rand() % 256;
    });

    std::generate(std::begin(response.yvp_data), std::end(response.yvp_data), []() {
        return rand() % 256;
    });

    return response;
}

} // namespace YAV
