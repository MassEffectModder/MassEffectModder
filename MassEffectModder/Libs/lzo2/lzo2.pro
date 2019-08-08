TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    lzo_init.c \
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

QMAKE_CFLAGS += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g
