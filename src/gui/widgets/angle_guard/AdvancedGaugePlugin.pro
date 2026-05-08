QT += widgets designer

TEMPLATE = lib
CONFIG += designer plugin release
TARGET = $$qtLibraryTarget(advancedgaugeplugin)

DEFINES += ADVANCEDGAUGEPLUGIN_LIBRARY

SOURCES += \
    advancedgauge.cpp \
    advancedgaugeplugin.cpp \
    gaugewidget.cpp \
    stepslider.cpp

HEADERS += \
    advancedgauge.h \
    advancedgaugeplugin.h \
    gaugewidget.h \
    stepslider.h

# Путь установки (скопируется в папку designer вашего Qt)
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

######
#QT += core widgets gui

#TARGET = GaugeDemo
#TEMPLATE = app

#CONFIG += c++17
#CONFIG += console   # оставьте, если нужен вывод qDebug; для финальной версии можно убрать

#SOURCES += \
 #   main.cpp \
  #  advancedgauge.cpp \
   # gaugewidget.cpp \
    #stepslider.cpp

#HEADERS += \
 #   advancedgauge.h \
   # gaugewidget.h \
    #stepslider.h

# Предупреждения об устаревших API (опционально)
#DEFINES += QT_DEPRECATED_WARNINGS

# Для Windows: скрыть консольное окно (раскомментируйте, если не нужна отладка)
# win32: CONFIG -= console
