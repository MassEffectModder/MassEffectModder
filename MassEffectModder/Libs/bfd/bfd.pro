TEMPLATE = lib
CONFIG += staticlib

QT -= gui core

SOURCES += \
    archive.c \
    archures.c \
    bfd.c \
    bfdio.c \
    cache.c \
    compress.c \
    cpu-i386.c \
    dwarf2.c \
    filename_cmp.c \
    fnmatch.c \
    format.c \
    hash.c \
    hashtab.c \
    lbasename.c \
    libbfd.c \
    linker.c \
    objalloc.c \
    opncls.c \
    reloc.c \
    section.c \
    stab-syms.c \
    syms.c \
    targets.c \
    unlink-if-ordinary.c \
    xexit.c \
    xmalloc.c \
    xstrdup.c \
    xstrerror.c

win32 {
SOURCES += \
    coffgen.c \
    pei-x86_64.c \
    pex64igen.c
}

linux {
SOURCES += \
    elf.c \
    elf64.c \
    elf64-x86-64.c \
    elfxx-x86.c
}

macos {
SOURCES += \
    mach-o-x86-64.c \
    mach-o.c
}

HEADERS += \
    ansidecl.h \
    bfd.h \
    bfd_stdint.h \
    bfdlink.h \
    bfdver.h \
    coff-bfd.h \
    config.h \
    dwarf2.h \
    elf-bfd.h \
    environ.h \
    filenames.h \
    fnmatch.h \
    fopen-bin.h \
    genlink.h \
    hashtab.h \
    libbfd.h \
    libcoff.h \
    libiberty.h \
    libpei.h \
    mach-o.h \
    objalloc.h \
    peicode.h \
    symcat.h \
    sysdep.h

DEFINES += HAVE_CONFIG_H

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

win32-g++: {
    QMAKE_CFLAGS += -Wno-format-extra-args -Wno-format
}
