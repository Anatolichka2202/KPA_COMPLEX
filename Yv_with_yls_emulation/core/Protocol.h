#ifndef PROTOCOL_H
#define PROTOCOL_H
// /core/include/bkd/protocol/unified_protocol.h
#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <optional>
#include <cstring>
#include <algorithm>

namespace BKD {
namespace Protocol {

constexpr std::array<uint16_t, 256> INDEX_TO_BLOCK_MASK = {
    // Первые 12 индексов
    0b000000000000,  // 0: Ничего (тест)
    0b000000000001,  // 1: Только блок 0
    0b000000000011,  // 2: Блоки 0 и 1
    0b000000000111,  // 3: Блоки 0,1,2
    0b000000001111,  // 4: Блоки 0-3
    0b000000011111,  // 5: Блоки 0-4
    0b000000111111,  // 6: Блоки 0-5
    0b000001111111,  // 7: Блоки 0-6
    0b000011111111,  // 8: Блоки 0-7
    0b000111111111,  // 9: Блоки 0-8
    0b001111111111,  // 10: Блоки 0-9
    0b011111111111,  // 11: Блоки 0-10
    0b111111111111,  // 12: Все 12 блоков
    // Остальные индексы (13-255) можно использовать
};





// ==================== БАЗОВЫЕ КОНСТАНТЫ ====================
constexpr int NUM_BLOCKS = 12;              //12 блоков управления
constexpr int NUM_DRIVES_PER_BLOCK = 3;     // по 3 привода на блок
constexpr int NUM_PYRO_PER_BLOCK = 8;       // по 8 пиросредств на блок

constexpr size_t BKD_REQUEST_SIZE = 9;      // ЯЛС→БКД
constexpr size_t BKD_RESPONSE_SIZE = 95;    // БКД→ЯЛС
constexpr size_t YV_TO_YLS_SIZE = 128;      // ЯВ→ЯЛС
constexpr size_t YLS_TO_YV_SIZE = 8192;     // ЯЛС→ЯВ

// Команды
enum Command : uint8_t {
    CMD_REQUEST = 1,    // Запрос от ЯВ к БКД
    CMD_RESPONSE = 2,   // Ответ от БКД к ЯВ
    // ... другие команды по мере необходимости
};


// Функции для работы с индексом
inline uint16_t getBlockMask(uint8_t index) {
    return (index < INDEX_TO_BLOCK_MASK.size()) ? INDEX_TO_BLOCK_MASK[index] : 0;
}

inline bool isBlockActiveInIndex(uint8_t index, uint8_t block) {
    return block < NUM_BLOCKS && ((getBlockMask(index) >> block) & 1);
}

inline uint8_t findIndexForMask(uint16_t mask) {
    // Простой поиск по таблице
    for (uint8_t i = 0; i < INDEX_TO_BLOCK_MASK.size(); ++i) {
        if (INDEX_TO_BLOCK_MASK[i] == mask) {
            return i;
        }
    }
    // Если не нашли - возвращаем индекс для всех блоков (12)
    return 12;
}

// ==================== ПАКЕТ ЯЛС→БКД (9 байт) ====================
#pragma pack(push, 1)
struct BKDRequest {
    uint8_t block_number;           // [0] Номер блока (0-11)
    uint8_t command;                // [1] Команда (CMD_REQUEST)
    uint16_t drive_angles[3];       // [2-7] Углы 3 приводов
    uint8_t pyro_mask;              // [8] Маска пиро

    bool isValid() const {
        return block_number < NUM_BLOCKS &&
               command == CMD_REQUEST;
    }

    //                      МЕТОДЫ

    uint16_t getDriveAngle(int drive) const {
        if (drive >= 0 && drive < NUM_DRIVES_PER_BLOCK) {
            return drive_angles[drive];
        }
        return 0;
    }

    void setDriveAngle( int drive, uint16_t angle) {
        if (drive >= 0 && drive < NUM_DRIVES_PER_BLOCK) {
            drive_angles[drive] = angle;
        }
    }

    bool getPyroStatus(int pyro_index) const {
        if (pyro_index >= 0 && pyro_index < NUM_PYRO_PER_BLOCK) {
            return (pyro_mask >> pyro_index) & 1;
        }
        return false;
    }

