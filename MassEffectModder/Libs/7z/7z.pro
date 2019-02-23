TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    7zAlloc.c \
    7zArcIn.c \
    7zBuf.c \
    7zBuf2.c \
    7zCrc.c \
    7zCrcOpt.c \
    7zDec.c \
    7zFile.c \
    7zStream.c \
    7zUnpack.c \
    Alloc.c \
    Bcj2.c \
    Bra.c \
    Bra86.c \
    BraIA64.c \
    CpuArch.c \
    Delta.c \
    LzFind.c \
    Lzma2Dec.c \
    Lzma2Enc.c \
    LzmaDec.c \
    LzmaEnc.c \
    LzmaLib.c \
    Ppmd7.c \
    Ppmd7Dec.c

HEADERS += \
    7z.h \
    7zAlloc.h \
    7zBuf.h \
    7zCrc.h \
    7zFile.h \
    7zTypes.h \
    Alloc.h \
    Bcj2.h \
    Bra.h \
    Compiler.h \
    CpuArch.h \
    Delta.h \
    LzFind.h \
    LzHash.h \
    Lzma2Dec.h \
    Lzma2Enc.h \
    LzmaDec.h \
    LzmaEnc.h \
    LzmaLib.h \
    Ppmd7.h \
    Precomp.h

QMAKE_CFLAGS += -D_7ZIP_ST
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g
