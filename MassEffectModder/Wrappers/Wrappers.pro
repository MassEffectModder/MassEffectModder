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
    WrapperUnLzx.cpp \
    WrapperUnzip.cpp \
    WrapperZlib.cpp

!equals(WRAPPERS_SHARED, true) {
SOURCES += \
    WrapperBc7.cpp \
    WrapperDxtc.cpp \
    WrapperOodle.cpp \
    WrapperPng.cpp \
    WrapperUnrar.cpp
}

!equals(WRAPPERS_SHARED, true) {
    macos {
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

equals(QMAKE_CXX, g++) {
QMAKE_CXXFLAGS += -Wno-format-truncation -Wno-stringop-truncation
}
QMAKE_CXXFLAGS +=
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -g1 -O3
QMAKE_CXXFLAGS_DEBUG += -g

DEFINES +=

INCLUDEPATH += \
    $$PWD/../Libs/7z \
    $$PWD/../Libs/bfd \
    $$PWD/../Libs/bc7 \
    $$PWD/../Libs/dxtc \
    $$PWD/../Libs/png \
    $$PWD/../Libs/oodle \
    $$PWD/../Libs/zlib \
    $$PWD/../Libs/unrar

DEPENDPATH += \
    $$PWD/../Libs/7z \
    $$PWD/../Libs/bfd \
    $$PWD/../Libs/bc7 \
    $$PWD/../Libs/dxtc \
    $$PWD/../Libs/png \
    $$PWD/../Libs/oodle \
    $$PWD/../Libs/zlib \
    $$PWD/../Libs/unrar

macos {
    CONFIG(debug, debug|release) {
        contains(QT_ARCH, x86_64) {
            QMAKE_APPLE_DEVICE_ARCHS=x86_64h
        }
        contains(QT_ARCH, arm64) {
            QMAKE_APPLE_DEVICE_ARCHS=arm64
        }
    }
}

equals(WRAPPERS_SHARED, true) {

QMAKE_CXXFLAGS += -DEXPORT_LIBS

win32: {
Release:LIBS += \
    -L$$OUT_PWD/../Libs/7z/release -l7z \
    -L$$OUT_PWD/../Libs/unlzx/release -lunlzx \
    -L$$OUT_PWD/../Libs/zlib/release -lzlib

!equals(WRAPPERS_SHARED, true) {
    Release:LIBS += -L$$OUT_PWD/../Libs/bc7/release -lbc7 \
    Release:LIBS += -L$$OUT_PWD/../Libs/dxtc/release -ldxtc
}

Debug:LIBS += \
    -L$$OUT_PWD/../Libs/7z/debug -l7z \
    -L$$OUT_PWD/../Libs/unlzx/debug -lunlzx \
    -L$$OUT_PWD/../Libs/zlib/debug -lzlib

!equals(WRAPPERS_SHARED, true) {
    Debug:LIBS += -L$$OUT_PWD/../Libs/bc7/debug -lbc7 \
    Debug:LIBS += -L$$OUT_PWD/../Libs/dxtc/debug -ldxtc \
}

} else:unix: {
LIBS += \
    -L$$OUT_PWD/../Libs/7z -l7z \
    -L$$OUT_PWD/../Libs/unlzx -lunlzx \
    -L$$OUT_PWD/../Libs/zlib -lzlib

!equals(WRAPPERS_SHARED, true) {
    LIBS += -L$$OUT_PWD/../Libs/bc7 -lbc7 \
    LIBS += -L$$OUT_PWD/../Libs/dxtc -ldxtc \
    LIBS += -L$$OUT_PWD/../Libs/oodle -loodle
}

}

}
