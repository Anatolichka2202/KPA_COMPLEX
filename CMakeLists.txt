cmake_minimum_required(VERSION 3.16)
project(kpa_complex VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем путь к Boost
set(BOOST_ROOT "C:/boost_1_90_0")

# Запрещаем использование конфигурационных файлов Boost (BoostConfig.cmake)
set(Boost_NO_BOOST_CMAKE ON)

# Ищем Boost без указания компонентов (system теперь header-only)
find_package(Boost REQUIRED)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets)

# Добавить поддиректории
add_subdirectory(src/core)
add_subdirectory(src/network)
add_subdirectory(src/emulators)
#add_subdirectory(src/gui)
add_subdirectory(src/logging)
add_subdirectory(src/matlab_manager)

# Исполняемый файл
add_executable(kpa_complex src/main.cpp)

# Для Boost.Asio в header-only режиме добавим определение
target_compile_definitions(kpa_complex PRIVATE BOOST_ASIO_HEADER_ONLY)

target_link_libraries(kpa_complex
    PRIVATE
    bkd_core
    bkd_network
    bkd_emulators
    Qt6::Core
    Qt6::Widgets
    # Boost::system больше не линкуем, так как он header-only
)