    void setPyroStatus(int pyro_index, bool status) {
        if (pyro_index >= 0 && pyro_index < NUM_PYRO_PER_BLOCK) {
            if (status) {
                pyro_mask |= (1 << pyro_index);
            }
            else {
                pyro_mask &= ~(1 << pyro_index);
            }
        }
    }

};
#pragma pack(pop)
static_assert(sizeof(BKDRequest) == BKD_REQUEST_SIZE, "BKDRequest size mismatch");

// ==================== ПАКЕТ БКД→ЯЛС (95 байт) ====================
#pragma pack(push, 1)
struct BKDResponse {
    uint8_t block_number;           // [0] Номер блока
    uint8_t command;                // [1] Команда (CMD_RESPONSE)
    uint16_t drive_angles[3];       // [2-7] Углы приводов
    uint8_t yaz_data[16];           // [8-23] Данные ЯАЗ
    uint8_t pyro_mask;              // [24] Маска пиро
    uint8_t ylk_data[16];           // [25-40] Данные ЯЛК
    uint8_t yvp_data[54];           // [41-94] Данные ЯВП

    bool isValid() const {
        return block_number < NUM_BLOCKS &&
               command == CMD_RESPONSE;
    }

    //                      МЕТОДЫ

    uint16_t getDriveAngle(int drive) const {
        if (drive >= 0 && drive < NUM_DRIVES_PER_BLOCK) {
            return drive_angles[drive];
        }
        return 0;
    }

    void setDriveAngle(int drive, uint16_t angle) {
        if (drive >= 0 && drive < NUM_DRIVES_PER_BLOCK) {
            drive_angles[drive] = angle;
        }
    }

    bool getPyroStatus(int pyro_index) const {
        if (pyro_index >= 0 && pyro_index < NUM_PYRO_PER_BLOCK) {
            return (pyro_mask >> pyro_index) & 1;
        }
        return false;
    }

    void setPyroStatus(int pyro_index, bool status) {
        if (pyro_index >= 0 && pyro_index < NUM_PYRO_PER_BLOCK) {
            if (status) {
                pyro_mask |= (1 << pyro_index);
            }
            else {
                pyro_mask &= ~(1 << pyro_index);
            }
        }
    }

};
#pragma pack(pop)
static_assert(sizeof(BKDResponse) == BKD_RESPONSE_SIZE, "BKDResponse size mismatch");




// ==================== ПАКЕТ ЯВ→ЯЛС (128 байт) ====================
#pragma pack(push, 1)
struct YVToYLSPacket {
    uint8_t yls_index;           // [0] Индекс в таблице ЯЛС (0-255)
    uint8_t command;                // [1] Команда для всех блоков
    uint16_t drives[NUM_BLOCKS][NUM_DRIVES_PER_BLOCK];  // [2-73] Углы для 12 блоков
    uint8_t pyro_masks[NUM_BLOCKS]; // [74-85] Маски пиро для 12 блоков
    uint8_t reserved[42];           // [86-127] Резерв

    bool isBlockActive(uint8_t block) const {
        return isBlockActiveInIndex(yls_index, block);
    }

    // Получить запрос для конкретного блока
    BKDRequest getBlockRequest(uint8_t block) const {
        BKDRequest req{};
        if (isBlockActive(block)) {
            req.block_number = block;
            req.command = command;
            req.drive_angles[0] = drives[block][0];
            req.drive_angles[1] = drives[block][1];
            req.drive_angles[2] = drives[block][2];
            req.pyro_mask = pyro_masks[block];
        }
        return req;
    }

    // Установить данные для блока
    uint16_t getBlockMask() const {
        return BKD::Protocol::getBlockMask(yls_index);
    }

                //МЕТОДЫ

    void setBlockData(uint8_t block, const uint16_t angles[3], uint8_t pyro_mask) {
        if (block < NUM_BLOCKS) {
            // Включаем блок в маске
            uint16_t current_mask = getBlockMask();
            current_mask |= (1 << block);

            // Находим индекс для этой маски
            yls_index = findIndexForMask(current_mask);

            // Устанавливаем данные
            drives[block][0] = angles[0];
            drives[block][1] = angles[1];
            drives[block][2] = angles[2];
            pyro_masks[block] = pyro_mask;
        }
    }



    // Установить данные для блока из массива std::array
    void setBlockData(uint8_t block, const std::array<uint16_t, 3>& angles, uint8_t pyro_mask) {
        setBlockData(block, angles.data(), pyro_mask);
    }

private:

};
#pragma pack(pop)
static_assert(sizeof(YVToYLSPacket) == YV_TO_YLS_SIZE, "YVToYLSPacket size mismatch");

// ==================== ПАКЕТ ЯЛС→ЯВ (8192 байт) ====================
#pragma pack(push, 1)
struct YLSToYVPacket {
    uint8_t yls_index;           // [0] Индекс в таблице ЯЛС (0-255)
    uint8_t command;                // [1] Команда (CMD_RESPONSE)
    uint16_t drives[NUM_BLOCKS][NUM_DRIVES_PER_BLOCK];  // [2-73] Углы от блоков
    uint8_t yaz_data[192];          // [74-265] Данные ЯАЗ (12×16)
    uint8_t pyro_masks[NUM_BLOCKS]; // [266-277] Маски пиро
    uint8_t ylk_data[192];          // [278-469] Данные ЯЛК (12×16)
    uint8_t yvp_data[648];          // [470-1117] Данные ЯВП (12×54)
    uint8_t reserved[7074];         // [1118-8191] Резерв

