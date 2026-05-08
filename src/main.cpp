#include <QApplication>
#include <QTimer>
#include <csignal>
#include <atomic>
#include "core/master.h"
#include "network/real_yls_network.h"
#include "core/test_widget.h"

std::atomic<bool> g_running{ true };
void signal_handler(int) { g_running = false; }

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    QApplication app(argc, argv);

    using namespace bkd::core;
    using namespace bkd::network;

    Master::GuiCmdQueue   from_gui;
    Master::TickDataQueue to_gui;
    Master::TickDataQueue to_logger;

    auto network = std::make_unique<RealYlsNetwork>(YLS_IP, YLS_PORT);
    if (!network->start()) {
        // при ошибке можно продолжить, но с эмуляцией? пока просто выходим
        return 1;
    }

    Master master(std::move(network), from_gui, to_gui, to_logger);
    master.start();

    TestWidget widget(from_gui, to_gui);
    widget.show();

    // Таймер обновления 30 FPS
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &widget, &TestWidget::processIncoming);
    timer.start(33);  // ~30 кадров в секунду

    int result = app.exec();

    master.stop();
    return result;
}
