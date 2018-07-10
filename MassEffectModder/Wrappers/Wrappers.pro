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

macx {
    # macOS clang doesn't have OpenMP enabled
    # we need provide version with enabled
    # brew version setup:
    QMAKE_CC  = /usr/local/opt/llvm/bin/clang
    QMAKE_CXX = /usr/local/opt/llvm/bin/clang++
    QMAKE_LIBDIR += /usr/local/opt/llvm/lib
}

win32:!win32-g++: {
    PRE_TARGETDEPS += \
    Release:$$OUT_PWD/../Libs/dxtc/release/libdxtc.a \
    Release:$$OUT_PWD/../Libs/lzma/release/liblzma.a \
    Release:$$OUT_PWD/../Libs/lzo2/release/liblzo2.a \
    Release:$$OUT_PWD/../Libs/xdelta3/release/libxdelta3.a \
    Release:$$OUT_PWD/../Libs/zlib/release/libzlib.a
    Debug:$$OUT_PWD/../Libs/dxtc/debug/libdxtc.a \
    Debug:$$OUT_PWD/../Libs/lzma/debug/liblzma.a \
    Debug:$$OUT_PWD/../Libs/lzo2/debug/liblzo2.a \
    Debug:$$OUT_PWD/../Libs/xdelta3/debug/libxdelta3.a \
    Debug:$$OUT_PWD/../Libs/zlib/debug/libzlib.a
} else:unix|win32-g++: {
    PRE_TARGETDEPS += \
    $$OUT_PWD/../Libs/dxtc/libdxtc.a \
    $$OUT_PWD/../Libs/lzma/liblzma.a \
    $$OUT_PWD/../Libs/lzo2/liblzo2.a \
    $$OUT_PWD/../Libs/xdelta3/libxdelta3.a \
    $$OUT_PWD/../Libs/zlib/libzlib.a
}

INCLUDEPATH += $$PWD/../Libs/dxtc $$PWD/../Libs/lzma $$PWD/../Libs/lzo2 $$PWD/../Libs/xdelta3 $$PWD/../Libs/zlib

DEPENDPATH += $$PWD/../Libs/dxtc $$PWD/../Libs/lzma $$PWD/../Libs/lzo2 $$PWD/../Libs/xdelta3 $$PWD/../Libs/zlib

LIBS += \
    -L$$OUT_PWD/../Libs/dxtc/ -ldxtc \
    -L$$OUT_PWD/../Libs/lzma/ -llzma \
    -L$$OUT_PWD/../Libs/lzo2/ -llzo2 \
    -L$$OUT_PWD/../Libs/xdelta3/ -lxdelta3 \
    -L$$OUT_PWD/../Libs/zlib/ -lzlib