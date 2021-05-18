TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    Libs/7z \
    Libs/bfd \
    Libs/bc7 \
    Libs/dxtc \
    Libs/lzo2 \
    Libs/unlzx

!win32 {
SUBDIRS += Libs/omp
}

SUBDIRS += \
    Libs/oodle \
    Libs/png \
    Libs/zlib

equals(ZSTD_ENABLE, true) {
SUBDIRS += \
    Libs/zstd
}

SUBDIRS += \
    Libs/unrar \
    Wrappers \
    MassEffectModder