    // Установить ответ от блока
    void setBlockResponse(uint8_t block, const BKDResponse& response) {
        if (block < NUM_BLOCKS) {
            // Включаем блок в текущей маске
            uint16_t current_mask = getBlockMask();
            current_mask |= (1 << block);

            // Ищем соответствующий индекс
            yls_index = findIndexForMask(current_mask);
            drives[block][0] = response.drive_angles[0];
            drives[block][1] = response.drive_angles[1];
            drives[block][2] = response.drive_angles[2];

            // Копируем данные подсистем
            std::memcpy(&yaz_data[block * 16], response.yaz_data, 16);
            pyro_masks[block] = response.pyro_mask;
            std::memcpy(&ylk_data[block * 16], response.ylk_data, 16);
            std::memcpy(&yvp_data[block * 54], response.yvp_data, 54);
        }
    }

    // Получить ответ блока
    BKDResponse getBlockResponse(uint8_t block) const {
        BKDResponse resp{};
        if (block < NUM_BLOCKS && (yls_index & (1 << block))) {
            resp.block_number = block;
            resp.command = command;
            resp.drive_angles[0] = drives[block][0];
            resp.drive_angles[1] = drives[block][1];
            resp.drive_angles[2] = drives[block][2];

            std::memcpy(resp.yaz_data, &yaz_data[block * 16], 16);
            resp.pyro_mask = pyro_masks[block];
            std::memcpy(resp.ylk_data, &ylk_data[block * 16], 16);
            std::memcpy(resp.yvp_data, &yvp_data[block * 54], 54);
        }
        return resp;
    }

    uint16_t getBlockMask() const {
        return BKD::Protocol::getBlockMask(yls_index);
    }

    //МЕТОДЫ

    void setBlockData(uint8_t block, const uint16_t angles[3], uint8_t pyro_mask) {
        if (block < NUM_BLOCKS) {
            // Включаем блок в маске
            uint16_t current_mask = getBlockMask();
            current_mask |= (1 << block);

            // Находим индекс для этой маски
            yls_index = findIndexForMask(current_mask);

            // Устанавливаем данные
            drives[block][0] = angles[0];
            drives[block][1] = angles[1];
            drives[block][2] = angles[2];
            pyro_masks[block] = pyro_mask;
        }
    }

    // Установить данные для блока из массива std::array
    void setBlockData(uint8_t block, const std::array<uint16_t, 3>& angles, uint8_t pyro_mask) {
        setBlockData(block, angles.data(), pyro_mask);
    }

private:

};
#pragma pack(pop)


// Преобразование сырого значения в угол в градусах (-8.0 до +8.0) - позже будет доработано и добавлено
inline double rawToAngle(uint16_t raw) {
    // Предполагаю, что значение представляет собой фиксированную точку
    // или использует определенный масштаб
    return (static_cast<double>(raw) / 4096.0 - 8.0);
}

// Преобразование угла в градусах в сырое значение
inline uint16_t angleToRaw(double angle) {

    // Ограничиваю угол диапазоном -8.0 до +8.0
    angle = (std::max)(-8.0, (std::min)(8.0, angle));
    return static_cast<uint16_t>((angle + 8.0) * 4096.0);
}




} // namespace Protocol
} // namespace BKD
#endif // PROTOCOL_H

/* пример таблицы адресов ЯЛС
Индекс     | Маска (бинарно)    | Описание
    -------|--------------------|-------------------
    0      | 000000000000       | Ничего (тест)
    1      | 000000000001       | Только блок 0
    2      | 000000000011       | Блоки 0 и 1
    3      | 000000000111       | Блоки 0,1,2
    4      | 000000001111       | Блоки 0-3
    5      | 000000111111       | Блоки 0-5
    6      | 000001111111       | Блоки 0-6
    7      | 000011111111       | Блоки 0-7
    8      | 000111111111       | Блоки 0-8
    9      | 001111111111       | Блоки 0-9
    10     | 011111111111       | Блоки 0-10
    11     | 111111111111       | Все 12 блоков
*/
