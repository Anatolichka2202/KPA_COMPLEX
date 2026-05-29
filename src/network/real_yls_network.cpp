#include "real_yls_network.h"
#include <iostream>
#include <chrono>

namespace bkd::network {

RealYlsNetwork::RealYlsNetwork(const std::string& ip, uint16_t port)
    : ip_(ip), port_(port), socket_(io_context_), initialized_(false) {}

bool RealYlsNetwork::start() {
    if (initialized_) return true;
    try {
        socket_.open(boost::asio::ip::udp::v4());
        socket_.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to init socket: " << e.what() << std::endl;
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
    size_t sent = socket_.send_to(boost::asio::buffer(&request, sizeof(request)), yls_endpoint, 0, ec);
    if (ec || sent != sizeof(request)) {
        std::cerr << "Send failed: " << ec.message() << std::endl;
        return std::nullopt;
    }

    // Асинхронный приём с таймаутом
    core::YLSToYVPacket response;
    boost::asio::ip::udp::endpoint sender;
    bool received = false;
    boost::system::error_code receive_ec;

    boost::asio::steady_timer timer(io_context_);
    timer.expires_after(std::chrono::microseconds(core::NETWORK_TIMEOUT_US));
    timer.async_wait([&](boost::system::error_code ec) {
        if (!ec) {
            socket_.cancel(); // прерывает ожидающий async_receive_from
        }
    });

    socket_.async_receive_from(
        boost::asio::buffer(&response, sizeof(response)), sender,
        [&](boost::system::error_code ec, size_t bytes) {
            receive_ec = ec;
            if (!ec && bytes == sizeof(response)) {
                received = true;
            }
            timer.cancel();
        });

    io_context_.restart();
    io_context_.run(); // блокируется, пока не завершится receive или таймер

    if (received && !receive_ec) {
        return response;
    } else {
        if (receive_ec && receive_ec != boost::asio::error::operation_aborted) {
            std::cerr << "Receive error: " << receive_ec.message() << std::endl;
        }
        return std::nullopt;
    }
}

} // namespace bkd::network
