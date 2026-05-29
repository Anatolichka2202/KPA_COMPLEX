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

// 1. Сначала подключаем все наши заголовки (которые включают STL)
#include "packet_modifier.h"

// 2. Затем отключаем макросы и включаем windows.h
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// 3. Принудительно убиваем min/max на случай, если они всё же появились
#undef min
#undef max

// 4. Остальные системные заголовки
#include <windivert.h>
#include <iostream>

#pragma comment(lib, "windivert.lib")

namespace TunnelLib {

PacketModifier::PacketModifier(const DivertConfig& config) : config_(config) {}

PacketModifier::~PacketModifier() {
    stop();
    if (divertHandle_) {
        WinDivertClose(divertHandle_);
    }
}

bool PacketModifier::start() {
    divertHandle_ = WinDivertOpen(config_.filter.c_str(),
                                  WINDIVERT_LAYER_NETWORK,
                                  0,
                                  0);
    if (divertHandle_ == INVALID_HANDLE_VALUE) {
        std::cerr << "WinDivertOpen failed: " << GetLastError() << std::endl;
        return false;
    }
    running_ = true;
    workerThread_ = std::thread(&PacketModifier::divertLoop, this);
    return true;
}

void PacketModifier::stop() {
    running_ = false;
    if (divertHandle_) {
        WinDivertShutdown(divertHandle_, WINDIVERT_SHUTDOWN_RECV);
    }
    if (workerThread_.joinable())
        workerThread_.join();
}

void PacketModifier::divertLoop() {
    const int MAX_PACKET_SIZE = 65535;
    uint8_t* packetBuffer = new uint8_t[MAX_PACKET_SIZE];
    WINDIVERT_ADDRESS addr;
    UINT recvLen;

    while (running_) {
        // Порядок аргументов: (handle, buf, size, &recvLen, &addr)
        if (!WinDivertRecv(divertHandle_, packetBuffer, MAX_PACKET_SIZE, &recvLen, &addr)) {
            DWORD err = GetLastError();
            if (err == ERROR_NO_DATA || err == ERROR_OPERATION_ABORTED) break;
            std::cerr << "WinDivertRecv error: " << err << std::endl;
            continue;
        }

        bool modified = false;
        if (config_.modifier) {
            modified = config_.modifier(packetBuffer, recvLen);
        }

        // WinDivertSend(handle, packet, len, NULL, &addr)
        if (!WinDivertSend(divertHandle_, packetBuffer, recvLen, NULL, &addr)) {
            std::cerr << "WinDivertSend error: " << GetLastError() << std::endl;
        }
    }
    delete[] packetBuffer;
}

} // namespace TunnelLib
