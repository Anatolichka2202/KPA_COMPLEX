#include "logic.h"
#include "idata_processor.h"
#include <iostream>
#include <cstring>

namespace YAV {

YVLogic::YVLogic(std::unique_ptr<Network::IUdpClient> network_client)
    : network_client_(std::move(network_client)) {
    std::cout << "YVLogic created" << std::endl;

    // НАЧАЛЬНАЯ ИНИЦИАЛИЗАЦИЯ ПАКЕТОВ
    memset(&outgoing_packet_, 0, sizeof(outgoing_packet_));
    memset(&incoming_packet_, 0, sizeof(incoming_packet_));

    outgoing_packet_.command = BKD::Protocol::CMD_REQUEST;
    incoming_packet_.command = BKD::Protocol::CMD_RESPONSE;

    // Генерация начальных углов
    generateInitialData();
}

YVLogic::~YVLogic() {
    stop();
    std::cout << "YVLogic destroyed" << std::endl;
}

void YVLogic::generateInitialData() {
    std::lock_guard<std::mutex> lock(data_mutex_);

    for (int block = 0; block < BKD::Protocol::NUM_BLOCKS; ++block) {
        outgoing_packet_.drives[block][0] = 1000 * (block + 1);
        outgoing_packet_.drives[block][1] = 2000 * (block + 1);
        outgoing_packet_.drives[block][2] = 3000 * (block + 1);
        outgoing_packet_.pyro_masks[block] = 0;  // Все пиро выключены
    }

    // Устанавливаем индекс для ВСЕХ блоков
    outgoing_packet_.yls_index = 12;  // Все 12 блоков
}

bool YVLogic::initialize(const Config& config) {
    config_ = config;

    data_processor_ = createDataProcessor(config_.yls_mode,
                                         std::move(network_client_),
                                         config_);

    if (!data_processor_->initialize()) {
        if (error_callback_) error_callback_("Failed to initialize processor");
        return false;
    }

    data_processor_->setResponseCallback([this](const BKD::Protocol::YLSToYVPacket& response) {
        handleResponse(response);
    });

    if (status_callback_) status_callback_("Initialized");
    return true;
}

void YVLogic::start() {
    is_running_ = true;
    if (status_callback_) status_callback_("Started");
}

void YVLogic::stop() {
    is_running_ = false;
    if (status_callback_) status_callback_("Stopped");
}

void YVLogic::setPyroMask(uint8_t block, uint8_t pyro_mask) {
    std::lock_guard<std::mutex> lock(data_mutex_);

    if (block < BKD::Protocol::NUM_BLOCKS) {
        outgoing_packet_.pyro_masks[block] = pyro_mask;
        std::cout << "Logic: Block " << (int)block << " pyro mask set to 0x"
                  << std::hex << (int)pyro_mask << std::dec << std::endl;
    }
}

BKD::Protocol::YLSToYVPacket YVLogic::getCurrentData() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return incoming_packet_;
}

void YVLogic::sendCurrentPacket() {
    if (!is_running_ || !data_processor_) return;

    BKD::Protocol::YVToYLSPacket packet_to_send;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        packet_to_send = outgoing_packet_;
    }

    data_processor_->processRequest(packet_to_send);
}

void YVLogic::handleResponse(const BKD::Protocol::YLSToYVPacket& response) {
    {
        std::lock_guard<std::mutex> lock(data_mutex_);

        // 1. Сохраняем ответ для GUI
        incoming_packet_ = response;
        //                  ЛОГИРОВАНИ          //
        std::cout << "Logic: Response received, yls_index="
                  << (int)response.yls_index << std::endl;

        for (int block = 0; block < BKD::Protocol::NUM_BLOCKS; block++) {
            if (response.getBlockMask() & (1 << block)) {
                std::cout << "  Block " << block
                          << ": angles=["
                          << response.drives[block][0] << ","
                          << response.drives[block][1] << ","
                          << response.drives[block][2] << "]"
                          << ", pyro=0x" << std::hex
                          << (int)response.pyro_masks[block] << std::dec
                          << std::endl;
            }
        }

        // //////////////////////////////////////
        // 2. Обновляем углы в outgoing_packet_ (для следующей отправки) // Маски пиро НЕ трогаем - они управляются пользователем
        uint16_t active_mask = response.getBlockMask();

        for (int block = 0; block < BKD::Protocol::NUM_BLOCKS; ++block) {
            if (active_mask & (1 << block)) {
                outgoing_packet_.drives[block][0] = response.drives[block][0];
                outgoing_packet_.drives[block][1] = response.drives[block][1];
                outgoing_packet_.drives[block][2] = response.drives[block][2];
                // Маску пиро НЕ обновляем!
            }
        }

        // 3. Обновляем индекс
        outgoing_packet_.yls_index = response.yls_index;
    }

    // 4. Уведомляем GUI
    if (data_callback_) {
        data_callback_(response);
    }
}

} // namespace YAV
