#pragma once
#include "idata_processor.h"
#include "network.h"
#include <map>
#include <vector>
#include <chrono>

namespace YAV {

class NetworkEmulationProcessor : public IDataProcessor {
public:
    NetworkEmulationProcessor(std::unique_ptr<Network::IUdpClient> network_client,
                              const Config& config);

    void processRequest(const BKD::Protocol::YVToYLSPacket& request) override;
    void setResponseCallback(ResponseCallback callback) override;
    bool initialize() override;
    void shutdown() override;
    Stats getStats() const override { return stats_; }

private:
    struct BKDConnection {
        std::string ip;
        uint16_t port;
        int timeout_ms;
    };

    struct PendingRequest {
        uint8_t block;
        std::chrono::steady_clock::time_point sent_time;
        BKDConnection connection;
    };

    BKD::Protocol::BKDResponse sendToBKD(uint8_t block_num,
                                         const BKD::Protocol::BKDRequest& request,
                                         const BKDConnection& connection);

    void sendAllRequests(const BKD::Protocol::YVToYLSPacket& request);
    void processResponses();

    std::vector<BKDConnection> createConnections();

    Config config_;
    std::unique_ptr<Network::IUdpClient> network_client_;
    ResponseCallback response_callback_;
    Stats stats_;

    std::vector<PendingRequest> pending_requests_;
    std::map<uint8_t, BKD::Protocol::BKDResponse> received_responses_;
};

} // namespace YAV
