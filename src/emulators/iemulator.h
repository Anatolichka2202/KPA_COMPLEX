#pragma once

#include "core/types.h"

namespace bkd::emulators {

    class IBlockEmulator {
    public:
        virtual ~IBlockEmulator() = default;

        // Обработать запрос к блоку, вернуть ответ.
        // Может генерировать исключения, которые должны обрабатываться вызывающим кодом.
        virtual BKDResponse process(const BKDRequest& req) = 0;
    };

} // namespace bkd::emulators