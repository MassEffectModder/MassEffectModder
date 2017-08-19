QT += core gui widgets

CONFIG += c++14 static

TARGET = MassEffectModder

TEMPLATE = app

SOURCES += Main.cpp \
    Gui/MainWindow.cpp \
    Gui/LayoutMeSelect.cpp \
    ConfigIni.cpp

HEADERS += \
    Gui/MainWindow.h \
    Gui/LayoutMeSelect.h \
    ConfigIni.h

DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS += -fopenmp

macx {
    QMAKE_CC  = /usr/local/opt/llvm/bin/clang
    QMAKE_CXX = /usr/local/opt/llvm/bin/clang++
}
