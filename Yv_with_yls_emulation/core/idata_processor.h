#ifndef IDATA_PROCESSOR_H
#define IDATA_PROCESSOR_H

#pragma once

#include "config.h"
#include "Protocol.h"
#include <functional>
#include <memory>
#include "network.h"
namespace YAV {

class IDataProcessor {
public:
    using ResponseCallback = std::function<void(const BKD::Protocol::YLSToYVPacket&)>;

    virtual ~IDataProcessor() = default;

    virtual void processRequest(const BKD::Protocol::YVToYLSPacket& request) = 0;
    virtual void setResponseCallback(ResponseCallback callback) = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    struct Stats {
        uint64_t packets_sent = 0;
        uint64_t packets_received = 0;
        uint64_t errors = 0;
    };
    virtual Stats getStats() const = 0;
};

// Фабрика для создания процессоров
std::unique_ptr<IDataProcessor> createDataProcessor(
    Config::YLSMode mode,
    std::unique_ptr<Network::IUdpClient> network_client,
    const Config& config);

} // namespace YAV

#endif // IDATA_PROCESSOR_H
