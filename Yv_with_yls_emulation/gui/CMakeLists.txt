add_executable(${PROJECT_NAME}


)

# Источники для GUI
target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
    mainwindow.cpp
    blockwidget.cpp
    qtudpclient.cpp  # Добавили Qt-реализацию
    networkfactory.cpp

)

# Заголовки для GUI
target_sources(${PROJECT_NAME} PRIVATE
    mainwindow.h
    blockwidget.h
    qtudpclient.h
    networkfactory.h
)

# UI формы
target_sources(${PROJECT_NAME} PRIVATE
    mainwindow.ui
    blockwidget.ui
)

# Директории include
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
)

# Линковка с библиотекой core и Qt
target_link_libraries(${PROJECT_NAME}
    yav_core  # Наша библиотека core
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network

)

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
