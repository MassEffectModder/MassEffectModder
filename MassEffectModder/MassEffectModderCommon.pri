TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    Libs/7z \
    Libs/bfd \
    Libs/dxtc \
    Libs/lzo2

!win32 {
SUBDIRS += Libs/omp
}

SUBDIRS += \
    Libs/png \
    Libs/xdelta3 \
    Libs/zlib

equals(ZSTD_ENABLE, true) {
SUBDIRS += \
    Libs/zstd
}

SUBDIRS += \
    Libs/unrar \
    Wrappers \
    MassEffectModder

