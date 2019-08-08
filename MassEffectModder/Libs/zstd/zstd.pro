TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
           debug.c \
           entropy_common.c \
           error_private.c \
           fse_compress.c \
           fse_decompress.c \
           hist.c \
           huf_compress.c \
           huf_decompress.c \
           pool.c \
           threading.c \
           hash.c \
           zstd_common.c \
           zstd_compress.c \
           zstd_ddict.c \
           zstd_decompress.c \
           zstd_decompress_block.c \
           zstd_double_fast.c \
           zstd_fast.c \
           zstd_lazy.c \
           zstd_ldm.c \
           zstd_opt.c

HEADERS += \
           bitstream.h \
           compiler.h \
           cpu.h \
           debug.h \
           error_private.h \
           fse.h \
           hist.h \
           huf.h \
           mem.h \
           pool.h \
           threading.h \
           xxhash.h \
           zstd_compress_internal.h \
           zstd_ddict.h \
           zstd_decompress_block.h \
           zstd_decompress_internal.h \
           zstd_double_fast.h \
           zstd_errors.h \
           zstd_fast.h \
           zstd_internal.h \
           zstd_lazy.h \
           zstd_ldm.h \
           zstd_opt.h

QMAKE_CFLAGS += -O3 -D ZSTD_STRIP_ERROR_STRINGS \
            -Wall -Wextra -Wcast-qual -Wcast-align -Wshadow \
            -Wstrict-aliasing=1 -Wswitch-enum -Wdeclaration-after-statement \
            -Wstrict-prototypes -Wundef -Wpointer-arith \
            -Wvla -Wformat=2 -Winit-self -Wfloat-equal -Wwrite-strings \
            -Wredundant-decls -Wmissing-prototypes -Wc++-compat

QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g
