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
    CpuArch.c \
    LzFind.c \
    Lzma2Dec.c \
    Lzma2Enc.c \
    LzmaDec.c \
    LzmaEnc.c \
    LzmaLib.c \

HEADERS += \
    7z.h \
    7zAlloc.h \
    7zBuf.h \
    7zCrc.h \
    7zFile.h \
    7zTypes.h \
    Alloc.h \
    Bcj2.h \
    Compiler.h \
    CpuArch.h \
    LzFind.h \
    LzHash.h \
    Lzma2Dec.h \
    Lzma2Enc.h \
    LzmaDec.h \
    LzmaEnc.h \
    LzmaLib.h \
    Precomp.h

equals(QMAKE_CXX, g++) {
QMAKE_CFLAGS += -Wno-format-truncation -Wno-stringop-truncation
}
QMAKE_CFLAGS += -DZ7_ST -DZ7_NO_METHODS_FILTERS -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g
