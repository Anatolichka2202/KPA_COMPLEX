// full_emulation_processor.cpp
#include "full_emulation_processor.h"
#include <iostream>
#include <cstring>

namespace YAV {

FullEmulationProcessor::FullEmulationProcessor(const Config& config) {
    std::cout << "FullEmulationProcessor created" << std::endl;
}

FullEmulationProcessor::~FullEmulationProcessor() {
    std::cout << "FullEmulationProcessor destroyed" << std::endl;
}

bool FullEmulationProcessor::initialize() {
    std::cout << "FullEmulationProcessor initialized" << std::endl;
    return true;
}

void FullEmulationProcessor::setResponseCallback(ResponseCallback callback) {
    response_callback_ = callback;
}

BKD::Protocol::BKDResponse FullEmulationProcessor::emulateBKD(const BKD::Protocol::BKDRequest& request) {
    BKD::Protocol::BKDResponse response;
    memset(&response, 0, sizeof(response));

    response.block_number = request.block_number;
    response.command = BKD::Protocol::CMD_RESPONSE;

    // Эмуляция: увеличиваем углы на 100
    response.drive_angles[0] = request.drive_angles[0] + 100;
    response.drive_angles[1] = request.drive_angles[1] + 100;
    response.drive_angles[2] = request.drive_angles[2] + 100;

    // Маску пиро возвращаем как получили (или можно инвертировать для теста)
    response.pyro_mask = request.pyro_mask;

    return response;
}

void FullEmulationProcessor::processRequest(const BKD::Protocol::YVToYLSPacket& request) {
    packet_counter_++;

    // Создаем ответный пакет
    BKD::Protocol::YLSToYVPacket response;
    memset(&response, 0, sizeof(response));

    response.yls_index = request.yls_index;
    response.command = BKD::Protocol::CMD_RESPONSE;

    uint16_t active_mask = request.getBlockMask();
    int processed_blocks = 0;

    // Обрабатываем каждый активный блок
    for (int block = 0; block < BKD::Protocol::NUM_BLOCKS; ++block) {
        if (active_mask & (1 << block)) {
            processed_blocks++;

            // Создаем BKDRequest для этого блока
            BKD::Protocol::BKDRequest bkd_request;
            bkd_request.block_number = block;
            bkd_request.command = request.command;
            bkd_request.drive_angles[0] = request.drives[block][0];
            bkd_request.drive_angles[1] = request.drives[block][1];
            bkd_request.drive_angles[2] = request.drives[block][2];
            bkd_request.pyro_mask = request.pyro_masks[block];

            // Эмулируем ответ БКД
            BKD::Protocol::BKDResponse bkd_response = emulateBKD(bkd_request);

            // Записываем ответ в общий пакет
            response.setBlockResponse(block, bkd_response);

            // Логируем каждые 100 пакетов
            if (packet_counter_ % 100 == 0) {
                std::cout << "  Block " << block
                          << ": request=" << bkd_request.drive_angles[0]
                          << "," << bkd_request.drive_angles[1]
                          << "," << bkd_request.drive_angles[2]
                          << ", pyro=0x" << std::hex << (int)bkd_request.pyro_mask << std::dec
                          << " -> response=" << bkd_response.drive_angles[0]
                          << "," << bkd_response.drive_angles[1]
                          << "," << bkd_response.drive_angles[2]
                          << ", pyro=0x" << std::hex << (int)bkd_response.pyro_mask << std::dec
                          << std::endl;
            }
        }
    }

    // Логируем каждые 100 пакетов
    if (packet_counter_ % 100 == 0) {
        std::cout << "=== EMULATION PACKET #" << packet_counter_ << " ===" << std::endl;
        std::cout << "Processed blocks: " << processed_blocks << std::endl;
        std::cout << "=================================" << std::endl;
    }

    // Отправляем ответ
    if (response_callback_) {
        response_callback_(response);
    }
}

FullEmulationProcessor::Stats FullEmulationProcessor::getStats() const {
    Stats stats;
    stats.packets_sent = packet_counter_;
    stats.packets_received = packet_counter_; // Эмулятор получает и отправляет столько же
    stats.errors = 0;
    return stats;
}

// И shutdown() тоже должен быть реализован:
void FullEmulationProcessor::shutdown() {
    // Просто очищаем callback
    response_callback_ = nullptr;
}

} // namespace YAV
