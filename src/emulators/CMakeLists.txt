add_library(bkd_emulators STATIC
    # пока пусто, потом добавим SimpleEmulator и др.
    simple_emulator.cpp
)

target_include_directories(bkd_emulators PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(bkd_emulators PRIVATE bkd_core)
