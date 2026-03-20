#include "master.h"
#include "timer.h"
#include "types.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>

static_assert(std::is_class_v<bkd::core::CycleTimer>, "CycleTimer is not a class");

namespace bkd::core {

    Master::Master(std::unique_ptr<network::INetworkLayer> network,
        GuiCmdQueue& from_gui,
        TickDataQueue& to_gui,
        TickDataQueue& to_logger)
        : network_(std::move(network))
        , from_gui_(from_gui)
        , to_gui_(to_gui)
        , to_logger_(to_logger)
        , running_(false)
    {
        // Инициализация current_out_ (например, нулями)
        std::memset(&current_out_, 0, sizeof(current_out_));
        current_out_.command = 1; // команда запроса
    }

    Master::~Master() {
        stop();
    }

    void Master::start() {
        if (running_) return;
        running_ = true;
        thread_ = std::thread(&Master::run, this);
    }

    void Master::stop() {
        running_ = false;
        if (thread_.joinable())
            thread_.join();
    }

    void Master::run() {
        // Установим высокий приоритет потока (зависит от ОС)

        CycleTimer timer{std::chrono::microseconds(CYCLE_INTERVAL_US)};
        timer.start();


        while (running_) {
            auto cycle_start = std::chrono::steady_clock::now();

            // 1. Обработка команд от GUI
            processGuiCommands();

            // 2. Обновление циклограммы (пока заглушка)
            // updateFromTimeline(...);

            // 3. Формирование исходящего пакета
            buildOutgoingPacket();

            // 4. Отправка через сетевой слой и получение ответа
            auto response = network_->exchange(current_out_);

            // 5. Формирование TickData
            TickData data;
            data.tick_time = 0; // пока 0, позже получим из PTP
            data.outgoing = current_out_;
            if (response.has_value()) {
                data.incoming = *response;
                data.response_received = true;
            }
            else {
                data.response_received = false;
                // Можно заполнить incoming нулями или старыми данными
            }

            // 6. Отправка данных в GUI и логгер (если очереди не переполнены)
            to_gui_.push(data);
            to_logger_.push(data);

            // 7. Ожидание до конца такта
            timer.wait_next();
        }
    }

    void Master::processGuiCommands() {
        GuiCommand cmd;
        while (from_gui_.pop(cmd)) {
            switch (cmd.type) {
            case GuiCommand::SET_PYRO_MASK:
                if (cmd.block >= 0 && cmd.block < NUM_BLOCKS) {
                    current_out_.pyro_masks[cmd.block] = cmd.pyro_mask;
                }
                break;
            case GuiCommand::SET_DRIVE_ANGLES:
                if (cmd.block >= 0 && cmd.block < NUM_BLOCKS) {
                    for (int i = 0; i < 3; ++i)
                        current_out_.drives[cmd.block][i] = cmd.drive.angles[i];
                }
                break;
            default:
                break;
            }
        }
    }

    void Master::buildOutgoingPacket() {
        // Здесь можно обновить yls_index на основе активных блоков.
        // Пока просто установим индекс 12 (все блоки активны).
        current_out_.yls_index = 12;
    }

} // namespace bkd::core
