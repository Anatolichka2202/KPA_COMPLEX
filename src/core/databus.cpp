// core/databus.cpp
#include "databus.h"
#include <cassert>

namespace bkd::core {

DataBus g_dataBus;

void DataBus::setAngle(int block, int idx, int16_t value) {
    assert(block >= 0 && block < NUM_BLOCKS);
    assert(idx >= 0 && idx < ANGLES_PER_BLOCK);
    angles_[block][idx].store(value, std::memory_order_relaxed);
}

int16_t DataBus::getAngle(int block, int idx) const {
    assert(block >= 0 && block < NUM_BLOCKS);
    assert(idx >= 0 && idx < ANGLES_PER_BLOCK);
    return angles_[block][idx].load(std::memory_order_relaxed);
}

void DataBus::setPyroMask(int block, uint8_t mask) {
    assert(block >= 0 && block < NUM_BLOCKS);
    pyroMasks_[block].store(mask, std::memory_order_relaxed);
}

uint8_t DataBus::getPyroMask(int block) const {
    assert(block >= 0 && block < NUM_BLOCKS);
    return pyroMasks_[block].load(std::memory_order_relaxed);
}

void DataBus::setSetpointAngle(int block, int idx, int16_t value) {
    assert(block >= 0 && block < NUM_BLOCKS);
    assert(idx >= 0 && idx < 2); // только два угла
    setpointAngles_[block][idx].store(value, std::memory_order_relaxed);
}

int16_t DataBus::getSetpointAngle(int block, int idx) const {
    assert(block >= 0 && block < NUM_BLOCKS);
    assert(idx >= 0 && idx < 2);
    return setpointAngles_[block][idx].load(std::memory_order_relaxed);
}

void DataBus::setSetpointPyroMask(int block, uint8_t mask) {
    assert(block >= 0 && block < NUM_BLOCKS);
    setpointPyroMasks_[block].store(mask, std::memory_order_relaxed);
}

uint8_t DataBus::getSetpointPyroMask(int block) const {
    assert(block >= 0 && block < NUM_BLOCKS);
    return setpointPyroMasks_[block].load(std::memory_order_relaxed);
}

void DataBus::setCommandPending(int block, bool pending) {
    assert(block >= 0 && block < NUM_BLOCKS);
    commandPending_[block].store(pending, std::memory_order_relaxed);
}

bool DataBus::isCommandPending(int block) const {
    assert(block >= 0 && block < NUM_BLOCKS);
    return commandPending_[block].load(std::memory_order_relaxed);
}

void DataBus::setTickTime(uint64_t us) {
    tickTime_.store(us, std::memory_order_relaxed);
}

uint64_t DataBus::getTickTime() const {
    return tickTime_.load(std::memory_order_relaxed);
}

} // namespace bkd::core
