cmake_minimum_required(VERSION 3.16)
project(YAV_GUI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим Qt (здесь, а не в поддиректориях)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts Network)

# Включаем поддиректории
add_subdirectory(core)
add_subdirectory(gui)