TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    WrapperLzma.cpp \
    WrapperLzo.cpp \
    WrapperPng.cpp \
    WrapperUnzip.cpp \
    WrapperXdelta.cpp \
    WrapperZlib.cpp

HEADERS += Wrappers.h

QMAKE_CXXFLAGS +=
QMAKE_CXXFLAGS_RELEASE += -g1
QMAKE_CXXFLAGS_DEBUG += -g

DEFINES +=

win32-g++: {
    Release:PRE_TARGETDEPS += \
    $$OUT_PWD/../Libs/7zdec/release/lib7zdec.a \
    $$OUT_PWD/../Libs/dxtc/release/libdxtc.a \
    $$OUT_PWD/../Libs/lzo2/release/liblzo2.a \
    $$OUT_PWD/../Libs/png/release/libpng.a \
    $$OUT_PWD/../Libs/xdelta3/release/libxdelta3.a \
    $$OUT_PWD/../Libs/zlib/release/libzlib.a
    Debug:PRE_TARGETDEPS += \
    $$OUT_PWD/../Libs/7zdec/debug/lib7zdec.a \
    $$OUT_PWD/../Libs/dxtc/debug/libdxtc.a \
    $$OUT_PWD/../Libs/lzo2/debug/liblzo2.a \
    $$OUT_PWD/../Libs/png/debug/libpng.a \
    $$OUT_PWD/../Libs/xdelta3/debug/libxdelta3.a \
    $$OUT_PWD/../Libs/zlib/debug/libzlib.a
} else:unix: {
    PRE_TARGETDEPS += \
    $$OUT_PWD/../Libs/7zdec/lib7zdec.a \
    $$OUT_PWD/../Libs/dxtc/libdxtc.a \
    $$OUT_PWD/../Libs/lzo2/liblzo2.a \
    $$OUT_PWD/../Libs/png/libpng.a \
    $$OUT_PWD/../Libs/xdelta3/libxdelta3.a \
    $$OUT_PWD/../Libs/zlib/libzlib.a
}

INCLUDEPATH += $$PWD/../Libs/7zdec $$PWD/../Libs/dxtc $$PWD/../Libs/lzo2 $$PWD/../Libs/png $$PWD/../Libs/xdelta3 $$PWD/../Libs/zlib

DEPENDPATH += $$PWD/../Libs/7zdec $$PWD/../Libs/dxtc $$PWD/../Libs/lzo2 $$PWD/../Libs/png $$PWD/../Libs/xdelta3 $$PWD/../Libs/zlib
