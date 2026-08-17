#pragma once

#include "config.h"
#include <string>

namespace YAV {

class ConfigManager {
public:
    // Загрузить конфигурацию из файла
    static Config load(const std::string& filename = "yv_config.ini");

    // Сохранить конфигурацию в файл
    static void save(const Config& config, const std::string& filename = "yv_config.ini");

    // Создать конфигурацию по умолчанию
    static Config createDefault();
};

} // namespace YAV
