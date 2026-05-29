#ifdef __in
#undef __in
#endif
#ifdef __out
#undef __out
#endif
#ifdef __in_opt
#undef __in_opt
#endif
#ifdef __out_opt
#undef __out_opt
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "tunnel_lib.h"
#include <iostream>

bool IsElevated() {
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, cbSize, &cbSize))
            fRet = Elevation.TokenIsElevated;
        CloseHandle(hToken);
    }
    return fRet;
}
int main() {
    std::string ifaceName = TunnelLib::getDefaultEthernetInterface();
    if (ifaceName.empty()) {
        std::cerr << "Could not find network interface" << std::endl;
        return 1;
    }
    std::cout << "Using interface: " << ifaceName << std::endl;

    TunnelLib::TunnelConfig cfg;
    cfg.deviceName = ifaceName;
    cfg.deviceA_ip = "192.168.17.246";
    cfg.deviceA_port = 200;
    cfg.deviceB_ip = "192.168.17.230";
    cfg.deviceB_port = 101;
    // Пока без модификации
    cfg.packetModifier = nullptr;   // или закомментировать

    TunnelLib::Tunnel tunnel(cfg);
    if (!tunnel.start()) {
        std::cerr << "Failed to start tunnel" << std::endl;
        return 1;
    }

    std::cout << "Tunnel is running. Press Enter to stop." << std::endl;
    std::cin.get();
    tunnel.stop();
    return 0;
}
