TEMPLATE = lib
CONFIG += staticlib warn_off

QT -= gui core

SOURCES += \
     oodle.cpp

!win32 {
SOURCES += \
    log.c \
    pe_linker.c \
    search_hsearch_r.c \
    winapi.c
}

HEADERS += \
    oodle.h

!win32 {
HEADERS += \
    log.h \
    pe_linker.h \
    search_hsearch_r.h \
    winnt_types.h
}

!win32 {
QMAKE_CFLAGS += -fshort-wchar -Wno-multichar -DNDEBUG -O3
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -g1
QMAKE_CFLAGS_DEBUG += -g
}

QMAKE_CXXFLAGS += -DNDEBUG -O3
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -g1
QMAKE_CXXFLAGS_DEBUG += -g

DEFINES +=
