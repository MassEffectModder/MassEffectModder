TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    lzo1x_1o.c \
    lzo1x_d1.c \
    lzo1x_d2.c

HEADERS += \
    lzo\lzo1x.h \
    lzo\lzoconf.h \
    lzo\lzodefs.h \
    config1x.h \
    lzo_conf.h \
    lzo_dict.h \
    lzo_func.h \
    lzo_ptr.h \
    lzo_supp.h \

QMAKE_CFLAGS +=

macx {
    # macOS clang doesn't have OpenMP enabled
    # we need provide version with enabled
    # brew version setup:
    QMAKE_CC  = /usr/local/opt/llvm/bin/clang
    QMAKE_CXX = /usr/local/opt/llvm/bin/clang++
    QMAKE_LIBDIR += /usr/local/opt/llvm/lib
}
