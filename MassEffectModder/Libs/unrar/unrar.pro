TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    unpack.c

HEADERS += \
    dmc_unrar.c

DEFINES +=

#QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS_RELEASE += -g1
QMAKE_CXXFLAGS_DEBUG += -g
