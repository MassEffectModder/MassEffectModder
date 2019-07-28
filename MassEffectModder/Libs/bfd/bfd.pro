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
    coffgen.c \
    dwarf2.c \
    filename_cmp.c \
    fnmatch.c \
    format.c \
    hash.c \
    hashtab.c \
    libbfd.c \
    linker.c \
    pei-x86_64.c \
    pex64igen.c \
    objalloc.c \
    opncls.c \
    reloc.c \
    section.c \
    syms.c \
    targets.c \
    unlink-if-ordinary.c \
    xexit.c \
    xmalloc.c \
    xstrdup.c \
    xstrerror.c

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
    objalloc.h \
    peicode.h \
    symcat.h \
    sysdep.h

DEFINES += HAVE_CONFIG_H
