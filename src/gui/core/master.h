#pragma once

#include "types.h"
#include "../../core/lockfree_queues.h"
#include "../../network/inetwork_layer.h"
#include <memory>
#include <thread>
#include <atomic>

namespace bkd::core {

    class Master {
    public:
        using OutgoingQueue = SPSCQueue<YVToYLSPacket, 16>;   // для отправки в сеть (не используется при прямом вызове)
        using IncomingQueue = SPSCQueue<YLSToYVPacket, 16>;  // для приёма из сети (не используется при прямом вызове)
        using GuiCmdQueue = SPSCQueue<GuiCommand, 32>;
        using TickDataQueue = SPSCQueue<TickData, 64>;       // для GUI и логгера

        // Конструктор принимает указатель на реализацию сетевого слоя и ссылки на очереди.
        Master(std::unique_ptr<network::INetworkLayer> network,
            GuiCmdQueue& from_gui,
            TickDataQueue& to_gui,
            TickDataQueue& to_logger);
        ~Master();

        void start();
        void stop();

         void setPollingEnabled(bool enabled);
    private:
        void run();

        std::unique_ptr<network::INetworkLayer> network_;
        GuiCmdQueue& from_gui_;
        TickDataQueue& to_gui_;
        TickDataQueue& to_logger_;

        std::atomic<bool> running_;
        std::thread thread_;

        // Текущие уставки (формируются из циклограммы и команд GUI)
        YVToYLSPacket current_out_;

        // Вспомогательные функции
        void processGuiCommands();
        void updateFromTimeline(Timestamp now);  // позже добавим циклограмму
        void buildOutgoingPacket();

        std::atomic<bool> polling_enabled_;  // флаг, разрешающий опрос
    };

} // namespace bkd::core
