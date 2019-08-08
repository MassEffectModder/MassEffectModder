TEMPLATE = lib
CONFIG += staticlib warn_off

QT -= gui core

HEADERS += \
    xdelta3-blkcache.h \
    xdelta3-decode.h \
    xdelta3-djw.h \
    xdelta3-fgk.h \
    xdelta3-hash.h \
    xdelta3-internal.h \
    xdelta3-list.h \
    xdelta3-lzma.h \
    xdelta3-main.h \
    xdelta3-merge.h \
    xdelta3-second.h \
    xdelta3-test.h \
    xdelta3-cfgs.h \
    xdelta3.h

SOURCES = \
    xdelta3_lib.c

QMAKE_CFLAGS += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g

DEFINES += \
    HAVE_CONFIG_H \
    REGRESSION_TEST=0 \
    SHELL_TESTS=0 \
    EXTERNAL_COMPRESSION=0 \
    SECONDARY_DJW=1 \
    SECONDARY_FGK=1 \
    SECONDARY_LZMA=0 \
    XD3_DEBUG=0 \
    XD3_MAIN=0 \
    XD3_ENCODER=1 \
    XD3_WIN32=0 \
    XD3_STDIO=0 \
    XD3_POSIX=0 \
    VCDIFF_TOOLS=0
