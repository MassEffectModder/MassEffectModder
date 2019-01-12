TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    dmc_unrar.c

HEADERS +=

DEFINES +=

#QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS_RELEASE += -g1
QMAKE_CXXFLAGS_DEBUG += -g
