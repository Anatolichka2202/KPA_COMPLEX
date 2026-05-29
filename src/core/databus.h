// core/databus.h
#pragma once

#include <atomic>
#include <cstdint>
#include <array>
#include "types.h"
namespace bkd::core {

// Константы
//constexpr int NUM_BLOCKS = 12;
constexpr int ANGLES_PER_BLOCK = 2;

// Класс DataBus – thread-safe хранилище состояния
class DataBus {
public:
    // --- Углы (текущие, от ЯЛС) ---
    void setAngle(int block, int idx, int16_t value);
    int16_t getAngle(int block, int idx) const;

    // --- Пиро маски (текущие) ---
    void setPyroMask(int block, uint8_t mask);
    uint8_t getPyroMask(int block) const;

    // --- Уставки (от GUI или сценария) ---
    void setSetpointAngle(int block, int idx, int16_t value);
    int16_t getSetpointAngle(int block, int idx) const;

    void setSetpointPyroMask(int block, uint8_t mask);
    uint8_t getSetpointPyroMask(int block) const;

    // --- Команда на отправку (признак, что требуется command=8) ---
    void setCommandPending(int block, bool pending);
    bool isCommandPending(int block) const;

    // --- Глобальное время такта (микросекунды) ---
    void setTickTime(uint64_t us);
    uint64_t getTickTime() const;

private:
    // Углы: атомарные массивы
    std::array<std::array<std::atomic<int16_t>, ANGLES_PER_BLOCK>, NUM_BLOCKS> angles_;
    std::array<std::atomic<uint8_t>, NUM_BLOCKS> pyroMasks_;

    // Уставки
    std::array<std::array<std::atomic<int16_t>, 2>, NUM_BLOCKS> setpointAngles_; // пока только 2 угла
    std::array<std::atomic<uint8_t>, NUM_BLOCKS> setpointPyroMasks_;

    // Флаги необходимости отправки SET команды
    std::array<std::atomic<bool>, NUM_BLOCKS> commandPending_;

    // Глобальное время
    std::atomic<uint64_t> tickTime_{0};
};

// Глобальный экземпляр (пока для совместимости с существующим кодом)
extern DataBus g_dataBus;

} // namespace bkd::core
