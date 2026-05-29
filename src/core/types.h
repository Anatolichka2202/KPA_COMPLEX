#pragma once

#include <cstdint>
#include <array>
#include <cstddef>

namespace bkd::core {

// ========== Константы, настраиваемые при сборке ==========
// Эти значения можно менять перед компиляцией
inline constexpr int NUM_BLOCKS = 12;               // количество блоков
inline constexpr int CYCLE_INTERVAL_US = 10000;     // 10 мс = 100 Гц
inline constexpr int NETWORK_TIMEOUT_US = 5000;     // таймаут ожидания ответа (5 мс)
inline constexpr bool USE_REAL_YLS = true;          // true – работа с реальным ЯЛС, false – эмуляция
//inline constexpr const char* YLS_IP =  "127.0.0.1";   /*"192.168.17.230";*/ // IP реального ЯЛС
inline constexpr const char* YLS_IP =  "192.168.17.230";   /*"192.168.17.230";*/ // IP реального ЯЛС
inline constexpr uint16_t YLS_PORT = 101;           // порт ЯЛС (из main.c: udp_bind(pcb, IP_ADDR_ANY, 1080))

// ========== Структуры пакетов   ==========
#pragma pack(push, 1)



// Запрос от ЯВ к ЯЛС (152 байта)
typedef struct __attribute__ ((packed)) {
    uint8_t yls_addr;               // 0: адрес ЯЛС (обычно 0x01)
    uint8_t command;                // 1: команда (1 – запрос, 8 – установка)
    uint32_t counter;               // 2-5: счётчик сообщений
    uint8_t yps_bkho_a;             // 6: маска пиро БКХО-А
    uint8_t yps_bkd[12];            // 7-18: маски пиро для 12 БКД
    int16_t ykp[12][2];             // 19-66: углы приводов (12 блоков × 2 угла) – по 2 байта
    uint8_t yaz[12];                // 67-78: команды ЯАЗ (по 1 байту на блок)
}YVToYLSPacket_;

//static_assert(sizeof(YVToYLSPacket) == 152, "YVToYLSPacket size mismatch");

constexpr uint8_t a = 152 - sizeof(YVToYLSPacket_);

typedef struct __attribute__ ((packed))
{
    YVToYLSPacket_ data;
    uint8_t reserve[a];
}YVToYLSPacket; //финальное сообщение


static_assert(sizeof(YVToYLSPacket) == 152, "YVToYLSPacket size mismatch");

// Ответ от ЯЛС к ЯВ (8192 байта)
struct YLSToYVPacket {
    struct {
    uint8_t yls_addr;
    uint8_t command;                // обычно 2
    uint32_t counter;
    uint8_t yps_bkho_a[3];          // 6-8
    uint8_t yps_bkd[12][3];            // 9-44: маски срабатывания (12 блоков × 3 процессора)
    int16_t ykp[12][3][2];          //
    uint8_t yaz[12][3][24];         // 189-1052: данные ЯАЗ
    uint8_t ylk[12][16];            // 1053-1244: данные ЯЛК
    uint8_t yvp[12][54];            // 1245-1892: данные ЯВП
    }data;

    uint8_t reserved[8192 - sizeof(data)];         // 1893-8191
};


static_assert(sizeof(YLSToYVPacket) == 8192, "YLSToYVPacket size mismatch");

// Запрос к подсистеме (БКД) – 11 байт
struct BKDRequest {
    uint8_t block_number;                   // 0-11
    uint8_t command;                        // 1 = запрос
    int16_t drive_angles[4];
    uint8_t pyro_mask;                      // маска пиро (8 бит)
};


static_assert(sizeof(BKDRequest) == 11, "BKDRequest size mismatch");



// Ответ от подсистемы (95 байт)
struct BKDResponse {
    uint8_t block_number;
    uint8_t command;                        // 2 = ответ
    int16_t drive_angles[4];
    uint8_t yaz_data[16];
    uint8_t pyro_mask;
    uint8_t ylk_data[16];
    uint8_t yvp_data[52];
};
static_assert(sizeof(BKDResponse) == 95, "BKDResponse size mismatch");

#pragma pack(pop)

// ========== Вспомогательные функции для масок ==========
inline uint16_t indexToBlockMask(uint8_t idx) {
    static const uint16_t masks[] = {
        0b000000000000,  // 0: none
        0b000000000001,  // 1: block0
        0b000000000011,  // 2: blocks 0,1
        0b000000000111,  // 3: 0-2
        0b000000001111,  // 4: 0-3
        0b000000011111,  // 5: 0-4
        0b000000111111,  // 6: 0-5
        0b000001111111,  // 7: 0-6
        0b000011111111,  // 8: 0-7
        0b000111111111,  // 9: 0-8
        0b001111111111,  // 10: 0-9
        0b011111111111,  // 11: 0-10
        0b111111111111   // 12: all 12 blocks
    };
    return (idx <= 12) ? masks[idx] : 0;
}

inline bool isBlockActive(const YVToYLSPacket& pkt, uint8_t block) {
    if (block >= NUM_BLOCKS) return false;
    uint16_t mask = indexToBlockMask(pkt.data.yls_addr);
    return (mask >> block) & 1;
}

// ========== Временные метки ==========
using Timestamp = uint64_t;  // наносекунды от PTP

// ========== Команды от GUI к Master ==========
struct GuiCommand {
    enum Type {
        CMD_NONE,
        SET_PYRO_MASK,
        SET_DRIVE_ANGLES,
        START_POLLING,
        STOP_POLLING
    };
    Type type = CMD_NONE;
    int block = 0;
    union {
        uint8_t pyro_mask;
        struct { uint16_t angles[4]; } drive;
    };
};

// ========== Данные для GUI и логгера ==========
struct TickData {
    Timestamp tick_time;
    YVToYLSPacket outgoing;
    YLSToYVPacket incoming;
    bool response_received = false;
};



} // namespace bkd::core
