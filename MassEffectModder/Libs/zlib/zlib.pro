TEMPLATE = lib
CONFIG += staticlib warn_off

QT -= gui core

SOURCES += \
    adler32.c \
    compress.c \
    crc32.c \
    deflate_fast.c \
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
    match.h \
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

win32-g++: {
    SOURCES += iowin32.c
    HEADERS += iowin32.h
}

QMAKE_CFLAGS += -O3
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g

DEFINES += Z_HAVE_UNISTD_H Z_HAVE_STDARG_H
