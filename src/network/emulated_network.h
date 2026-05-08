#pragma once

#include "inetwork_layer.h"
#include "emulators/iemulator.h"
#include "core/types.h"
#include <array>
#include <memory>

namespace bkd::network {

class EmulatedNetwork : public INetworkLayer {
public:
    explicit EmulatedNetwork(std::array<std::unique_ptr<emulators::IBlockEmulator>, core::NUM_BLOCKS> emulators);

    bool start() override;
    void stop() override;

    std::optional<core::YLSToYVPacket> exchange(const core::YVToYLSPacket& request) override;

private:
    std::array<std::unique_ptr<emulators::IBlockEmulator>, core::NUM_BLOCKS> emulators_;
};

} // namespace bkd::network
