#ifndef YLSEMULATOR_H
#define YLSEMULATOR_H

#pragma once

#include "config.h"
#include "Protocol.h"
#include <memory>
#include <vector>
#include <functional>

namespace YAV {

// Форвард-декларация
namespace Network {
class IUdpClient;
}

class YLSEmulator {
public:
    using ResponseCallback = std::function<void(const BKD::Protocol::YLSToYVPacket&)>;

    YLSEmulator(std::unique_ptr<Network::IUdpClient> network_client, const Config& config);
    ~YLSEmulator();

    // Основной метод обработки
    void processPacket(const BKD::Protocol::YVToYLSPacket& request_packet);

    // Установить callback для ответов
    void setResponseCallback(ResponseCallback callback);

    // Обновить конфигурацию
    void updateConfig(const Config& config);

    // Получить статистику
    struct Stats {
        uint64_t packets_sent = 0;
        uint64_t packets_received = 0;
        uint64_t errors = 0;
    };

    Stats getStats() const { return stats_; }

private:
private:
    // Обработка в режиме полной эмуляции (FULL_EMULATE)
    void processFullEmulation(const BKD::Protocol::YVToYLSPacket& request);

    // Обработка в режиме эмуляции ЯЛС с реальными БКД (EMULATE_YLS)
    void processEmulationWithRealBKD(const BKD::Protocol::YVToYLSPacket& request);

    // Обработка в реальном режиме (REAL)
    void processReal(const BKD::Protocol::YVToYLSPacket& request);

    // Отправка запроса блоку БКД (используется в EMULATE_YLS)
    BKD::Protocol::BKDResponse sendToBKD(uint8_t block_num,
                                         const BKD::Protocol::BKDRequest& request);

    // Генерация случайного ответа для FULL_EMULATE
    BKD::Protocol::BKDResponse generateFakeBKDResponse(uint8_t block_num,
                                                       const BKD::Protocol::BKDRequest& request);
    Config config_;
    std::unique_ptr<Network::IUdpClient> network_client_;
    ResponseCallback response_callback_;
    Stats stats_;
};

} // namespace YAV
#endif // YLSEMULATOR_H
