QT += core
QT -= gui

CONFIG += c++14 static precompile_header console
CONFIG -= app_bundle

TARGET = MassEffectModderNoGui

TEMPLATE = app

SOURCES += \
    Exceptions/SignalHandler.cpp \
    Helpers/Logs.cpp \
    Helpers/Misc.cpp \
    Helpers/FileStream.cpp \
    Helpers/MemoryStream.cpp \
    Helpers/Stream.cpp \
    Helpers/ParallelCRC.cpp \
    Main.cpp \
    CmdLineParams.cpp \
    CmdLineTools.cpp \
    GameData.cpp \
    Package.cpp \
    Texture.cpp \
    TextureProps.cpp \
    TreeScan.cpp \
    ConfigIni.cpp

PRECOMPILED_HEADER = Precompiled.h

HEADERS += \
    Exceptions/Backtrace.h \
    Exceptions/SignalHandler.h \
    Helpers/Logs.h \
    Helpers/Misc.h \
    Helpers/FileStream.h \
    Helpers/MemoryStream.h \
    Helpers/Stream.h \
    Helpers/ParallelCRC.h \
    CmdLineParams.h \
    CmdLineTools.h \
    GameData.h \
    Package.h \
    ConfigIni.h \
    MemTypes.h \
    Texture.h \
    TextureProps.h \
    TreeScan.h \
    Version.h

DEFINES += QT_DEPRECATED_WARNINGS

precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}

QMAKE_CXXFLAGS += -g

win32-g++: {
    Release:PRE_TARGETDEPS += $$OUT_PWD/../Wrappers/release/libWrappers.a
    Debug:PRE_TARGETDEPS += $$OUT_PWD/../Wrappers/debug/libWrappers.a
} else:unix: {
    PRE_TARGETDEPS += $$OUT_PWD/../Wrappers/libWrappers.a
}

INCLUDEPATH += $$PWD/../Wrappers

DEPENDPATH += $$PWD/../Wrappers

win32-g++: {
Release:LIBS += \
    -L$$OUT_PWD/../Wrappers/release -lWrappers \
    -L$$OUT_PWD/../Libs/dxtc/release -ldxtc \
    -L$$OUT_PWD/../Libs/lzma/release -llzma \
    -L$$OUT_PWD/../Libs/lzo2/release -llzo2 \
    -L$$OUT_PWD/../Libs/xdelta3/release -lxdelta3 \
    -L$$OUT_PWD/../Libs/zlib/release -lzlib
Debug:LIBS += \
    -L$$OUT_PWD/../Wrappers/debug -lWrappers \
    -L$$OUT_PWD/../Libs/dxtc/debug -ldxtc \
    -L$$OUT_PWD/../Libs/lzma/debug -llzma \
    -L$$OUT_PWD/../Libs/lzo2/debug -llzo2 \
    -L$$OUT_PWD/../Libs/xdelta3/debug -lxdelta3 \
    -L$$OUT_PWD/../Libs/zlib/debug -lzlib
} else:unix: {
LIBS += \
    -L$$OUT_PWD/../Wrappers -lWrappers \
    -L$$OUT_PWD/../Libs/dxtc -ldxtc \
    -L$$OUT_PWD/../Libs/lzma -llzma \
    -L$$OUT_PWD/../Libs/lzo2 -llzo2 \
    -L$$OUT_PWD/../Libs/xdelta3 -lxdelta3 \
    -L$$OUT_PWD/../Libs/zlib -lzlib
}

macx {
    # macOS doesn't have OpenMP installed
    # build from sources and install:
    OMP_PATH=/usr/local/libomp
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp -I$$OMP_PATH/include
    LIBS += -L$$OMP_PATH/lib -lomp

    # WA: PCH file clash with targer file name
    PRECOMPILED_DIR = ".pch"

    SOURCES += Exceptions/BacktraceMac.cpp
}

win32 {
    SOURCES += Exceptions/BacktraceWin.cpp
    LIBS += -lbfd -liberty -limagehlp -lintl -liconv -lz -lgomp

    QMAKE_CXXFLAGS += -fopenmp

    # WA: this bad. Assuming Qtcreator/project is on the same disk as msys2.
    # And assuming msys64 is main directory of msys2 64bit installation.
    # It should be /mingw64/lib/binutils but doesn't work in Qt env.
    QMAKE_LIBDIR += c:/msys64/mingw64/lib/binutils
}

linux {
    # libomp need to be compiled from sources
    OMP_PATH=/usr/local/libomp
    QMAKE_CXXFLAGS += -fopenmp -I$$OMP_PATH/include
    LIBS += -L$$OMP_PATH/lib -lomp

    # WA: PCH file clash with targer file name
    PRECOMPILED_DIR = ".pch"

    # backtrace require compile with 'dynamic' flag
    QMAKE_LFLAGS += -rdynamic

    SOURCES += Exceptions/BacktraceLin.cpp
}
