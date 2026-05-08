#include "core/queues.h"

namespace bkd::core {

SPSCQueue<GuiCommand, 32> g_guiToMaster;
SPSCQueue<TickData, 64> g_masterToGui;
SPSCQueue<TickData, 64> g_masterToLogger;

} // namespace bkd::core
