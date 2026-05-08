#pragma once
#include "../../core/lockfree_queues.h"
#include "types.h"

namespace bkd::core {
extern SPSCQueue<GuiCommand, 32> g_guiToEmulator;
extern SPSCQueue<TickData, 64> g_emulatorToGui;
extern SPSCQueue<TickData, 64> g_deviceToLogger;
}
