QT += core

equals(GUI_MODE, true) {
    QT += gui widgets
} else {
    QT -= gui
    CONFIG += console
}

CONFIG += c++14 static precompile_header
CONFIG -= app_bundle

equals(GUI_MODE, true) {
    TARGET = MassEffectModder
} else {
    TARGET = MassEffectModderNoGui
}

TEMPLATE = app

SOURCES += \
    Exceptions/SignalHandler.cpp \
    Helpers/Crc32.cpp \
    Helpers/FileStream.cpp \
    Helpers/Logs.cpp \
    Helpers/MemoryStream.cpp \
    Helpers/MiscHelpers.cpp \
    Helpers/Stream.cpp \
    CmdLineHelp.cpp \
    CmdLineParams.cpp \
    CmdLineTools.cpp \
    ConfigIni.cpp \
    DLC.cpp \
    Image.cpp \
    ImageBMP.cpp \
    ImageDDS.cpp \
    ImageTGA.cpp \
    GameData.cpp \
    LODSettings.cpp \
    MD5BadEntries.cpp \
    MD5ModEntries.cpp \
    Main.cpp \
    MipMap.cpp \
    Misc.cpp \
    Package.cpp \
    Resources.cpp \
    Texture.cpp \
    TextureEmptyMips.cpp \
    TextureProcess.cpp \
    TextureProps.cpp \
    TextureReplace.cpp \
    TOCFile.cpp \
    TreeScan.cpp

equals(GUI_MODE, true) {
SOURCES += \
    Gui/MainWindow.cpp \
    Gui/LayoutMeSelect.cpp
HEADERS += \
    Gui/MainWindow.h \
    Gui/LayoutMeSelect.h
}

PRECOMPILED_HEADER = Precompiled.h

HEADERS += \
    Exceptions/Backtrace.h \
    Exceptions/Exception.h \
    Exceptions/SignalHandler.h \
    Helpers/ByteBuffer.h \
    Helpers/BinarySearch.h \
    Helpers/Crc32.h \
    Helpers/FileStream.h \
    Helpers/Logs.h \
    Helpers/MemoryStream.h \
    Helpers/MiscHelpers.h \
    Helpers/QSort.h \
    Helpers/Stream.h \
    CmdLineParams.h \
    CmdLineTools.h \
    ConfigIni.h \
    DLC.h \
    Image.h \
    GameData.h \
    LODSettings.h \
    MD5BadEntries.h \
    MD5ModEntries.h \
    Misc.h \
    MipMap.h \
    MipMaps.h \
    MemTypes.h \
    Package.h \
    Resources.h \
    Texture.h \
    TextureProps.h \
    TOCFile.h \
    TreeScan.h

include(Version.pri)

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += MEM_VERSION=\\\"$$VERSION\\\"

RESOURCES = Resources.qrc
QMAKE_RESOURCE_FLAGS += --no-compress

precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}

equals(GUI_MODE, true) {
    DEFINES += GUI
}

CONFIG(release, debug | release) {
    DEFINES += NDEBUG
}

QMAKE_CXXFLAGS +=
QMAKE_CXXFLAGS_RELEASE += -g1
QMAKE_CXXFLAGS_DEBUG += -g

win32-g++: {
    QMAKE_LFLAGS_RELEASE="-Wl,--relax,--gc-sections"
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
    -L$$OUT_PWD/../Libs/7z/release -l7z \
    -L$$OUT_PWD/../Libs/dxtc/release -ldxtc \
    -L$$OUT_PWD/../Libs/lzo2/release -llzo2 \
    -L$$OUT_PWD/../Libs/png/release -lpng \
    -L$$OUT_PWD/../Libs/xdelta3/release -lxdelta3 \
    -L$$OUT_PWD/../Libs/zlib/release -lzlib \
#    -L$$OUT_PWD/../Libs/zstd/release -lzstd \
    -L$$OUT_PWD/../Libs/unrar/release -lunrar
Debug:LIBS += \
    -L$$OUT_PWD/../Wrappers/debug -lWrappers \
    -L$$OUT_PWD/../Libs/7z/debug -l7z \
    -L$$OUT_PWD/../Libs/dxtc/debug -ldxtc \
    -L$$OUT_PWD/../Libs/lzo2/debug -llzo2 \
    -L$$OUT_PWD/../Libs/png/debug -lpng \
    -L$$OUT_PWD/../Libs/xdelta3/debug -lxdelta3 \
    -L$$OUT_PWD/../Libs/zlib/debug -lzlib \
#    -L$$OUT_PWD/../Libs/zstd/debug -lzstd \
    -L$$OUT_PWD/../Libs/unrar/debug -lunrar
} else:unix: {
LIBS += \
    -L$$OUT_PWD/../Wrappers -lWrappers \
    -L$$OUT_PWD/../Libs/7z -l7z \
    -L$$OUT_PWD/../Libs/dxtc -ldxtc \
    -L$$OUT_PWD/../Libs/lzo2 -llzo2 \
    -L$$OUT_PWD/../Libs/png -lpng \
    -L$$OUT_PWD/../Libs/xdelta3 -lxdelta3 \
    -L$$OUT_PWD/../Libs/zlib -lzlib \
#    -L$$OUT_PWD/../Libs/zstd -lzstd \
    -L$$OUT_PWD/../Libs/unrar -lunrar
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
    QMAKE_CXXFLAGS_RELEASE += -I/usr/local/libomp-static/include
    QMAKE_CXXFLAGS_DEBUG += -I/usr/local/libomp/include
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS_RELEASE += -L/usr/local/libomp-static/lib
    QMAKE_LFLAGS_DEBUG += -L/usr/local/libomp/lib
    LIBS += -lomp -ldl

    # WA: PCH file clash with targer file name
    PRECOMPILED_DIR = ".pch"

    # backtrace require compile with 'dynamic' flag
    QMAKE_LFLAGS += -rdynamic

    SOURCES += Exceptions/BacktraceLin.cpp
}
