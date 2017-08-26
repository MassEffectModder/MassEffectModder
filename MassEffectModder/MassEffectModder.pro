QT += core gui widgets

CONFIG += c++14 static precompile_header
CONFIG -= app_bundle

TARGET = MassEffectModder

TEMPLATE = app

SOURCES += Main.cpp \
    Exceptions/SignalHandler.cpp \
    Gui/MainWindow.cpp \
    Gui/LayoutMeSelect.cpp \
    Logs/Logs.cpp \
    ConfigIni.cpp

PRECOMPILED_HEADER = Precompiled.h

HEADERS += \
    Exceptions/SignalHandler.h \
    Gui/MainWindow.h \
    Gui/LayoutMeSelect.h \
    Logs/Logs.h \
    ConfigIni.h

DEFINES += QT_DEPRECATED_WARNINGS

precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}

QMAKE_CXXFLAGS += -fopenmp

macx {
    QMAKE_CC  = /usr/local/opt/llvm/bin/clang
    QMAKE_CXX = /usr/local/opt/llvm/bin/clang++
    PRECOMPILED_DIR = ".pch"
}
