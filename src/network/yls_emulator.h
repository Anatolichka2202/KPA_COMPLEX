#pragma once

#include "core/types.h"
#include "emulators/iemulator.h"
#include <boost/asio.hpp>
#include <array>
#include <memory>
#include <thread>

namespace bkd::network {

class YlsEmulator {
public:
    YlsEmulator(uint16_t listen_port,
                std::array<std::unique_ptr<emulators::IBlockEmulator>, core::NUM_BLOCKS> emulators);
    ~YlsEmulator();

    void start();
    void stop();

private:
    void run();

    uint16_t listen_port_;
    std::array<std::unique_ptr<emulators::IBlockEmulator>, core::NUM_BLOCKS> emulators_;

    boost::asio::io_context io_context_;
    std::unique_ptr<boost::asio::ip::udp::socket> socket_;
    std::unique_ptr<std::thread> thread_;
    bool running_;
};

} // namespace bkd::network
