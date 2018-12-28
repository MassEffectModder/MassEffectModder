TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    filter_sse2_intrinsics.c \
    intel_init.c \
    png.c \
    pngerror.c \
    pngget.c \
    pngmem.c \
    pngpread.c \
    pngread.c \
    pngrio.c \
    pngrtran.c \
    pngrutil.c \
    pngset.c \
    pngtrans.c \
    pngwio.c \
    pngwrite.c \
    pngwtran.c \
    pngwutil.c

HEADERS += \
    config.h \
    png.h \
    pngconf.h \
    pngdebug.h \
    pnginfo.h \
    pnglibconf.h \
    pngprefix.h \
    pngpriv.h \
    pngstruct.h

QMAKE_CFLAGS += -O2 -I../zlib -Wno-unused-parameter
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g

DEFINES += HAVE_CONFIG_H
