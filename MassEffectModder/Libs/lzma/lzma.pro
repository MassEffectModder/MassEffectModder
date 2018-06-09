TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    Alloc.c \
    7zAlloc.c \
    LzmaDec.c \
    LzmaLib.c

HEADERS += \
    Alloc.h \
    7zAlloc.h \
    7zTypes.h \
    Compiler.h \
    LzmaDec.h \
    LzmaLib.h \
    Precomp.h

QMAKE_CFLAGS +=

macx {
    # macOS clang doesn't have OpenMP enabled
    # we need provide version with enabled
    # brew version setup:
    QMAKE_CC  = /usr/local/opt/llvm/bin/clang
    QMAKE_CXX = /usr/local/opt/llvm/bin/clang++
    QMAKE_LIBDIR += /usr/local/opt/llvm/lib
}
