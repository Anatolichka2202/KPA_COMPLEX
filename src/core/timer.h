#pragma once

#include <chrono>

namespace bkd::core {

class CycleTimer {
public:
    // interval - длительность такта (например, 10 мс = 10000 мкс)
    explicit CycleTimer(std::chrono::microseconds interval);

    // Запустить таймер: запомнить время первого такта
    void start();

    // Заблокировать до следующего такта
    void wait_next();

private:
    std::chrono::microseconds interval_;
    std::chrono::steady_clock::time_point next_;
};

} // namespace bkd::core
