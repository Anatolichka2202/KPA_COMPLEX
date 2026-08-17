#include "idata_processor.h"
#include "full_emulation_processor.h"
#include "network_emulation_processor.h"
#include "real_network_processor.h"

namespace YAV {

std::unique_ptr<IDataProcessor> createDataProcessor(
    Config::YLSMode mode,
    std::unique_ptr<Network::IUdpClient> network_client,
    const Config& config) {

    switch (mode) {
    case Config::YLSMode::FULL_EMULATE:
        return std::make_unique<FullEmulationProcessor>(config);

    case Config::YLSMode::EMULATE_YLS:
        return std::make_unique<NetworkEmulationProcessor>(
            std::move(network_client), config);

    case Config::YLSMode::REAL:
        return std::make_unique<RealNetworkProcessor>(
            std::move(network_client), config);

    default:
        // Возвращаем полную эмуляцию по умолчанию
        return std::make_unique<FullEmulationProcessor>(config);
    }
}

} // namespace YAV
