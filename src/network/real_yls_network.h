#pragma once

#include "inetwork_layer.h"
#include <boost/asio.hpp>
#include <string>
#include <optional>

namespace bkd::network {

class RealYlsNetwork : public INetworkLayer {
public:
    RealYlsNetwork(const std::string& ip, uint16_t port);
    ~RealYlsNetwork() override = default;

    bool start() override;
    void stop() override;

    std::optional<core::YLSToYVPacket> exchange(const core::YVToYLSPacket& request) override;

private:
    std::string ip_;
    uint16_t port_;
    boost::asio::io_context io_context_;
    boost::asio::ip::udp::socket socket_;
    bool initialized_;
};

} // namespace bkd::network
