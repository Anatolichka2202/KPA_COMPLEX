#pragma once

#include <cstdint>
#include <array>
#include <cstddef>

namespace bkd::core {

    // ========== Константы, настраиваемые при сборке ==========
    // Эти значения можно менять перед компиляцией
    inline constexpr int NUM_BLOCKS = 12;               // количество блоков
    inline constexpr int CYCLE_INTERVAL_US = 10000;     // 10 мс = 100 Гц
    inline constexpr int NETWORK_TIMEOUT_US = 8000;     // таймаут ожидания ответа (8 мс)
    inline constexpr bool USE_REAL_YLS = true;          // true – работа с реальным ЯЛС, false – эмуляция
    inline constexpr const char* YLS_IP = "192.168.0.100"; // IP реального ЯЛС (если USE_REAL_YLS)
    inline constexpr uint16_t YLS_PORT = 15000;          // порт ЯЛС

    // ========== Структуры пакетов (из старого Protocol.h) ==========
#pragma pack(push, 1)

    struct BKDRequest {
        uint8_t block_number;          // 0-11
        uint8_t command;                // 1 = запрос
        uint16_t drive_angles[3];       // углы приводов
        uint8_t pyro_mask;              // маска пиро (8 бит)
    };
    static_assert(sizeof(BKDRequest) == 9, "BKDRequest size mismatch");

    struct BKDResponse {
        uint8_t block_number;
        uint8_t command;                 // 2 = ответ
        uint16_t drive_angles[3];
        uint8_t yaz_data[16];
        uint8_t pyro_mask;
        uint8_t ylk_data[16];
        uint8_t yvp_data[54];
    };
    static_assert(sizeof(BKDResponse) == 95, "BKDResponse size mismatch");

    struct YVToYLSPacket {
        uint8_t yls_index;                // индекс активных блоков (0-255)
        uint8_t command;                   // команда для всех блоков (1)
        uint16_t drives[NUM_BLOCKS][3];    // углы для всех блоков
        uint8_t pyro_masks[NUM_BLOCKS];     // маски пиро для всех блоков
        uint8_t reserved[42];               // резерв (можно использовать для временных меток)
    };
    static_assert(sizeof(YVToYLSPacket) == 128, "YVToYLSPacket size mismatch");

    struct YLSToYVPacket {
        uint8_t yls_index;
        uint8_t command;                    // 2
        uint16_t drives[NUM_BLOCKS][3];
        uint8_t yaz_data[192];               // 12*16
        uint8_t pyro_masks[NUM_BLOCKS];
        uint8_t ylk_data[192];                // 12*16
        uint8_t yvp_data[648];                 // 12*54
        uint8_t reserved[7074];                // остаток до 8192
    };
    static_assert(sizeof(YLSToYVPacket) == 8192, "YLSToYVPacket size mismatch");

#pragma pack(pop)

    // ========== Временные метки ==========
    using Timestamp = uint64_t;  // наносекунды от некоторого начала (например, PTP)

    // ========== Команды от GUI к Master ==========
    struct GuiCommand {
        enum Type {
            CMD_NONE,
            SET_PYRO_MASK,          // установить маску пиро для блока
            SET_DRIVE_ANGLES,       // установить углы приводов для блока
            // можно добавить другие
        };
        Type type = CMD_NONE;
        int block = 0;              // номер блока (0-11)
        union {
            uint8_t pyro_mask;
            struct {
                uint16_t angles[3];
            } drive;
        };
    };

    // ========== Данные, передаваемые между потоками ==========
    struct TickData {
        Timestamp tick_time;               // время начала такта
        YVToYLSPacket outgoing;              // отправленный запрос
        YLSToYVPacket incoming;               // полученный ответ (если есть)
        bool response_received = false;      // флаг, что ответ пришёл вовремя
        // Можно добавить статусы блоков (N/A, OK, ERROR)
    };

    // ========== Функции для работы с индексами и масками ==========
    // (можно скопировать из старого Protocol.h)

    inline uint16_t indexToBlockMask(uint8_t idx) {
        static const uint16_t masks[] = {
            0b000000000000,  // 0
            0b000000000001,  // 1
            0b000000000011,  // 2
            0b000000000111,  // 3
            0b000000001111,  // 4
            0b000000011111,  // 5
            0b000000111111,  // 6
            0b000001111111,  // 7
            0b000011111111,  // 8
            0b000111111111,  // 9
            0b001111111111,  // 10
            0b011111111111,  // 11
            0b111111111111   // 12
        };
        return (idx <= 12) ? masks[idx] : 0;
    }

    inline bool isBlockActive(const YVToYLSPacket& pkt, uint8_t block) {
        if (block >= NUM_BLOCKS) return false;
        uint16_t mask = indexToBlockMask(pkt.yls_index);
        return (mask >> block) & 1;
    }

} // namespace bkd::core
