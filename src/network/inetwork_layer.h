#pragma once

#include "core/types.h"
#include <optional>

namespace bkd::network {

    class INetworkLayer {
    public:
        virtual ~INetworkLayer() = default;

        // Отправить запрос и получить ответ (блокирующий вызов с таймаутом).
        // Должен выполняться в потоке Master, но с использованием асинхронности внутри.
        // Возвращает ответ или nullopt, если таймаут/ошибка.
        virtual std::optional<bkd::core::YLSToYVPacket> exchange(const bkd::core::YVToYLSPacket& request) = 0;

        // Запустить фоновые потоки/инициализацию (если нужны).
        virtual bool start() = 0;

        // Остановить и освободить ресурсы.
        virtual void stop() = 0;
    };

} // namespace bkd::network
