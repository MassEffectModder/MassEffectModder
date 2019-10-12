QT += core
QT -= gui

CONFIG += c++17 static precompile_header console
CONFIG -= app_bundle
CONFIG += sdk_no_version_check

TARGET = Md5SumGen

TEMPLATE = app

SOURCES += \
    ../MassEffectModder/Helpers/FileStream.cpp \
    ../MassEffectModder/Helpers/Logs.cpp \
    ../MassEffectModder/Helpers/MemoryStream.cpp \
    ../MassEffectModder/Helpers/MiscHelpers.cpp \
    ../MassEffectModder/Helpers/SignalHandler.cpp \
    ../MassEffectModder/Helpers/Stream.cpp \
    ../Wrappers/WrapperZlib.cpp \
    Main.cpp \
    MD5EntriesME1.cpp \
    MD5EntriesME1PL.cpp \
    MD5EntriesME2.cpp \
    MD5EntriesME3.cpp

PRECOMPILED_HEADER = ../MassEffectModder/Precompiled.h

HEADERS += \
    ../MassEffectModder/Helpers/ByteBuffer.h \
    ../MassEffectModder/Helpers/Exception.h \
    ../MassEffectModder/Helpers/FileStream.h \
    ../MassEffectModder/Helpers/Logs.h \
    ../MassEffectModder/Helpers/MemoryStream.h \
    ../MassEffectModder/Helpers/MiscHelpers.h \
    ../MassEffectModder/Helpers/Stream.h \
    MD5Entries.h

DEFINES += QT_DEPRECATED_WARNINGS

precompile_header:!isEmpty(PRECOMPILED_HEADER) {
    DEFINES += USING_PCH
}
PRECOMPILED_DIR = ".pch"

QMAKE_CXXFLAGS +=
QMAKE_CXXFLAGS_DEBUG += -g

INCLUDEPATH += $$PWD/../Wrappers $$PWD/../MassEffectModder
!win32 {
    INCLUDEPATH += $$PWD/../Libs/omp
}

win32-g++: {
Release:LIBS += \
    -L$$OUT_PWD/../Libs/zlib/release -lzlib
Debug:LIBS += \
    -L$$OUT_PWD/../Libs/zlib/debug -lzlib
} else:unix: {
LIBS += \
    -L$$OUT_PWD/../Libs/zlib -lzlib
}
