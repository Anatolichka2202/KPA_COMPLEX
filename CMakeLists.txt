cmake_minimum_required(VERSION 3.16)
project(kpa_complex VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Boost
set(BOOST_ROOT "C:/boost_1_90_0")
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost REQUIRED)

# Qt
find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets Charts GraphsWidgets)

# Добавляем поддиректории (важно: TunnelLib должна быть до bkd_network)
add_subdirectory(src/core)
add_subdirectory(src/network/TunnelLib)   # <-- добавить ДО network
add_subdirectory(src/network)             # теперь bkd_network может линковаться с tunnel
add_subdirectory(src/emulators)
add_subdirectory(src/gui)
add_subdirectory(src/logging)
add_subdirectory(src/matlab_manager)

# Исполняемый файл
add_executable(kpa_complex src/main.cpp)

# Копирование WinDivert.dll и .sys в папку с exe
set(WINDIVERT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/network/TunnelLib")
if(EXISTS "${WINDIVERT_DIR}/WinDivert.dll" AND EXISTS "${WINDIVERT_DIR}/WinDivert64.sys")
    add_custom_command(TARGET kpa_complex POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${WINDIVERT_DIR}/WinDivert.dll"
            "${WINDIVERT_DIR}/WinDivert64.sys"
            $<TARGET_FILE_DIR:kpa_complex>
        COMMENT "Copying WinDivert files"
    )
else()
    message(WARNING "WinDivert.dll or WinDivert64.sys not found in ${WINDIVERT_DIR}")
endif()

if(WIN32)
    target_link_libraries(kpa_complex PRIVATE ws2_32)
endif()

target_compile_definitions(kpa_complex PRIVATE BOOST_ASIO_HEADER_ONLY)

target_link_libraries(kpa_complex
    PRIVATE
    bkd_core
    bkd_network
    bkd_emulators
    kpa_gui
    Qt6::Core
    Qt6::Widgets
    Qt6::GraphsWidgets
)
