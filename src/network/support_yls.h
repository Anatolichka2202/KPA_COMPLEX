#pragma once

#include "core/types.h"
#include <boost/asio.hpp>
#include <string>
#include <array>

namespace bkd::network {

class SupportYls {
public:
    SupportYls(const std::string& yls_ip, uint16_t yls_port);
    ~SupportYls() = default;

    // Основной метод: принимает запрос от Master, отправляет команды на реальную ЯЛС,
    // собирает ответы и возвращает ответный пакет.
    core::YLSToYVPacket processRequest(const core::YVToYLSPacket& request);

private:
    // Вспомогательные методы для отправки команды конкретной подсистеме
    // (пока универсальные, на основе BKDRequest/BKDResponse)
    core::BKDResponse sendToSubsystem(uint8_t cell_number, const core::BKDRequest& req);

    std::string yls_ip_;
    uint16_t yls_port_;
    boost::asio::io_context io_context_;
    boost::asio::ip::udp::socket socket_;
};

} // namespace bkd::network
