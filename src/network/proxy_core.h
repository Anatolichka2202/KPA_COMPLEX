// udp_proxy_backend.h
#pragma once
#include "i_proxy_backend.h"
#include <boost/asio.hpp>
#include <atomic>
#include <thread>
#include <string>

namespace bkd::proxy {

class UdpProxyBackend : public IProxyBackend {
public:
    UdpProxyBackend(const std::string& listen_ip, uint16_t listen_port,
                    const std::string& yls_ip, uint16_t yls_port);
    ~UdpProxyBackend() override;

    bool start() override;
    void stop() override;
    void setModifier(PacketModifier modifier) override { modifier_ = std::move(modifier); }

private:
    void runLoop();

    std::string listen_ip_;
    uint16_t listen_port_;
    std::string yls_ip_;
    uint16_t yls_port_;

    boost::asio::io_context io_context_;
    boost::asio::ip::udp::socket listen_socket_;
    boost::asio::ip::udp::endpoint yls_endpoint_;

    PacketModifier modifier_;
    std::atomic<bool> running_{false};
    std::thread worker_;
};

} // namespace bkd::proxy
