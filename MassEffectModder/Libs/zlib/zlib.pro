TEMPLATE = lib
CONFIG += staticlib warn_off

QT -= gui core

SOURCES += \
    adler32.c \
    compress.c \
    crc32.c \
    deflate.c \
    infback.c \
    inffast.c \
    inflate.c \
    inftrees.c \
    ioapi.c \
    iomemapi.c \
    trees.c \
    uncompr.c \
    unzip-ioapi.c \
    unzip.c \
    zutil.c

HEADERS += \
    crc32.h \
    crypt.h \
    deflate.h \
    gzguts.h \
    inffast.h \
    inffixed.h \
    inflate.h \
    inftrees.h \
    ioapi.h \
    iomemapi.h \
    trees.h \
    unzip.h \
    zconf.h \
    zlib.h \
    zutil.h

win32: {
    SOURCES += iowin32.c
    HEADERS += iowin32.h
}

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

DEFINES += Z_HAVE_UNISTD_H Z_HAVE_STDARG_H
