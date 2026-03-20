#pragma once

#include "inetwork_layer.h"
#include <random>

namespace bkd::network {

    class DummyNetwork : public INetworkLayer {
    public:
        DummyNetwork() = default;

        bool start() override { return true; }
        void stop() override {}

        std::optional<bkd::core::YLSToYVPacket> exchange(const bkd::core::YVToYLSPacket& request) override {
            // Имитируем ответ от ЯЛС: копируем запрос и добавляем случайные данные
            bkd::core::YLSToYVPacket resp{};
            resp.yls_index = request.yls_index;
            resp.command = 2; // ответ

            // Копируем углы и маски из запроса (как эхо)
            for (int b = 0; b < bkd::core::NUM_BLOCKS; ++b) {
                for (int d = 0; d < 3; ++d)
                    resp.drives[b][d] = request.drives[b][d];
                resp.pyro_masks[b] = request.pyro_masks[b];
            }

            // Заполняем остальные данные случайными числами (имитация)
            static std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<uint8_t> dist(0, 255);

            for (auto& v : resp.yaz_data) v = dist(rng);
            for (auto& v : resp.ylk_data) v = dist(rng);
            for (auto& v : resp.yvp_data) v = dist(rng);

            return resp;
        }
    };

} // namespace bkd::network
