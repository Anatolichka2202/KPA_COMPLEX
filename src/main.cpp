#include "core/master.h"
#include "network/dummy_network.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <chrono>
#include <thread>

std::atomic<bool> g_running{ true };

void signal_handler(int) { g_running = false; }

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    using namespace bkd::core;
    using namespace bkd::network;

    // Очереди
    Master::GuiCmdQueue   from_gui;
    Master::TickDataQueue to_gui;
    Master::TickDataQueue to_logger;

    // Сетевой слой (заглушка)
    auto network = std::make_unique<DummyNetwork>();
    if (!network->start()) {
        std::cerr << "Failed to start network layer" << std::endl;
        return 1;
    }

    // Master
    Master master(std::move(network), from_gui, to_gui, to_logger);
    master.start();

    // Главный цикл (пока без GUI)
    unsigned long tick_count = 0;
    while (g_running) {
        // Читаем все накопившиеся тики
        TickData data;
        while (to_gui.pop(data)) {
            ++tick_count;
            // Можно выводить не каждый, а только каждый 10-й, например
            if (tick_count % 10 == 0) {
                std::cout << "Tick #" << tick_count
                          << ", response: " << data.response_received
                          << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // чуть чаще опрос
    }

    master.stop();
    // network остановится в деструкторе, но можно явно:
    // network->stop(); // но у нас network уже перемещён в master

    std::cout << "Total ticks processed: " << tick_count << std::endl;
    return 0;
}
