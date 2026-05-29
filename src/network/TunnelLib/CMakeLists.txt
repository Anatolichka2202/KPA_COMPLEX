cmake_minimum_required(VERSION 3.15)
project(TunnelLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Npcap SDK (ручная настройка) ----
if(NOT DEFINED Packet_ROOT)
    set(Packet_ROOT "C:/tools/npcap-sdk" CACHE PATH "Path to Npcap SDK")
endif()

set(Packet_INCLUDE_DIRS "${Packet_ROOT}/Include")
set(Packet_LIBRARY_DIR "${Packet_ROOT}/Lib/x64")

if(NOT EXISTS "${Packet_INCLUDE_DIRS}/pcap.h")
    message(FATAL_ERROR "pcap.h not found in ${Packet_INCLUDE_DIRS}. Check Packet_ROOT")
endif()

set(Packet_LIBRARIES
    "${Packet_LIBRARY_DIR}/wpcap.lib"
    "${Packet_LIBRARY_DIR}/packet.lib"
)

# ---- WinDivert ----
# По умолчанию ищем в папке lib (рядом с проектом) или в lib/windivert
if(NOT DEFINED WINDIVERT_ROOT)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/windivert.h")
        set(WINDIVERT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib" CACHE PATH "Path to WinDivert" FORCE)
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/windivert/windivert.h")
        set(WINDIVERT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/windivert" CACHE PATH "Path to WinDivert" FORCE)
    else()
        set(WINDIVERT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/windivert" CACHE PATH "Path to WinDivert")
    endif()
endif()

# Путь к заголовку
find_path(WINDIVERT_INCLUDE_DIR windivert.h PATHS "${WINDIVERT_ROOT}" "${WINDIVERT_ROOT}/include" REQUIRED)

# Путь к библиотеке (windivert.lib или libwindivert.a)
find_library(WINDIVERT_LIBRARY NAMES windivert PATHS "${WINDIVERT_ROOT}" "${WINDIVERT_ROOT}/lib" REQUIRED)

# ---- Основная статическая библиотека ----
add_library(tunnel STATIC
    src/arp_spoofer.cpp
    src/packet_modifier.cpp
    src/tunnel_lib.cpp
)

target_include_directories(tunnel PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${Packet_INCLUDE_DIRS}
    ${WINDIVERT_INCLUDE_DIR}
)

target_link_libraries(tunnel
    ${Packet_LIBRARIES}
    ${WINDIVERT_LIBRARY}
    ws2_32
    iphlpapi
)
target_compile_definitions(tunnel PRIVATE HAVE_REMOTE NOMINMAX)
target_compile_definitions(tunnel PRIVATE NOMINMAX)
target_compile_definitions(tunnel PRIVATE HAVE_REMOTE)
target_compile_definitions(tunnel PRIVATE HAVE_REMOTE NOMINMAX WIN32_LEAN_AND_MEAN)
target_compile_definitions(tunnel PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)

# ---- Пример исполняемого файла ----
add_executable(tunnel_example main.cpp)
target_link_libraries(tunnel_example tunnel)

# ---- Автоматическое копирование WinDivert.dll и WinDivert64.sys ----
# Определим, где лежат файлы .dll и .sys (обычно рядом с заголовком)
get_filename_component(WINDIVERT_BIN_DIR "${WINDIVERT_INCLUDE_DIR}" DIRECTORY)

# Убедимся, что файлы существуют, и скопируем их
if(EXISTS "${WINDIVERT_BIN_DIR}/WinDivert.dll")
    set(WINDIVERT_DLL "${WINDIVERT_BIN_DIR}/WinDivert.dll")
elseif(EXISTS "${WINDIVERT_ROOT}/WinDivert.dll")
    set(WINDIVERT_DLL "${WINDIVERT_ROOT}/WinDivert.dll")
endif()

if(EXISTS "${WINDIVERT_BIN_DIR}/WinDivert64.sys")
    set(WINDIVERT_SYS "${WINDIVERT_BIN_DIR}/WinDivert64.sys")
elseif(EXISTS "${WINDIVERT_ROOT}/WinDivert64.sys")
    set(WINDIVERT_SYS "${WINDIVERT_ROOT}/WinDivert64.sys")
elseif(EXISTS "${WINDIVERT_BIN_DIR}/WinDivert.sys")
    set(WINDIVERT_SYS "${WINDIVERT_BIN_DIR}/WinDivert.sys")
elseif(EXISTS "${WINDIVERT_ROOT}/WinDivert.sys")
    set(WINDIVERT_SYS "${WINDIVERT_ROOT}/WinDivert.sys")
endif()

if(WINDIVERT_DLL AND WINDIVERT_SYS)
    add_custom_command(TARGET tunnel_example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${WINDIVERT_DLL}"
            "${WINDIVERT_SYS}"
            $<TARGET_FILE_DIR:tunnel_example>
        COMMENT "Copying WinDivert.dll and WinDivert.sys to output directory"
    )
else()
    message(WARNING "WinDivert.dll or .sys not found, skipping copy. You must manually place them next to the executable.")
endif()

# ---- Установка (опционально) ----
install(TARGETS tunnel tunnel_example
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)
