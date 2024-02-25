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

macos {
    CONFIG(debug, debug|release) {
        contains(QT_ARCH, x86_64) {
            QMAKE_APPLE_DEVICE_ARCHS=x86_64h
        }
        contains(QT_ARCH, arm64) {
            QMAKE_APPLE_DEVICE_ARCHS=arm64
        }
    }
}

QMAKE_CFLAGS += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g
