#pragma once

#include "../../core/lockfree_queues.h"
#include "types.h"
#include <atomic>
#include <thread>
#include <random>

namespace bkd::core {

class Emulator {
public:
    using GuiCmdQueue = SPSCQueue<GuiCommand, 32>;
    using TickDataQueue = SPSCQueue<TickData, 64>;

    Emulator(GuiCmdQueue& from_gui, TickDataQueue& to_gui, TickDataQueue& to_logger);
    ~Emulator();

    void start();
    void stop();

private:
    void run();

    GuiCmdQueue& from_gui_;
    TickDataQueue& to_gui_;
    TickDataQueue& to_logger_;

    std::atomic<bool> running_;
    std::thread thread_;

    // Эмулируемые состояния блоков
    struct BlockState {
        int16_t currentAngles[4] = {0,0,0,0};
        int16_t targetAngles[4] = {0,0,0,0};
        uint8_t pyroMask = 0;
        uint8_t requestedPyroMask = 0; // то, что пришло от GUI
    };
    BlockState blocks_[NUM_BLOCKS];

    // Генераторы случайных чисел
    std::mt19937 rng_;
    std::uniform_int_distribution<int> probDist_; // для вероятности 30%

    void processGuiCommands();
    void updateSimulation();   // движение углов, пиро
    YLSToYVPacket buildResponse(const YVToYLSPacket& request);
};

} // namespace bkd::core
