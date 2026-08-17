#pragma once

#include <functional>
#include <memory>

namespace YAV {

class ITimer {
public:
    virtual ~ITimer() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual void setInterval(int ms) = 0;
};

class ITimerFactory {
public:
    virtual ~ITimerFactory() = default;
    virtual std::unique_ptr<ITimer> createTimer(std::function<void()> callback) = 0;
};

} // namespace YAV
