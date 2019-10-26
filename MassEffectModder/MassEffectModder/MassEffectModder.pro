QT += core

equals(GUI_MODE, true) {
    QT += gui widgets
} else {
    QT -= gui
    CONFIG += console
}

CONFIG += c++17 static precompile_header
CONFIG -= app_bundle
CONFIG += sdk_no_version_check

equals(GUI_MODE, true) {
    TARGET = MassEffectModder
} else {
    TARGET = MassEffectModderNoGui
}

TEMPLATE = app

SOURCES += \
    CmdLine/CmdLineHelp.cpp \
    CmdLine/CmdLineParams.cpp \
    CmdLine/CmdLineTools.cpp \
    GameData/DLC.cpp \
    GameData/GameData.cpp \
    GameData/LODSettings.cpp \
    GameData/Package.cpp \
    GameData/TOCFile.cpp \
    Helpers/Crc32.cpp \
    Helpers/FileStream.cpp \
    Helpers/Logs.cpp \
    Helpers/MemoryStream.cpp \
    Helpers/MiscHelpers.cpp \
    Helpers/Stream.cpp \
    Image/Image.cpp \
    Image/ImageBMP.cpp \
    Image/ImageDDS.cpp \
    Image/ImageTGA.cpp \
    Md5/MD5BadEntries.cpp \
    Md5/MD5ModEntries.cpp \
    MipMaps/MipMap.cpp \
    MipMaps/MipMaps.cpp \
    MipMaps/MipMapsEmptyMips.cpp \
    MipMaps/MipMapsReplace.cpp \
    Misc/Misc.cpp \
    Misc/MiscCheckGame.cpp \
    Misc/MiscMods.cpp \
    Misc/MiscModsInstall.cpp \
    Misc/MiscProcessGame.cpp \
    Misc/MiscTexture.cpp \
    Program/ConfigIni.cpp \
    Program/Main.cpp \
    Program/SignalHandler.cpp \
    Resources/Resources.cpp \
    Texture/Texture.cpp \
    Texture/TextureProps.cpp \
    Texture/TextureScan.cpp

equals(GUI_MODE, true) {
SOURCES += \
    Gui/MainWindow.cpp \
    Gui/MessageWindow.cpp \
    Gui/LayoutGameUtilities.cpp \
    Gui/LayoutInstallModsManager.cpp \
    Gui/LayoutMeSelect.cpp \
    Gui/LayoutModsManager.cpp \
    Gui/LayoutModules.cpp \
    Gui/LayoutTexturesManager.cpp \
    Gui/LayoutTextureUtilities.cpp
HEADERS += \
    Gui/MainWindow.h \
    Gui/MessageWindow.h \
    Gui/LayoutGameUtilities.h \
    Gui/LayoutInstallModsManager.h \
    Gui/LayoutMeSelect.h \
    Gui/LayoutModsManager.h \
    Gui/LayoutModules.h \
    Gui/LayoutTexturesManager.h \
    Gui/LayoutTextureUtilities.h
}

PRECOMPILED_HEADER = Types/Precompiled.h

HEADERS += \
    CmdLine/CmdLineParams.h \
    CmdLine/CmdLineTools.h \
    GameData/DLC.h \
    GameData/GameData.h \
    GameData/LODSettings.h \
    GameData/Package.h \
    GameData/TOCFile.h \
    Helpers/ByteBuffer.h \
    Helpers/BinarySearch.h \
    Helpers/Crc32.h \
    Helpers/Exception.h \
    Helpers/FileStream.h \
    Helpers/Logs.h \
    Helpers/MemoryStream.h \
    Helpers/MiscHelpers.h \
    Helpers/QSort.h \
    Helpers/Stream.h \
    Image/Image.h \
    Md5/MD5BadEntries.h \
    Md5/MD5ModEntries.h \
    Misc/Misc.h \
    MipMaps/MipMap.h \
    MipMaps/MipMaps.h \
    Program/ConfigIni.h \
    Program/SignalHandler.h \
    Resources/Resources.h \
    Texture/Texture.h \
    Texture/TextureProps.h \
    Texture/TextureScan.h \
    Types/MemTypes.h

include(Program/Version.pri)

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += MEM_VERSION=\"$$VERSION\" MEM_YEAR=\"$$MEM_YEAR\"

RESOURCES = Resources/Resources.qrc
QMAKE_RESOURCE_FLAGS += --no-compress

precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}
PRECOMPILED_DIR = ".pch"

equals(GUI_MODE, true) {
    DEFINES += GUI
}

QMAKE_CXXFLAGS_RELEASE -= -O2
equals(RELEASE_IN_DEBUG_MODE, true) {
    QMAKE_CXXFLAGS_RELEASE += -g
} else {
    CONFIG(release, debug | release) {
        DEFINES += NDEBUG
        macx {
            QMAKE_POST_LINK += dsymutil $$TARGET -o "$$TARGET".dSYM
        }
    }
    QMAKE_CXXFLAGS_RELEASE += -g1
    win32-g++: {
        QMAKE_CXXFLAGS_RELEASE += -O2
    } else {
        QMAKE_CXXFLAGS_RELEASE += -O3
    }
}

QMAKE_CXXFLAGS +=
QMAKE_CXXFLAGS_DEBUG += -g

win32-g++: {
    # Disable compiler warning
    QMAKE_CXXFLAGS += -Wno-deprecated-copy
    QMAKE_LFLAGS_RELEASE = "-Wl,--relax"
    Release:PRE_TARGETDEPS += $$OUT_PWD/../Wrappers/release/libWrappers.a
    Debug:PRE_TARGETDEPS += $$OUT_PWD/../Wrappers/debug/libWrappers.a
} else:unix: {
    PRE_TARGETDEPS += $$OUT_PWD/../Wrappers/libWrappers.a
}

INCLUDEPATH += $$PWD/../Wrappers
!win32 {
    INCLUDEPATH += $$PWD/../Libs/omp
}

DEPENDPATH += $$PWD/../Wrappers

win32-g++: {
Release:LIBS += \
    -L$$OUT_PWD/../Wrappers/release -lWrappers \
    -L$$OUT_PWD/../Libs/7z/release -l7z \
    -L$$OUT_PWD/../Libs/bfd/release -lbfd \
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
    -L$$OUT_PWD/../Libs/bfd/debug -lbfd \
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
    -L$$OUT_PWD/../Libs/bfd -lbfd \
    -L$$OUT_PWD/../Libs/dxtc -ldxtc \
    -L$$OUT_PWD/../Libs/lzo2 -llzo2 \
    -L$$OUT_PWD/../Libs/omp -lomp \
    -L$$OUT_PWD/../Libs/png -lpng \
    -L$$OUT_PWD/../Libs/xdelta3 -lxdelta3 \
    -L$$OUT_PWD/../Libs/zlib -lzlib \
#    -L$$OUT_PWD/../Libs/zstd -lzstd \
    -L$$OUT_PWD/../Libs/unrar -lunrar
}

macx {
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp
}

win32 {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -limagehlp -lgomp
}

linux {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -ldl
    equals(QMAKE_CXX, clang++) {
        # backtrace require compile with 'dynamic' flag
        QMAKE_LFLAGS += -rdynamic
    }
}
