#pragma once

#include "core/types.h"

namespace bkd::emulators {

    class IBlockEmulator {
    public:
        virtual ~IBlockEmulator() = default;

        // Обработать запрос к блоку, вернуть ответ.
        // Может генерировать исключения, которые должны обрабатываться вызывающим кодом.
        virtual core::BKDResponse process(const core::BKDRequest& req) = 0;
    };

} // namespace bkd::emulators
