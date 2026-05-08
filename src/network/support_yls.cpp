#include "support_yls.h"
#include <iostream>
#include <cstring>

namespace bkd::network {

SupportYls::SupportYls(const std::string& yls_ip, uint16_t yls_port)
    : yls_ip_(yls_ip), yls_port_(yls_port), socket_(io_context_)
{
    socket_.open(boost::asio::ip::udp::v4());
}

core::YLSToYVPacket SupportYls::processRequest(const core::YVToYLSPacket& request) {
    core::YLSToYVPacket response{};
    std::memset(&response, 0, sizeof(response));
    response.yls_index = request.yls_index;
    response.command = 2;

    // Буфер для ответов подсистем (индекс: block*5 + subsystem_id)
    // subsystem_id: 0=ЯПС, 1=ЯКП(1), 2=ЯКП(2?), 3=ЯАЗ, 4=ЯЛК, 5=ЯВП
    // Пока для простоты используем 6 ячеек на блок (хотя ЯКП может быть две)
    const int SUBSYSTEMS_PER_BLOCK = 6;
    std::array<core::BKDResponse, core::NUM_BLOCKS * SUBSYSTEMS_PER_BLOCK> subsystem_responses;

    int cmd_index = 0;
    for (uint8_t block = 0; block < core::NUM_BLOCKS; ++block) {
        if (!core::isBlockActive(request, block))
            continue;

        // 1. ЯПС
        {
            core::BKDRequest breq;
            breq.block_number = block;
            breq.command = 1;
            breq.pyro_mask = request.pyro_masks[block];
            // Для ЯПС углы не используются, обнулим
            std::memset(breq.drive_angles, 0, sizeof(breq.drive_angles));

            // Номер ячейки для ЯПС: база + block*6 + 0
            uint8_t cell = block * SUBSYSTEMS_PER_BLOCK + 0;
            subsystem_responses[cmd_index] = sendToSubsystem(cell, breq);
            ++cmd_index;
        }

        // 2. ЯКП (одна ячейка, управляет тремя углами)
        {
            core::BKDRequest breq;
            breq.block_number = block;
            breq.command = 1;
            for (int d = 0; d < 3; ++d)
                breq.drive_angles[d] = request.drives[block][d];
            breq.pyro_mask = 0;

            uint8_t cell = block * SUBSYSTEMS_PER_BLOCK + 1;
            subsystem_responses[cmd_index] = sendToSubsystem(cell, breq);
            ++cmd_index;
        }

        // 3. ЯАЗ (запрос телеметрии)
        {
            core::BKDRequest breq;
            breq.block_number = block;
            breq.command = 1;
            std::memset(breq.drive_angles, 0, sizeof(breq.drive_angles));
            breq.pyro_mask = 0;

            uint8_t cell = block * SUBSYSTEMS_PER_BLOCK + 2;
            subsystem_responses[cmd_index] = sendToSubsystem(cell, breq);
            ++cmd_index;
        }

        // 4. ЯЛК
        {
            core::BKDRequest breq;
            breq.block_number = block;
            breq.command = 1;
            std::memset(breq.drive_angles, 0, sizeof(breq.drive_angles));
            breq.pyro_mask = 0;

            uint8_t cell = block * SUBSYSTEMS_PER_BLOCK + 3;
            subsystem_responses[cmd_index] = sendToSubsystem(cell, breq);
            ++cmd_index;
        }

        // 5. ЯВП
        {
            core::BKDRequest breq;
            breq.block_number = block;
            breq.command = 1;
            std::memset(breq.drive_angles, 0, sizeof(breq.drive_angles));
            breq.pyro_mask = 0;

            uint8_t cell = block * SUBSYSTEMS_PER_BLOCK + 4;
            subsystem_responses[cmd_index] = sendToSubsystem(cell, breq);
            ++cmd_index;
        }

        // (При необходимости можно добавить вторую ячейку для ЯКП, если их две)
    }

    // Теперь собираем итоговый ответ из полученных данных
    cmd_index = 0;
    for (uint8_t block = 0; block < core::NUM_BLOCKS; ++block) {
        if (!core::isBlockActive(request, block))
            continue;

        // Углы приводов (из ЯКП)
        const auto& ykp_resp = subsystem_responses[cmd_index + 1]; // индекс 1 - ЯКП
        for (int d = 0; d < 3; ++d)
            response.drives[block][d] = ykp_resp.drive_angles[d];

        // Маска пиро (из ЯПС)
        const auto& yps_resp = subsystem_responses[cmd_index];
        response.pyro_masks[block] = yps_resp.pyro_mask;

        // Данные ЯАЗ (16 байт)
        const auto& yaz_resp = subsystem_responses[cmd_index + 2];
        std::memcpy(&response.yaz_data[block * 16], yaz_resp.yaz_data, 16);

        // Данные ЯЛК (16 байт)
        const auto& ylk_resp = subsystem_responses[cmd_index + 3];
        std::memcpy(&response.ylk_data[block * 16], ylk_resp.ylk_data, 16);

        // Данные ЯВП (54 байта)
        const auto& yvp_resp = subsystem_responses[cmd_index + 4];
        std::memcpy(&response.yvp_data[block * 54], yvp_resp.yvp_data, 54);

        cmd_index += SUBSYSTEMS_PER_BLOCK;
    }

    return response;
}

core::BKDResponse SupportYls::sendToSubsystem(uint8_t cell_number, const core::BKDRequest& req) {
    core::BKDResponse resp{};
    std::memset(&resp, 0, sizeof(resp));

    boost::asio::ip::udp::endpoint yls_endpoint(
        boost::asio::ip::make_address(yls_ip_), yls_port_);

    // Отправляем запрос
    boost::system::error_code ec;
    size_t sent = socket_.send_to(boost::asio::buffer(&req, sizeof(req)), yls_endpoint, 0, ec);
    if (ec || sent != sizeof(req)) {
        std::cerr << "sendToSubsystem: send error " << ec.message() << std::endl;
        return resp; // возвращаем нулевой ответ
    }

    // Ждём ответ с таймаутом
    std::array<uint8_t, sizeof(core::BKDResponse)> recv_buf;
    boost::asio::ip::udp::endpoint sender;
    socket_.non_blocking(true); // неблокирующий режим

    auto start = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::microseconds(125); // 125 мкс
    while (std::chrono::steady_clock::now() - start < timeout) {
        size_t len = socket_.receive_from(boost::asio::buffer(recv_buf), sender, 0, ec);
        if (!ec && len == sizeof(core::BKDResponse)) {
            std::memcpy(&resp, recv_buf.data(), len);
            break;
        } else if (ec != boost::asio::error::would_block) {
            // ошибка
            break;
        }
        // небольшая пауза, чтобы не грузить CPU
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    socket_.non_blocking(false);

    return resp;
}

} // namespace bkd::network
