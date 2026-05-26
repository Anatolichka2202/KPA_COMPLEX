#pragma once

#include "types.h"
#include "lockfree_queues.h"
#include "network/inetwork_layer.h"
#include <memory>
#include <thread>
#include <atomic>

namespace bkd::core {

class Master {
public:
    using GuiCmdQueue = SPSCQueue<GuiCommand, 32>;
    using TickDataQueue = SPSCQueue<TickData, 64>;

    Master(std::unique_ptr<network::INetworkLayer> network,
           GuiCmdQueue& from_gui,
           TickDataQueue& to_gui,
           TickDataQueue& to_logger);
    ~Master();

    void start();          // запускает поток, если не запущен
    void stop();           // останавливает поток (блокирует)
    bool isRunning() const { return running_; }

private:
    void run();
    void processGuiCommands();
    void buildOutgoingPacket(uint32_t counter);

    std::unique_ptr<network::INetworkLayer> network_;
    GuiCmdQueue& from_gui_;
    TickDataQueue& to_gui_;
    TickDataQueue& to_logger_;

    std::atomic<bool> running_;
    std::thread thread_;

    YVToYLSPacket current_out_;
    bool needSetCommand_ = false;   // true – следующий пакет должен быть с command=8
};

} // namespace bkd::core
