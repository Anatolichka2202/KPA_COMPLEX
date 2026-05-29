// udp_proxy_backend.cpp
#include "proxy_core.h"
#include <iostream>
#include <chrono>

using boost::asio::ip::udp;

namespace bkd::proxy {

UdpProxyBackend::UdpProxyBackend(const std::string& listen_ip, uint16_t listen_port,
                                 const std::string& yls_ip, uint16_t yls_port)
    : listen_ip_(listen_ip), listen_port_(listen_port),
    yls_ip_(yls_ip), yls_port_(yls_port),
    listen_socket_(io_context_),
    yls_endpoint_(boost::asio::ip::make_address(yls_ip), yls_port)
{}

UdpProxyBackend::~UdpProxyBackend() { stop(); }

bool UdpProxyBackend::start() {
    try {
        listen_socket_.open(udp::v4());
        listen_socket_.bind(udp::endpoint(boost::asio::ip::make_address(listen_ip_), listen_port_));
        running_ = true;
        worker_ = std::thread(&UdpProxyBackend::runLoop, this);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "UdpProxyBackend start failed: " << e.what() << std::endl;
        return false;
    }
}

void UdpProxyBackend::stop() {
    running_ = false;
    listen_socket_.cancel();
    if (worker_.joinable()) worker_.join();
    if (listen_socket_.is_open()) listen_socket_.close();
    io_context_.stop();
}

void UdpProxyBackend::runLoop() {
    const size_t BUFFER_SIZE = 8192; // достаточно для YLSToYVPacket
    std::vector<uint8_t> recv_buf(BUFFER_SIZE);
    udp::endpoint client_endpoint;

    while (running_) {
        try {
            size_t n = listen_socket_.receive_from(boost::asio::buffer(recv_buf), client_endpoint);
            if (n == 0) continue;

            // Вызываем модификатор (если установлен)
            if (modifier_ && modifier_(recv_buf.data(), static_cast<uint32_t>(n))) {
                // пакет изменён
            }

            // Пересылаем на ЯЛС
            listen_socket_.send_to(boost::asio::buffer(recv_buf.data(), n), yls_endpoint_);

            // Ждём ответ от ЯЛС (с таймаутом)
            boost::system::error_code ec;
            udp::endpoint yls_sender;
            size_t resp_len = listen_socket_.receive_from(boost::asio::buffer(recv_buf), yls_sender, 0, ec);
            if (ec) {
                std::cerr << "Receive from YLS timeout/error: " << ec.message() << std::endl;
                continue;
            }

            if (modifier_ && modifier_(recv_buf.data(), static_cast<uint32_t>(resp_len))) {
                // изменяем ответ, если нужно
            }

            // Отправляем ответ обратно БЦВМ
            listen_socket_.send_to(boost::asio::buffer(recv_buf.data(), resp_len), client_endpoint);
        } catch (const std::exception& e) {
            if (running_) std::cerr << "UdpProxyBackend loop error: " << e.what() << std::endl;
        }
    }
}

} // namespace bkd::proxy
