TEMPLATE = lib
CONFIG += staticlib

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

QMAKE_CFLAGS += -Wno-implicit-fallthrough

DEFINES += Z_HAVE_UNISTD_H Z_HAVE_STDARG_H

macx {
    # macOS clang doesn't have OpenMP enabled
    # we need provide version with enabled
    # brew version setup:
    QMAKE_CC  = /usr/local/opt/llvm/bin/clang
    QMAKE_CXX = /usr/local/opt/llvm/bin/clang++
    QMAKE_LIBDIR += /usr/local/opt/llvm/lib
}
