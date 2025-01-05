TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
        mspack/lzxd.c \
        unlzx.c

HEADERS += \
        mspack/lzx.h \
        mspack/mspack.h \
        mspack/readbits.h \
        mspack/readhuff.h \
        mspack/system.h

DEFINES += HAVE_INTTYPES_H=1 SIZEOF_OFF_T=8

QMAKE_CFLAGS += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g
