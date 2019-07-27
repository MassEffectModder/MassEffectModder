TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    Libs/7z \
    Libs/dxtc \
    Libs/lzo2

!win32 {
SUBDIRS += Libs/omp
}

SUBDIRS += \
    Libs/png \
    Libs/xdelta3 \
    Libs/zlib \
#    Libs/zstd \
    Libs/unrar \
    Wrappers \
    MassEffectModder
