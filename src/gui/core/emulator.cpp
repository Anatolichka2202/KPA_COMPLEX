#include "emulator.h"
#include "../../core/timer.h"
#include <cstring>
#include <algorithm>
#include <chrono>
inline constexpr int CYCLE_INTERVAL_US = 10000;
namespace bkd::core {

Emulator::Emulator(GuiCmdQueue& from_gui, TickDataQueue& to_gui, TickDataQueue& to_logger)
    : from_gui_(from_gui), to_gui_(to_gui), to_logger_(to_logger),
      running_(false), rng_(std::random_device{}()), probDist_(0, 99) {}

Emulator::~Emulator() {
    stop();
}

void Emulator::start() {
    if (running_) return;
    running_ = true;
    thread_ = std::thread(&Emulator::run, this);
}

void Emulator::stop() {
    running_ = false;
    if (thread_.joinable())
        thread_.join();
}

void Emulator::run() {
    CycleTimer timer{std::chrono::microseconds(CYCLE_INTERVAL_US)};
    timer.start();

    while (running_) {
        processGuiCommands();

        // Формируем запрос (фиктивный, для эмуляции достаточно текущих уставок)
        YVToYLSPacket request;
        std::memset(&request, 0, sizeof(request));
        request.yls_index = 12; // опрашиваем все блоки
        request.command = 1;
        // Копируем целевые углы и маски пиро из blocks_ (они уже обновлены командами)
        for (int b = 0; b < NUM_BLOCKS; ++b) {
            for (int a = 0; a < 4; ++a)
                request.drives[b][a] = blocks_[b].targetAngles[a];
            request.pyro_masks[b] = blocks_[b].requestedPyroMask;
        }

        // Симулируем ответ
        YLSToYVPacket response = buildResponse(request);
        updateSimulation(); // обновляем текущие углы и маски

        // Упаковываем TickData
        TickData data;
        data.tick_time = 0; // пока 0
        data.outgoing = request;
        data.incoming = response;
        data.response_received = true;
        to_gui_.push(data);
        to_logger_.push(data);

        timer.wait_next();
    }
}

void Emulator::processGuiCommands() {
    GuiCommand cmd;
    while (from_gui_.pop(cmd)) {
        if (cmd.block < 0 || cmd.block >= NUM_BLOCKS) continue;
        switch (cmd.type) {
        case GuiCommand::SET_PYRO_MASK:
            blocks_[cmd.block].requestedPyroMask = cmd.pyro_mask;
            break;
        case GuiCommand::SET_DRIVE_ANGLES:
            for (int i = 0; i < 4; ++i)
                blocks_[cmd.block].targetAngles[i] = cmd.drive.angles[i];
            break;
        case GuiCommand::START_POLLING:
            // игнорируем, эмуляция всегда активна
            break;
        case GuiCommand::STOP_POLLING:
            break;
        default:
            break;
        }
    }
}

void Emulator::updateSimulation() {
    for (int b = 0; b < NUM_BLOCKS; ++b) {
        BlockState& blk = blocks_[b];
        // Углы: движение к цели со скоростью 5°/тик (максимум)
        for (int a = 0; a < 4; ++a) {
            int16_t target = blk.targetAngles[a];
            int16_t current = blk.currentAngles[a];
            int16_t diff = target - current;
            if (std::abs(diff) <= 5) {
                blk.currentAngles[a] = target;
            } else {
                int16_t step = (diff > 0) ? 5 : -5;
                blk.currentAngles[a] = current + step;
            }
            // Добавляем случайное колебание ±2°, если достигли цели и не в движении
            if (blk.currentAngles[a] == target && target != 0) {
                int16_t r = (rng_() % 5) - 2;
                int16_t newVal = blk.currentAngles[a] + r;
                if (newVal > 140) newVal = 140;
                if (newVal < -140) newVal = -140;
                blk.currentAngles[a] = newVal;
            }
        }
        // Пиро: с вероятностью 30% за каждый бит, который запрошен, он становится активным
        uint8_t req = blk.requestedPyroMask;
        uint8_t& cur = blk.pyroMask;
        if (req != cur) {
            for (int i = 0; i < 8; ++i) {
                if ((req >> i) & 1) {
                    if (probDist_(rng_) < 30) { // 30% успеха
                        cur |= (1 << i);
                    }
                }
            }
        }
    }
}

YLSToYVPacket Emulator::buildResponse(const YVToYLSPacket& request) {
    YLSToYVPacket resp;
    std::memset(&resp, 0, sizeof(resp));
    resp.yls_index = request.yls_index;
    resp.command = 2;
    for (int b = 0; b < NUM_BLOCKS; ++b) {
        for (int a = 0; a < 4; ++a) {
            resp.drives[b][a] = blocks_[b].currentAngles[a];
        }
        resp.pyro_masks[b] = blocks_[b].pyroMask;
    }
    return resp;
}

} // namespace bkd::core
