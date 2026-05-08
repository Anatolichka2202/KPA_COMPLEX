cmake_minimum_required(VERSION 3.16)
project(kpa_complex_gui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Указываем путь к Boost
set(BOOST_ROOT "C:/boost_1_90_0")

# Запрещаем использование конфигурационных файлов Boost (BoostConfig.cmake)
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost REQUIRED)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets Charts)

# Создаём исполняемый файл
add_executable(kpa_complex_gui
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    block.h block.cpp block.ui                      # файлы блока
    widgets/angle_guard/advancedgauge.h
    widgets/angle_guard/advancedgauge.cpp
    widgets/angle_guard/gaugewidget.h
    widgets/angle_guard/gaugewidget.cpp
    widgets/angle_guard/stepslider.h
    widgets/angle_guard/stepslider.cpp
    core/emulator.h core/emulator.cpp
    core/timer.cpp
    blockmodel.h blockmodel.cpp

)
# Добавляем пути для включения (для компилятора и для uic)
target_include_directories(kpa_complex_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/angle_guard
)

# Чтобы AUTOUIC (uic) тоже находил заголовки при генерации ui_*.h, нужно добавить пути поиска
set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/angle_guard
)

target_compile_definitions(kpa_complex_gui PRIVATE BOOST_ASIO_HEADER_ONLY)

target_link_libraries(kpa_complex_gui PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    Boost::boost
)

# Если используешь Qt Graphs (Qt6::Graphs) – добавь в find_package и link
