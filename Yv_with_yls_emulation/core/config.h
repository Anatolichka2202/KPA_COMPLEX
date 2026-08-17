#ifndef CONFIG_H
#define CONFIG_H

#pragma once

#include <string>
#include <cstdint>
#include <array>

namespace YAV {



struct Config {
    enum class YLSMode {
        FULL_EMULATE,    // Эмуляция ЯЛС(запрос/ответ внутри программы)
        EMULATE_YLS,    // Эмуляция ЯЛС(реальная сеть, разбивка пакетов внутри программы)
        REAL        // Реальный ЯЛС по реальнйо сети
    };

    // Основные настройки
    YLSMode yls_mode = YLSMode::FULL_EMULATE;
    std::string name = "ГУИ_ЯВ-1";
    std::string log_file = "yav_gui.log";

    // Реальный ЯЛС
    struct RealYLS {
        std::string ip = "192.168.0.200";
        uint16_t port = 15000;
        int timeout_ms = 10;
    } real_yls;

    // Эмулированные БКД (12 блоков)
    struct EmulatedBKD {
        std::string ip = "127.0.0.1";
        uint16_t base_port = 15000;  //
        int bkd_timeout_ms = 20;
    } emulated_bkd;

    // Индивидуальные настройки БКД (если заданы, переопределяют базовые)
    struct IndividualBKD {
        bool configured = false;
        std::string ip;
        uint16_t port;
    };
    std::array<IndividualBKD, 12> individual_bkd;

    // Метод для получения IP БКД
    std::string getBkdIp(uint8_t block) const {
        if (block < individual_bkd.size() && individual_bkd[block].configured) {
            return individual_bkd[block].ip;
        }
        return emulated_bkd.ip;
    }

    // Проверка, настроен ли БКД
    bool isBkdConfigured(uint8_t block) const {
        if (block >= individual_bkd.size()) return false;
        return individual_bkd[block].configured;
    }

    // Интервалы (мс)
    struct Timing {
        int send_interval = 20;          // 50 Гц - отправка данных
        int gui_update_interval = 100;   // 10 Гц - обновление виджетов
        int chart_update_interval = 2000;// 0.5 Гц - обновление графиков
    } timing;

    // Логирование
    struct Logging {
        bool enabled = true;
        bool log_first_packets = true;   // Логировать первые 3 пакета
        int max_packets_logged = 3;
    } logging;

    // Методы
    bool isValid() const;
    uint16_t getBkdPort(uint8_t block) const {
        if (block < individual_bkd.size() && individual_bkd[block].configured) {
            return individual_bkd[block].port;
        }
        return emulated_bkd.base_port + block;
    }
};

} // namespace YAV
#endif // CONFIG_H
