#include "configmanager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace {

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return str.substr(start, end - start + 1);
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool parseBool(const std::string& value) {
    std::string val = toLower(trim(value));
    return (val == "true" || val == "1" || val == "yes" || val == "on");
}

} // namespace

namespace YAV {

Config ConfigManager::load(const std::string& filename) {
    Config config;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Config file not found: " << filename
                  << ". Using defaults." << std::endl;
        return createDefault();
    }

    std::string line;
    std::string current_section;

    int line_num = 0;
    while (std::getline(file, line)) {
        line_num++;
        //  СЫРУЮ СТРОКУ С НОМЕРОМ
        std::cout << "Line " << line_num << " raw: '";
        for (char c : line) {
            if (c == '\t') std::cout << "\\t";
            else if (c == '\r') std::cout << "\\r";
            else if (c == '\n') std::cout << "\\n";
            else if (c < 32) std::cout << "[" << (int)c << "]";
            else std::cout << c;
        }
        std::cout << "'" << std::endl;

        line = trim(line);

         std::cout << "Read line: '" << line << "'" << std::endl;

        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        if (line[0] == '[' && line.back() == ']') {
            current_section = toLower(line.substr(1, line.size() - 2));
            continue;
        }

        size_t eq_pos = line.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = trim(line.substr(0, eq_pos));
            std::string value = trim(line.substr(eq_pos + 1));
            std::string key_lower = toLower(key);

            // [yav] section
            if (current_section == "yav") {
                if (key_lower == "name") {
                    config.name = value;
                }
                else if (key_lower == "yls_mode") {
                    std::string mode = toLower(value);


                    // ////////////////////////////////////////////////////////////////////////////
                    // КРИТИЧЕСКАЯ ОТЛАДКА - ВЫВОДИМ ВСЁ!
                    std::cout << "\n!!!!!! DEBUG yls_mode parsing !!!!!!" << std::endl;
                    std::cout << "Original value: '" << value << "'" << std::endl;
                    std::cout << "Length: " << value.length() << std::endl;
                    std::cout << "Hex dump: ";
                    for (size_t i = 0; i < value.length(); ++i) {
                        std::cout << std::hex << (int)(unsigned char)value[i] << " ";
                    }
                    std::cout << std::dec << std::endl;

                    // Выводим ASCII символы
                    std::cout << "ASCII: ";
                    for (size_t i = 0; i < value.length(); ++i) {
                        char c = value[i];
                        if (c >= 32 && c <= 126) {
                            std::cout << c << " ";
                        } else {
                            std::cout << "[" << (int)c << "] ";
                        }
                    }
                    std::cout << std::endl;

                    // Проверяем наличие подчеркивания
                    size_t underscore_pos = value.find('_');
                    if (underscore_pos != std::string::npos) {
                        std::cout << "Found underscore at position: " << underscore_pos << std::endl;
                        std::cout << "Before underscore: '" << value.substr(0, underscore_pos) << "'" << std::endl;
                        std::cout << "After underscore: '" << value.substr(underscore_pos + 1) << "'" << std::endl;
                    } else {
                        std::cout << "NO UNDERSCORE FOUND in value!" << std::endl;
                    }

                    std::cout << "Lowercase mode: '" << mode << "'" << std::endl;

                                // //////////////////////////////////////


                    if (mode == "real") {
                        config.yls_mode = Config::YLSMode::REAL;

                    } else if (mode == "emulate_yls") {
                        config.yls_mode = Config::YLSMode::EMULATE_YLS;

                    } else {
                        config.yls_mode = Config::YLSMode::FULL_EMULATE; // default
                    std::cout <<"Defult settings FullEmulation";
                    }
                }
                else if (key_lower == "log_file") {
                    config.log_file = value;
                }
            }

            // [real_yls] section
            else if (current_section == "real_yls") {
                if (key_lower == "ip") {
                    config.real_yls.ip = value;
                }
                else if (key_lower == "port") {
                    config.real_yls.port = static_cast<uint16_t>(std::stoi(value));
                }
                else if (key_lower == "timeout_ms") {
                    config.real_yls.timeout_ms = std::stoi(value);
                }
            }

            // [emulated_bkd] section
            else if (current_section == "emulated_bkd") {
                if (key_lower == "ip") {
                    config.emulated_bkd.ip = value;
                }
                else if (key_lower == "base_port") {
                    config.emulated_bkd.base_port = static_cast<uint16_t>(std::stoi(value));
                }
                else if (key_lower == "bkd_timeout_ms") {
                    config.emulated_bkd.bkd_timeout_ms = std::stoi(value);
                }
            }

            else if (current_section.find("bkd") == 0) {
                // Парсим секции вида [bkd0], [bkd1], ...
                std::string block_str = current_section.substr(3); // убираем "bkd"
                try {
                    int block = std::stoi(block_str);
                    if (block >= 0 && block < 12) {
                        if (key_lower == "ip") {
                            config.individual_bkd[block].ip = value;
                            config.individual_bkd[block].configured = true;
                        }
                        else if (key_lower == "port") {
                            config.individual_bkd[block].port = static_cast<uint16_t>(std::stoi(value));
                            config.individual_bkd[block].configured = true;
                        }
                    }
                } catch (...) {
                    // Игнорируем ошибки парсинга
                }
            }

            // [timing] section
            else if (current_section == "timing") {
                if (key_lower == "send_interval") {
                    config.timing.send_interval = std::stoi(value);
                }
                else if (key_lower == "gui_update_interval") {
                    config.timing.gui_update_interval = std::stoi(value);
                }
                else if (key_lower == "chart_update_interval") {
                    config.timing.chart_update_interval = std::stoi(value);
                }
            }

            // [logging] section
            else if (current_section == "logging") {
                if (key_lower == "enabled") {
                    config.logging.enabled = parseBool(value);
                }
                else if (key_lower == "log_first_packets") {
                    config.logging.log_first_packets = parseBool(value);
                }
                else if (key_lower == "max_packets_logged") {
                    config.logging.max_packets_logged = std::stoi(value);
                }
            }
        }
    }

    return config;
}

void ConfigManager::save(const Config& config, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot save config to: " << filename << std::endl;
        return;
    }

    file << "[yav]\n";
    file << "name = " << config.name << "\n";
    file << "yls_mode = " << (config.yls_mode == Config::YLSMode::REAL ? "real" : "emulate") << "\n";
    file << "log_file = " << config.log_file << "\n\n";

    file << "[real_yls]\n";
    file << "ip = " << config.real_yls.ip << "\n";
    file << "port = " << config.real_yls.port << "\n";
    file << "timeout_ms = " << config.real_yls.timeout_ms << "\n\n";

    file << "[emulated_bkd]\n";
    file << "ip = " << config.emulated_bkd.ip << "\n";
    file << "base_port = " << config.emulated_bkd.base_port << "\n\n";

    file << "[timing]\n";
    file << "send_interval = " << config.timing.send_interval << "\n";
    file << "gui_update_interval = " << config.timing.gui_update_interval << "\n";
    file << "chart_update_interval = " << config.timing.chart_update_interval << "\n\n";

    file << "[logging]\n";
    file << "enabled = " << (config.logging.enabled ? "true" : "false") << "\n";
    file << "log_first_packets = " << (config.logging.log_first_packets ? "true" : "false") << "\n";
    file << "max_packets_logged = " << config.logging.max_packets_logged << "\n";

    file.flush();
}

Config ConfigManager::createDefault() {
    Config config;
    // Устанавливаем таймаут БКД по умолчанию
    config.emulated_bkd.bkd_timeout_ms = 20;
    return config;
}

} // namespace YAV
