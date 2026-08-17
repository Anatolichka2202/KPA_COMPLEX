#pragma once
#include "idata_processor.h"
#include "network.h"

namespace YAV {

class RealNetworkProcessor : public IDataProcessor {
public:
    RealNetworkProcessor(std::unique_ptr<Network::IUdpClient> network_client,
                         const Config& config);

    void processRequest(const BKD::Protocol::YVToYLSPacket& request) override;
    void setResponseCallback(ResponseCallback callback) override;
    bool initialize() override;
    void shutdown() override;
    Stats getStats() const override { return stats_; }

private:
    Config config_;
    std::unique_ptr<Network::IUdpClient> network_client_;
    ResponseCallback response_callback_;
    Stats stats_;
};

} // namespace YAV
