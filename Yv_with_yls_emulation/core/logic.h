#ifndef LOGIC_H
#define LOGIC_H

#pragma once

#include "config.h"
#include "Protocol.h"
#include "network.h"
#include <memory>
#include <mutex>
#include <functional>

namespace YAV {

class YVLogic {
public:
    using DataCallback = std::function<void(const BKD::Protocol::YLSToYVPacket&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using StatusCallback = std::function<void(const std::string&)>;

    YVLogic(std::unique_ptr<Network::IUdpClient> network_client = nullptr);
    ~YVLogic();

    // Инициализация (очистка пакетов)
    bool initialize(const Config& config);
    void start();
    void stop();

    // GUI -> Logic (ТОЛЬКО маска пиро!)
    void setPyroMask(uint8_t block, uint8_t pyro_mask);

    // Logic -> GUI (получение данных для отображения)
    BKD::Protocol::YLSToYVPacket getCurrentData() const;

    // Logic -> Network (отправка текущего пакета)
    void sendCurrentPacket();

    // Callbacks
    void setDataCallback(DataCallback callback) { data_callback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { error_callback_ = callback; }
    void setStatusCallback(StatusCallback callback) { status_callback_ = callback; }

private:
    void handleResponse(const BKD::Protocol::YLSToYVPacket& response);
    void generateInitialData();  // Создание начальных углов

    Config config_;
    std::unique_ptr<Network::IUdpClient> network_client_;
    std::unique_ptr<class IDataProcessor> data_processor_;

    // ДВА пакета - всё просто!
    BKD::Protocol::YVToYLSPacket outgoing_packet_;   // Что отправляем
    BKD::Protocol::YLSToYVPacket incoming_packet_;   // Что получили

    mutable std::mutex data_mutex_;
    bool is_running_ = false;

    DataCallback data_callback_;
    ErrorCallback error_callback_;
    StatusCallback status_callback_;
};

} // namespace YAV
#endif // LOGIC_H
