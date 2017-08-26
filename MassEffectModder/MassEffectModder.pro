QT += core gui widgets

CONFIG += c++14 static

TARGET = MassEffectModder

TEMPLATE = app

SOURCES += Main.cpp \
    Exceptions/SignalHandler.cpp \
    Gui/MainWindow.cpp \
    Gui/LayoutMeSelect.cpp \
    Logs/Logs.cpp \
    ConfigIni.cpp

HEADERS += \
    Exceptions/SignalHandler.h \
    Gui/MainWindow.h \
    Gui/LayoutMeSelect.h \
    Logs/Logs.h \
    ConfigIni.h

DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS += -fopenmp

macx {
    QMAKE_CC  = /usr/local/opt/llvm/bin/clang
    QMAKE_CXX = /usr/local/opt/llvm/bin/clang++
}
