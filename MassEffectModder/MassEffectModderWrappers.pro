TEMPLATE = subdirs

CONFIG += ordered

WRAPPERS_SHARED = true
cache(WRAPPERS_SHARED, set)

SUBDIRS += \
    Libs/7z \
    Libs/unlzx \
    Libs/zlib

SUBDIRS += \
    Libs/unrar \
    Wrappers
