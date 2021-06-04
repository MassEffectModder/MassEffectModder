TEMPLATE = subdirs

CONFIG += ordered

WRAPPERS_SHARED = true
cache(WRAPPERS_SHARED, set)

ZSTD_ENABLE = false
cache(ZSTD_ENABLE, set)

SUBDIRS += \
    Libs/7z \
    Libs/unlzx \
    Libs/zlib
}

SUBDIRS += \
    Libs/unrar \
    Wrappers
