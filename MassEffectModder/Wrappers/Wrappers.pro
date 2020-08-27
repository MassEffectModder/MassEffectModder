TEMPLATE = lib

!equals(WRAPPERS_SHARED, true) {
CONFIG += staticlib
} else {
CONFIG -= staticlib
CONFIG += dll
}

QT -= gui core

!equals(WRAPPERS_SHARED, true) {
SOURCES += \
    BacktraceCommon.cpp \
}

SOURCES += \
    Wrapper7Zip.cpp \
    WrapperDxtc.cpp \
    WrapperLzo.cpp \
    WrapperUnzip.cpp \
    WrapperZlib.cpp

!equals(WRAPPERS_SHARED, true) {
SOURCES += \
    WrapperPng.cpp \
    WrapperUnrar.cpp \
    WrapperXdelta.cpp \
}

equals(ZSTD_ENABLE, true) {
SOURCES += \
    WrapperZstd.cpp
}

!equals(WRAPPERS_SHARED, true) {
    macx {
        SOURCES += BacktraceMac.cpp
    }

    win32 {
        SOURCES += BacktraceWin.cpp
    }

    linux {
        SOURCES += BacktraceLin.cpp
    }
}

HEADERS += Wrappers.h

!macx {
QMAKE_CXXFLAGS += -Wno-format-truncation -Wno-stringop-truncation
}
#QMAKE_CXXFLAGS +=
#QMAKE_CXXFLAGS_RELEASE -= -O2
#QMAKE_CXXFLAGS_RELEASE += -g1 -O3
#QMAKE_CXXFLAGS_DEBUG += -g

DEFINES +=

INCLUDEPATH += \
    $$PWD/../Libs/7z \
    $$PWD/../Libs/bfd \
    $$PWD/../Libs/dxtc \
    $$PWD/../Libs/lzo2 \
    $$PWD/../Libs/png \
    $$PWD/../Libs/xdelta3 \
    $$PWD/../Libs/zlib \
    $$PWD/../Libs/zstd \
    $$PWD/../Libs/unrar

DEPENDPATH += \
    $$PWD/../Libs/7z \
    $$PWD/../Libs/bfd \
    $$PWD/../Libs/dxtc \
    $$PWD/../Libs/lzo2 \
    $$PWD/../Libs/png \
    $$PWD/../Libs/xdelta3 \
    $$PWD/../Libs/zlib \
    $$PWD/../Libs/zstd \
    $$PWD/../Libs/unrar


equals(WRAPPERS_SHARED, true) {

QMAKE_CXXFLAGS += -DEXPORT_LIBS

win32-g++: {
Release:LIBS += \
    -L$$OUT_PWD/../Libs/7z/release -l7z \
    -L$$OUT_PWD/../Libs/dxtc/release -ldxtc \
    -L$$OUT_PWD/../Libs/lzo2/release -llzo2 \
    -L$$OUT_PWD/../Libs/zlib/release -lzlib

equals(ZSTD_ENABLE, true) {
    Release:LIBS += -L$$OUT_PWD/../Libs/zstd/release -lzstd
}
Debug:LIBS += \
    -L$$OUT_PWD/../Libs/7z/debug -l7z \
    -L$$OUT_PWD/../Libs/dxtc/debug -ldxtc \
    -L$$OUT_PWD/../Libs/lzo2/debug -llzo2 \
    -L$$OUT_PWD/../Libs/zlib/debug -lzlib

equals(ZSTD_ENABLE, true) {
    Debug:LIBS += -L$$OUT_PWD/../Libs/zstd/debug -lzstd
}
} else:unix: {
LIBS += \
    -L$$OUT_PWD/../Libs/7z -l7z \
    -L$$OUT_PWD/../Libs/dxtc -ldxtc \
    -L$$OUT_PWD/../Libs/lzo2 -llzo2 \
    -L$$OUT_PWD/../Libs/zlib -lzlib

equals(ZSTD_ENABLE, true) {
    LIBS += -L$$OUT_PWD/../Libs/zstd -lzstd
}
}

}
