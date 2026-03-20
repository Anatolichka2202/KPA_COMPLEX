#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include <optional>

namespace bkd::core {

    template<typename T, size_t Capacity>
    class SPSCQueue {
    public:
        bool push(const T& value) {
            return queue_.push(value);
        }

        bool pop(T& value) {
            return queue_.pop(value);
        }

        std::optional<T> pop() {
            T value;
            if (queue_.pop(value))
                return value;
            return std::nullopt;
        }

        // Проверка, пуста ли очередь (неатомарно, только для отладки)
        bool empty() const {
            return queue_.empty();
        }

    private:
        boost::lockfree::spsc_queue<T, boost::lockfree::capacity<Capacity>> queue_;
    };

} // namespace bkd::core
