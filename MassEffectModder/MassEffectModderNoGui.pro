TEMPLATE = subdirs

CONFIG += ordered

GUI_MODE = false
cache(GUI_MODE, set)

SUBDIRS += \
    Libs/7z \
    Libs/dxtc \
    Libs/lzo2 \
    Libs/png \
    Libs/xdelta3 \
    Libs/zlib \
    Libs/unrar \
    Wrappers \
    MassEffectModder
