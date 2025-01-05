TEMPLATE = lib
CONFIG += staticlib warn_off

QT -= gui core

SOURCES += \
    3dquant_vpc.cpp \
    bc7_decode.cpp \
    bc7_definitions.cpp \
    bc7_encode.cpp \
    bc7_library.cpp \
    bc7_partitions.cpp \
    bc7_utils.cpp \
    shake.cpp

HEADERS += \
    3dquant_constants.h \
    3dquant_vpc.h \
    bc7_decode.h \
    bc7_definitions.h \
    bc7_encode.h \
    bc7_library.h \
    bc7_partitions.h \
    bc7_utils.h \
    shake.h

QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -g1
QMAKE_CXXFLAGS_DEBUG += -g

equals(WRAPPERS_SHARED, true) {

QMAKE_CXXFLAGS += -DEXPORT_LIBS

}
