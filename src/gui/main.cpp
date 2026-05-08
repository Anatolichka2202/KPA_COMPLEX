#include <QApplication>
#include "mainwindow.h"
#include "core/emulator.h"

// Определение глобальных очередей (должно быть только в одном .cpp)
namespace bkd::core {
SPSCQueue<GuiCommand, 32> g_guiToEmulator;
SPSCQueue<TickData, 64> g_emulatorToGui;
SPSCQueue<TickData, 64> g_deviceToLogger;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Запуск эмулятора в отдельном потоке
    bkd::core::Emulator emulator(bkd::core::g_guiToEmulator,
                                 bkd::core::g_emulatorToGui,
                                 bkd::core::g_deviceToLogger);
    emulator.start();

    MainWindow w;
    w.show();

    return app.exec();
}
