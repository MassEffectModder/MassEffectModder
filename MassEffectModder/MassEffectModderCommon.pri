TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    Libs/7z \
    Libs/bfd \
    Libs/dxtc \
    Libs/lzo2 \
    Libs/unlzx

!win32 {
SUBDIRS += Libs/omp
}

SUBDIRS += \
    Libs/png \
    Libs/zlib \
    Libs/zstd \
    Libs/unrar \
    Wrappers \
    MassEffectModder

