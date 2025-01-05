TEMPLATE = lib
CONFIG += staticlib warn_off

QT -= gui core

SOURCES += \
    Codec_DXTC_Alpha.cpp \
    Codec_DXTC_RGBA.cpp \
    CompressonatorXCodec.cpp

HEADERS += \
    Common.h \
    CompressonatorXCodec.h

DEFINES += USE_SSE USE_SSE2

QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -g1
QMAKE_CXXFLAGS_DEBUG += -g

equals(WRAPPERS_SHARED, true) {

QMAKE_CXXFLAGS += -DEXPORT_LIBS

}
