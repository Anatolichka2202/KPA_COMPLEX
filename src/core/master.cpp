#include "master.h"
#include "timer.h"
#include "types.h"
#include "databus.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>

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
    std::memset(&current_out_, 0, sizeof(current_out_));
    current_out_.data.command = 1; // по умолчанию запрос
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
    CycleTimer timer{std::chrono::microseconds(CYCLE_INTERVAL_US)};
    timer.start();

    uint32_t send_counter = 0;   // счётчик отправленных пакетов
    try {
    while (running_) {
        auto cycle_start = std::chrono::steady_clock::now();

        // 1. Обработка команд от GUI (обновление current_out_ и needSetCommand_)
        processGuiCommands();

        // 2. Формирование исходящего пакета (с учётом needSetCommand_)
        buildOutgoingPacket(send_counter);

        // 3. Отправка и получение ответа (с таймаутом внутри network_)
        auto response = network_->exchange(current_out_);

        // 4. Формирование TickData
        TickData data;
        data.tick_time = std::chrono::duration_cast<std::chrono::microseconds>(
                             cycle_start.time_since_epoch()).count();
        data.outgoing = current_out_;
        if (response.has_value()) {
            data.incoming = *response;
            data.response_received = true;
        } else {
            data.response_received = false;
        }

        if (data.response_received) {
            // Обновляем шину данными из ответа ЯЛС
            for (int block = 0; block < NUM_BLOCKS; ++block) {
                // Углы (берём из блока, процессор 0)
                for (int i = 0; i < ANGLES_PER_BLOCK; ++i) {
                    g_dataBus.setAngle(block, i, data.incoming.data.ykp[block][0][i]);
                }
                // Маска пиро
                g_dataBus.setPyroMask(block, data.incoming.data.yps_bkd[block][0]);
            }
            g_dataBus.setTickTime(data.tick_time);
        }


        // 5. Отправка в GUI и логгер
        to_gui_.push(data);
        to_logger_.push(data);

        // 6. Увеличить счётчик для следующего такта
        send_counter++;

        // 7. Ожидание до конца такта
        timer.wait_next();
    }
    } catch(const std::exception& e)
    {
        std::cerr << "Master thread exception: " << e.what() << std::endl;
    }catch(...){
        std::cerr << "Master thread unknown exception" << std::endl;
    }
}

void Master::processGuiCommands() {
    GuiCommand cmd;
    while (from_gui_.pop(cmd)) {
        switch (cmd.type) {
        case GuiCommand::SET_PYRO_MASK:
            if (cmd.block >= 0 && cmd.block < NUM_BLOCKS) {
                current_out_.data.yps_bkd[cmd.block] = cmd.pyro_mask;
                needSetCommand_ = true;
                g_dataBus.setSetpointPyroMask(cmd.block, cmd.pyro_mask);
                g_dataBus.setCommandPending(cmd.block, true);
            }
            break;

        case GuiCommand::SET_DRIVE_ANGLES:
            if (cmd.block >= 0 && cmd.block < NUM_BLOCKS) {

                for(int i=0; i<2; ++i)
                {
                     g_dataBus.setSetpointAngle(cmd.block, i, cmd.drive.angles[i]);
                }
                     g_dataBus.setCommandPending(cmd.block, true);

                // Сохраняем два угла (первый и второй)
                current_out_.data.ykp[cmd.block][0] = static_cast<int16_t>(cmd.drive.angles[0]);
                current_out_.data.ykp[cmd.block][1] = static_cast<int16_t>(cmd.drive.angles[1]);
                // Углы 3 и 4 пока игнорируем
                needSetCommand_ = true;
            }
            break;

        default:
            break;
        }
    }
}

void Master::buildOutgoingPacket(uint32_t counter) {
    // Устанавливаем command: 8 – если были изменения, иначе 1
    current_out_.data.command = needSetCommand_ ? 8 : 1;
    needSetCommand_ = false;   // сбрасываем после формирования пакета

    current_out_.data.yls_addr = 0x01;   // адрес ЯЛС
    current_out_.data.counter = counter;

    // yps_bkho_a и yaz пока остаются нулями (не используются)
}

} // namespace bkd::core
