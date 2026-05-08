#include "simple_emulator.h"
#include <cstring>
#include <random>
#include "core/types.h"
namespace bkd::emulators {

SimpleEmulator::SimpleEmulator() : angles_{0,0,0}, pyro_mask_(0) {}

core::BKDResponse SimpleEmulator::process(const core::BKDRequest& req) {
    // обновляем состояние
    for (int i = 0; i < 3; ++i)
        angles_[i] = req.drive_angles[i];
    pyro_mask_ = req.pyro_mask;

    core::BKDResponse resp;
    resp.block_number = req.block_number;
    resp.command = 2; // ответ
    for (int i = 0; i < 3; ++i)
        resp.drive_angles[i] = angles_[i]; // эхо
    resp.pyro_mask = pyro_mask_;           // эхо

    // заполняем остальные поля случайными данными (для теста)
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint8_t> dist(0, 255);

    for (auto& v : resp.yaz_data) v = dist(rng);
    for (auto& v : resp.ylk_data) v = dist(rng);
    for (auto& v : resp.yvp_data) v = dist(rng);

    return resp;
}

} // namespace bkd::emulators
