#include "emulated_network.h"
#include <cstring>

namespace bkd::network {

EmulatedNetwork::EmulatedNetwork(std::array<std::unique_ptr<emulators::IBlockEmulator>, core::NUM_BLOCKS> emulators)
    : emulators_(std::move(emulators)) {}

bool EmulatedNetwork::start() { return true; }
void EmulatedNetwork::stop() {}

std::optional<core::YLSToYVPacket> EmulatedNetwork::exchange(const core::YVToYLSPacket& request) {
    core::YLSToYVPacket response;
    std::memset(&response, 0, sizeof(response));
    response.yls_index = request.yls_index;
    response.command = 2; // ответ

    for (uint8_t b = 0; b < core::NUM_BLOCKS; ++b) {
        if (!core::isBlockActive(request, b))
            continue;

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

    return response;
}

} // namespace bkd::network
