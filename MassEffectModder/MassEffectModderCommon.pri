TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    Libs/7z \
    Libs/bfd \
    Libs/bc7 \
    Libs/dxtc \
    Libs/unlzx

!win32 {
SUBDIRS += Libs/omp
}

SUBDIRS += \
    Libs/oodle \
    Libs/png \
    Libs/zlib \
    Libs/unrar \
    Wrappers \
    MassEffectModder
