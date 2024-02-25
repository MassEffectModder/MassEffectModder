TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
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

macos {
    CONFIG(debug, debug|release) {
        contains(QT_ARCH, x86_64) {
            QMAKE_APPLE_DEVICE_ARCHS=x86_64h
            SOURCES += \
                filter_sse2_intrinsics.c \
                intel_init.c
        }
        contains(QT_ARCH, arm64) {
            QMAKE_APPLE_DEVICE_ARCHS=arm64
            SOURCES += \
                filter_neon_intrinsics.c \
                palette_neon_intrinsics.c \
                arm_init.c
        }
    } else {
    SOURCES += \
        filter_sse2_intrinsics.c \
        filter_neon_intrinsics.c \
        palette_neon_intrinsics.c \
        intel_init.c \
        arm_init.c
    }
} else {
    SOURCES += \
        filter_sse2_intrinsics.c \
        filter_neon_intrinsics.c \
        palette_neon_intrinsics.c \
        intel_init.c \
        arm_init.c
}

QMAKE_CFLAGS += -O3 -I../zlib -Wno-unused-parameter
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g

DEFINES += HAVE_CONFIG_H
