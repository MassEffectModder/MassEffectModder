TEMPLATE = subdirs

CONFIG += ordered

WRAPPERS_SHARED = true
cache(WRAPPERS_SHARED, set)

SUBDIRS += \
    Libs/7z \
    Libs/dxtc \
    Libs/lzo2 \
    Libs/png \
    Libs/unlzx \
    Libs/zlib

!equals(WRAPPERS_SHARED, true) {
SUBDIRS += \
    Libs/zstd
}

SUBDIRS += \
    Libs/unrar \
    Wrappers
