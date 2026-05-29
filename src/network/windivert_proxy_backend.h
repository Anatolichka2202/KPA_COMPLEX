// windivert_proxy_backend.h
#pragma once
#include "i_proxy_backend.h"
#include <memory>
#include <string>

namespace bkd::proxy {

class WinDivertProxyBackend : public IProxyBackend {
public:
    WinDivertProxyBackend(const std::string& bcvm_ip, const std::string& yls_ip);
    ~WinDivertProxyBackend() override;

    bool start() override;
    void stop() override;
    void setModifier(PacketModifier modifier) override;

private:
    std::string bcvm_ip_;
    std::string yls_ip_;
    PacketModifier modifier_;
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace bkd::proxy
