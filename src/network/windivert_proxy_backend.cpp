// windivert_proxy_backend.cpp
#include "windivert_proxy_backend.h"
#include "TunnelLib/include/tunnel_lib.h"      // ваша библиотека TunnelLib
#include <iostream>

namespace bkd::proxy {

class WinDivertProxyBackend::Impl {
public:
    TunnelLib::Tunnel* tunnel = nullptr;
    TunnelLib::TunnelConfig config;
    PacketModifier modifier;

    Impl(const std::string& bcvm_ip, const std::string& yls_ip)
        : config()
    {
        config.deviceName = TunnelLib::getDefaultEthernetInterface();
        if (config.deviceName.empty()) {
            throw std::runtime_error("No suitable Ethernet interface found");
        }
        config.deviceA_ip = bcvm_ip;
        config.deviceB_ip = yls_ip;
        // Порты не критичны для ARP+WinDivert, но можно задать для фильтра
        config.deviceA_port = 101;
        config.deviceB_port = 101;
        config.packetModifier = nullptr; // установим позже
    }
};

WinDivertProxyBackend::WinDivertProxyBackend(const std::string& bcvm_ip, const std::string& yls_ip)
    : bcvm_ip_(bcvm_ip), yls_ip_(yls_ip)
    , pImpl(std::make_unique<Impl>(bcvm_ip, yls_ip))
{}

WinDivertProxyBackend::~WinDivertProxyBackend() {
    stop();
}

void WinDivertProxyBackend::setModifier(PacketModifier modifier) {
    modifier_ = modifier;
    if (pImpl) {
        pImpl->modifier = modifier;
        // Обновим конфиг туннеля
        pImpl->config.packetModifier = [this](uint8_t* data, uint32_t len) -> bool {
            if (modifier_) return modifier_(data, len);
            return false;
        };
    }
}

bool WinDivertProxyBackend::start() {
    if (!pImpl) return false;
    pImpl->tunnel = new TunnelLib::Tunnel(pImpl->config);
    if (!pImpl->tunnel->start()) {
        delete pImpl->tunnel;
        pImpl->tunnel = nullptr;
        return false;
    }
    return true;
}

void WinDivertProxyBackend::stop() {
    if (pImpl && pImpl->tunnel) {
        pImpl->tunnel->stop();
        delete pImpl->tunnel;
        pImpl->tunnel = nullptr;
    }
}

} // namespace bkd::proxy
