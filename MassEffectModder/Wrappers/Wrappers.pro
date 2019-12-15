TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    BacktraceCommon.cpp \
    Wrapper7Zip.cpp \
    WrapperLzo.cpp \
    WrapperPng.cpp \
    WrapperUnzip.cpp \
    WrapperUnrar.cpp \
    WrapperXdelta.cpp \
    WrapperZlib.cpp

equals(ZSTD_ENABLE, true) {
SOURCES += \
    WrapperZstd.cpp
}

macx {
    SOURCES += BacktraceMac.cpp
}

win32 {
    SOURCES += BacktraceWin.cpp
}

linux {
    SOURCES += BacktraceLin.cpp
}

HEADERS += Wrappers.h

QMAKE_CXXFLAGS +=
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -g1 -O3
QMAKE_CXXFLAGS_DEBUG += -g

DEFINES +=

INCLUDEPATH += \
    $$PWD/../Libs/7z \
    $$PWD/../Libs/bfd \
    $$PWD/../Libs/dxtc \
    $$PWD/../Libs/lzo2 \
    $$PWD/../Libs/png \
    $$PWD/../Libs/xdelta3 \
    $$PWD/../Libs/zlib \
    $$PWD/../Libs/zstd \
    $$PWD/../Libs/unrar

DEPENDPATH += \
    $$PWD/../Libs/7z \
    $$PWD/../Libs/bfd \
    $$PWD/../Libs/dxtc \
    $$PWD/../Libs/lzo2 \
    $$PWD/../Libs/png \
    $$PWD/../Libs/xdelta3 \
    $$PWD/../Libs/zlib \
    $$PWD/../Libs/zstd \
    $$PWD/../Libs/unrar
