TEMPLATE = subdirs

CONFIG += ordered

WRAPPERS_SHARED = true
cache(WRAPPERS_SHARED, set)

SUBDIRS += \
    Libs/7z \
    Libs/bc7 \
    Libs/dxtc \
    Libs/oodle \
    Libs/png \
    Libs/unlzx \
    Libs/unrar \
    Libs/zlib \
    Wrappers
