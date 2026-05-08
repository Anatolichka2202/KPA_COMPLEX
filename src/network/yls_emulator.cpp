#include "yls_emulator.h"
#include <cstring>
#include <iostream>

namespace bkd::network {

YlsEmulator::YlsEmulator(uint16_t listen_port,
                         std::array<std::unique_ptr<emulators::IBlockEmulator>, core::NUM_BLOCKS> emulators)
    : listen_port_(listen_port), emulators_(std::move(emulators)), running_(false) {}

YlsEmulator::~YlsEmulator() {
    stop();
}

void YlsEmulator::start() {
    if (running_) return;
    running_ = true;
    try {
        socket_ = std::make_unique<boost::asio::ip::udp::socket>(io_context_,
                                                                 boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), listen_port_));
        std::cout << "YlsEmulator: bound to port " << listen_port_ << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "YlsEmulator: bind failed: " << e.what() << std::endl;
        return;
    }
    thread_ = std::make_unique<std::thread>(&YlsEmulator::run, this);
}

void YlsEmulator::stop() {
    running_ = false;
    io_context_.stop();
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
    socket_.reset();
}

void YlsEmulator::run() {
    boost::asio::ip::udp::endpoint sender_endpoint;
    std::array<uint8_t, sizeof(core::YVToYLSPacket)> recv_buf;

    while (running_) {
        try {
            size_t len = socket_->receive_from(boost::asio::buffer(recv_buf), sender_endpoint);
            if (len != sizeof(core::YVToYLSPacket)) {
                continue;
            }

            core::YVToYLSPacket request;
            std::memcpy(&request, recv_buf.data(), sizeof(request));

            core::YLSToYVPacket response{};
            std::memset(&response, 0, sizeof(response));
            response.yls_index = request.yls_index;
            response.command = 2;

            for (uint8_t b = 0; b < core::NUM_BLOCKS; ++b) {
                if (!core::isBlockActive(request, b)) {
                    continue;
                }
                core::BKDRequest breq;
                breq.block_number = b;
                breq.command = 1;
                for (int d = 0; d < 3; ++d)
                    breq.drive_angles[d] = request.drives[b][d];
                breq.pyro_mask = request.pyro_masks[b];

                auto bresp = emulators_[b]->process(breq);

                for (int d = 0; d < 3; ++d)
                    response.drives[b][d] = bresp.drive_angles[d];
                std::memcpy(&response.yaz_data[b * 16], bresp.yaz_data, 16);
                response.pyro_masks[b] = bresp.pyro_mask;
                std::memcpy(&response.ylk_data[b * 16], bresp.ylk_data, 16);
                std::memcpy(&response.yvp_data[b * 54], bresp.yvp_data, 54);
            }

            // Отправляем ответ
            socket_->send_to(boost::asio::buffer(&response, sizeof(response)), sender_endpoint);
        } catch (const std::exception& e) {
            std::cerr << "YlsEmulator: exception: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "YlsEmulator: unknown exception" << std::endl;
        }
    }
}

} // namespace bkd::network
