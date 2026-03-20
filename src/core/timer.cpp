#include "timer.h"
#include <thread>

namespace bkd::core {

CycleTimer::CycleTimer(std::chrono::microseconds interval)
    : interval_(interval) {}

void CycleTimer::start() {
    next_ = std::chrono::steady_clock::now() + interval_;
}

void CycleTimer::wait_next() {
    std::this_thread::sleep_until(next_);
    next_ += interval_;
}

} // namespace bkd::core
