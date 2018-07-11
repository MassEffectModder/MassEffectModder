TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    WrapperLzma.cpp \
    WrapperLzo.cpp \
    WrapperUnzip.cpp \
    WrapperXdelta.cpp \
    WrapperZlib.cpp

HEADERS += Wrappers.h

QMAKE_CFLAGS +=

DEFINES +=

win32-g++: {
    Release:PRE_TARGETDEPS += \
    $$OUT_PWD/../Libs/dxtc/release/libdxtc.a \
    $$OUT_PWD/../Libs/lzma/release/liblzma.a \
    $$OUT_PWD/../Libs/lzo2/release/liblzo2.a \
    $$OUT_PWD/../Libs/xdelta3/release/libxdelta3.a \
    $$OUT_PWD/../Libs/zlib/release/libzlib.a
    Debug:PRE_TARGETDEPS += \
    $$OUT_PWD/../Libs/dxtc/debug/libdxtc.a \
    $$OUT_PWD/../Libs/lzma/debug/liblzma.a \
    $$OUT_PWD/../Libs/lzo2/debug/liblzo2.a \
    $$OUT_PWD/../Libs/xdelta3/debug/libxdelta3.a \
    $$OUT_PWD/../Libs/zlib/debug/libzlib.a
} else:unix: {
    PRE_TARGETDEPS += \
    $$OUT_PWD/../Libs/dxtc/libdxtc.a \
    $$OUT_PWD/../Libs/lzma/liblzma.a \
    $$OUT_PWD/../Libs/lzo2/liblzo2.a \
    $$OUT_PWD/../Libs/xdelta3/libxdelta3.a \
    $$OUT_PWD/../Libs/zlib/libzlib.a
}

INCLUDEPATH += $$PWD/../Libs/dxtc $$PWD/../Libs/lzma $$PWD/../Libs/lzo2 $$PWD/../Libs/xdelta3 $$PWD/../Libs/zlib

DEPENDPATH += $$PWD/../Libs/dxtc $$PWD/../Libs/lzma $$PWD/../Libs/lzo2 $$PWD/../Libs/xdelta3 $$PWD/../Libs/zlib
