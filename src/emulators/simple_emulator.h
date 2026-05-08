#pragma once

#include "iemulator.h"
#include "core/types.h"

namespace bkd::emulators {

class SimpleEmulator : public IBlockEmulator {
public:
    SimpleEmulator();
    core::BKDResponse process(const core::BKDRequest& req) override;

private:
    // внутреннее состояние
    uint16_t angles_[3];
    uint8_t pyro_mask_;
};

} // namespace bkd::emulators
