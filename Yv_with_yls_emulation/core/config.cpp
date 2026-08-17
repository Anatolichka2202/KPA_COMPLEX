#include "config.h"

namespace YAV {

bool Config::isValid() const {
    if (yls_mode == YLSMode::REAL) {
        if (real_yls.ip.empty() || real_yls.port == 0) {
            return false;
        }
    }
    // Для эмулированного режима всегда валидно
    return true;
}



} // namespace YAV
