TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    bink.c

HEADERS += \
    binkdata.h

QMAKE_CFLAGS += -O3 -std=c11 -fomit-frame-pointer -fPIC -pthread -fno-math-errno -fno-signed-zeros -mstack-alignment=16 -Qunused-arguments
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g

DEFINES += HAVE_AV_CONFIG_H _ISOC99_SOURCE _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _THREAD_SAFE PIC
