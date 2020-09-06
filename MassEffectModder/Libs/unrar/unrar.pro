TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    unpack.c

HEADERS += \
    dmc_unrar.c

DEFINES +=

equals(QMAKE_CXX, g++) {
QMAKE_CFLAGS += -Wno-format-truncation -Wno-stringop-truncation
}
QMAKE_CFLAGS += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g
