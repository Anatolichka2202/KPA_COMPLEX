#include <QApplication>
#include <QTimer>
#include <csignal>
#include <atomic>
#include "core/master.h"
#include "network/real_yls_network.h"
#include "gui/mainwindow.h"
#include "core/queues.h"

std::atomic<bool> g_running{ true };
void signal_handler(int) { g_running = false; }

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    QApplication app(argc, argv);

    using namespace bkd::core;
    using namespace bkd::network;

    // Создаём сетевой слой (реальная ЯЛС)
    auto network = std::make_unique<RealYlsNetwork>(YLS_IP, YLS_PORT);
    if (!network->start()) {
        qWarning() << "Failed to start YLS network layer";
        // Можно продолжить, но без связи работать не будет
    }

    // Запускаем Master, передаём ему очереди
    Master master(std::move(network), g_guiToMaster, g_masterToGui, g_masterToLogger);
    master.start();

    // Создаём главное окно
    MainWindow w;
    w.show();

    // Таймер для обновления GUI из очереди (30 fps)
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &w, &MainWindow::updateFromMaster);
    timer.start(33);

    int result = app.exec();

    master.stop();
    return result;
}
