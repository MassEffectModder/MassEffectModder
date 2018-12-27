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

QMAKE_CFLAGS += -O3
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g
