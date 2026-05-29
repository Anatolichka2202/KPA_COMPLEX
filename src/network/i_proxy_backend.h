// proxy_backend.h
#pragma once
#include <functional>
#include <cstdint>

namespace bkd::proxy {

// Тип функции-модификатора пакета:
// - Вход: указатель на сырой пакет (Ethernet+IP+UDP+данные) и его длина.
// - Возвращает true, если пакет был изменён.
using PacketModifier = std::function<bool(uint8_t* packet, uint32_t len)>;

class IProxyBackend {
public:
    virtual ~IProxyBackend() = default;

    // Запустить прокси-сервер (блокирующий или в отдельном потоке – решает реализация)
    virtual bool start() = 0;

    // Остановить прокси
    virtual void stop() = 0;

    // Установить функцию модификации (может быть вызвана до start())
    virtual void setModifier(PacketModifier modifier) = 0;
};

} // namespace bkd::proxy
