#include "real_yls_network.h"
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#endif

namespace bkd::network {

RealYlsNetwork::RealYlsNetwork(const std::string& ip, uint16_t port)
    : ip_(ip), port_(port), socket_(io_context_), initialized_(false) {}

bool RealYlsNetwork::start() {
    if (initialized_) return true;
    try {
        socket_.open(boost::asio::ip::udp::v4());

        // Устанавливаем таймаут приёма
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = core::NETWORK_TIMEOUT_US; // 5000 мкс = 5 мс

#ifdef _WIN32
        if (setsockopt(socket_.native_handle(), SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char*>(&tv), sizeof(tv)) != 0) {
            // ошибка, но не критично
        }
#else
        if (setsockopt(socket_.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) {
            // ошибка
        }
#endif

        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void RealYlsNetwork::stop() {
    if (socket_.is_open()) socket_.close();
    initialized_ = false;
}

std::optional<core::YLSToYVPacket> RealYlsNetwork::exchange(const core::YVToYLSPacket& request) {
    if (!initialized_) return std::nullopt;

    boost::asio::ip::udp::endpoint yls_endpoint(
        boost::asio::ip::make_address(ip_), port_);

    boost::system::error_code ec;
    // Отправка
    size_t sent = socket_.send_to(boost::asio::buffer(&request, sizeof(request)), yls_endpoint, 0, ec);
    if (ec || sent != sizeof(request)) {
        return std::nullopt;
    }

    // Приём (блокирующий, но с таймаутом, установленным в start)
    core::YLSToYVPacket response;
    boost::asio::ip::udp::endpoint sender;
    size_t received = socket_.receive_from(boost::asio::buffer(&response, sizeof(response)), sender, 0, ec);
    if (ec || received != sizeof(response)) {
        return std::nullopt;
    }

    return response;
}

} // namespace bkd::network
