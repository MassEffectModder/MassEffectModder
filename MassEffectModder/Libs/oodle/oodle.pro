TEMPLATE = lib
CONFIG += staticlib warn_off

QT -= gui core

SOURCES += \
    oodle.cpp

HEADERS += \
    oodle.h

QMAKE_CFLAGS += -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g

DEFINES +=
