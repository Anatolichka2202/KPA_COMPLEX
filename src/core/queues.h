#pragma once

#include "core/lockfree_queues.h"
#include "core/types.h"

namespace bkd::core {

extern SPSCQueue<GuiCommand, 32> g_guiToMaster;
extern SPSCQueue<TickData, 64> g_masterToGui;
extern SPSCQueue<TickData, 64> g_masterToLogger;

} // namespace bkd::core
