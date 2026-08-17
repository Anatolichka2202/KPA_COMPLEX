// full_emulation_processor.h
#pragma once
#include "idata_processor.h"
#include "Protocol.h"
#include <functional>

namespace YAV {

class FullEmulationProcessor : public IDataProcessor {
public:
    FullEmulationProcessor(const Config& config);
    ~FullEmulationProcessor() override;

    bool initialize() override;
    void processRequest(const BKD::Protocol::YVToYLSPacket& request) override;
    void setResponseCallback(ResponseCallback callback) override;
    void shutdown() override;
    Stats getStats() const override ;
private:
    BKD::Protocol::BKDResponse emulateBKD(const BKD::Protocol::BKDRequest& request);

    ResponseCallback response_callback_;
    uint64_t packet_counter_ = 0;

};

} // namespace YAV
