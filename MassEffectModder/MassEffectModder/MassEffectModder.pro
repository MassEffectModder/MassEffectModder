QT += core

equals(GUI_MODE, true) {
    QT += gui widgets network
} else {
    QT -= gui
    CONFIG += console
}

CONFIG += c++17 static precompile_header
equals(GUI_MODE, true) {
    CONFIG += app_bundle
} else {
    CONFIG -= app_bundle
}
CONFIG += sdk_no_version_check

equals(GUI_MODE, true) {
    TARGET = MassEffectModder
} else {
    TARGET = MassEffectModderNoGui
}

TEMPLATE = app

SOURCES += \
    GameData/GameData.cpp \
    GameData/Package.cpp \
    GameData/Properties.cpp \
    GameData/TOCFile.cpp \
    GameData/UserSettings.cpp \
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
    Texture/TextureCube.cpp \
    Texture/TextureMovie.cpp \
    Texture/TextureScan.cpp

equals(GUI_MODE, true) {
SOURCES += \
    Gui/MainWindow.cpp \
    Gui/MessageWindow.cpp \
    Gui/GuiGameUtilities.cpp \
    Gui/GuiModsManager.cpp \
    Gui/GuiTextureUtilities.cpp \
    Gui/InstallerWindow.cpp \
    Gui/LayoutInstallerMain.cpp \
    Gui/LayoutInstallMods.cpp \
    Gui/LayoutMain.cpp \
    Gui/LayoutTexturesManager.cpp \
    Gui/PixmapLabel.cpp \
    Gui/Updater.cpp
} else {
SOURCES += \
    CmdLine/CmdLineHelp.cpp \
    CmdLine/CmdLineParams.cpp \
    CmdLine/CmdLineTools.cpp
}

PRECOMPILED_HEADER = Types/Precompiled.h

HEADERS += \
    GameData/GameData.h \
    GameData/Package.h \
    GameData/Properties.h \
    GameData/TOCFile.h \
    GameData/UserSettings.h \
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
    Misc/CommonStrings.h \
    Misc/Misc.h \
    MipMaps/MipMap.h \
    MipMaps/MipMaps.h \
    Program/ConfigIni.h \
    Program/SignalHandler.h \
    Resources/Resources.h \
    Texture/Texture.h \
    Texture/TextureCube.h \
    Texture/TextureMovie.h \
    Texture/TextureScan.h \
    Types/MemTypes.h
equals(GUI_MODE, true) {
HEADERS += \
    Gui/MainWindow.h \
    Gui/MessageWindow.h \
    Gui/InstallerWindow.h \
    Gui/LayoutInstallMods.h \
    Gui/LayoutInstallerMain.h \
    Gui/LayoutHandle.h \
    Gui/LayoutMain.h \
    Gui/LayoutTexturesManager.h \
    Gui/PixmapLabel.h \
    Gui/Updater.h
} else {
HEADERS += \
    CmdLine/CmdLineParams.h \
    CmdLine/CmdLineTools.h
}

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
            equals(GUI_MODE, true) {
                QMAKE_POST_LINK += dsymutil "$$TARGET".app/Contents/MacOS/$$TARGET -o "$$TARGET".app/Contents/MacOS/"$$TARGET".dSYM
            } else {
                QMAKE_POST_LINK += dsymutil $$TARGET -o "$$TARGET".dSYM
            }
        }
    }
    QMAKE_CXXFLAGS_RELEASE += -g1 -O3
}

QMAKE_CXXFLAGS +=
QMAKE_CXXFLAGS_DEBUG += -g

win32-g++: {
    # Disable compiler warning
    QMAKE_CXXFLAGS += -Wno-deprecated-copy
    QMAKE_LFLAGS_RELEASE = "-Wl,--relax"

    COMPILER_VERSION = $$system($$QMAKE_CXX " -dumpversion")
    VERSIONS = $$split(COMPILER_VERSION, .)
    COMPILER_MAJOR_VERSION = $$member(VERSIONS, 0)
    greaterThan(COMPILER_MAJOR_VERSION, 8) {
        # Disabled dynamic base, needed to get symbols matched with base
        QMAKE_LFLAGS_RELEASE += "-Wl,--disable-dynamicbase"
        QMAKE_LFLAGS_DEBUG += "-Wl,--disable-dynamicbase"
        # Enforce DWARF-4 for backtraces (bfd code needs to be updated)
        QMAKE_CXXFLAGS += -gdwarf-4
    }

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
    -L$$OUT_PWD/../Libs/bc7/release -lbc7 \
    -L$$OUT_PWD/../Libs/dxtc/release -ldxtc \
    -L$$OUT_PWD/../Libs/unlzx/release -lunlzx \
    -L$$OUT_PWD/../Libs/png/release -lpng \
    -L$$OUT_PWD/../Libs/oodle/release -loodle \
    -L$$OUT_PWD/../Libs/zlib/release -lzlib \
    -L$$OUT_PWD/../Libs/unrar/release -lunrar
equals(ZSTD_ENABLE, true) {
    Release:LIBS += -L$$OUT_PWD/../Libs/zstd/debug -lzstd
}
Debug:LIBS += \
    -L$$OUT_PWD/../Wrappers/debug -lWrappers \
    -L$$OUT_PWD/../Libs/7z/debug -l7z \
    -L$$OUT_PWD/../Libs/bfd/debug -lbfd \
    -L$$OUT_PWD/../Libs/bc7/debug -lbc7 \
    -L$$OUT_PWD/../Libs/dxtc/debug -ldxtc \
    -L$$OUT_PWD/../Libs/unlzx/debug -lunlzx \
    -L$$OUT_PWD/../Libs/png/debug -lpng \
    -L$$OUT_PWD/../Libs/oodle/debug -loodle \
    -L$$OUT_PWD/../Libs/zlib/debug -lzlib \
    -L$$OUT_PWD/../Libs/unrar/debug -lunrar
equals(ZSTD_ENABLE, true) {
    Debug:LIBS += -L$$OUT_PWD/../Libs/zstd/debug -lzstd
}
} else:unix: {
LIBS += \
    -L$$OUT_PWD/../Wrappers -lWrappers \
    -L$$OUT_PWD/../Libs/7z -l7z \
    -L$$OUT_PWD/../Libs/bfd -lbfd \
    -L$$OUT_PWD/../Libs/bc7 -lbc7 \
    -L$$OUT_PWD/../Libs/dxtc -ldxtc \
    -L$$OUT_PWD/../Libs/unlzx -lunlzx \
    -L$$OUT_PWD/../Libs/omp -lomp \
    -L$$OUT_PWD/../Libs/png -lpng \
    -L$$OUT_PWD/../Libs/oodle -loodle \
    -L$$OUT_PWD/../Libs/zlib -lzlib \
    -L$$OUT_PWD/../Libs/unrar -lunrar
equals(ZSTD_ENABLE, true) {
    LIBS += -L$$OUT_PWD/../Libs/zstd -lzstd
}
}

macx {
    QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden -fvisibility-inlines-hidden -Xpreprocessor -fopenmp
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.00
}

win32 {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -limagehlp -lgomp -lversion
}

linux {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -ldl
    equals(QMAKE_CXX, clang++) {
        # backtrace require compile with 'dynamic' flag
        QMAKE_LFLAGS += -rdynamic
    }
}

# ASan
#QMAKE_CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
#QMAKE_LFLAGS += -fsanitize=address -fno-omit-frame-pointer
