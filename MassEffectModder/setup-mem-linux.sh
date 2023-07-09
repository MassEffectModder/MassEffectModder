#!/bin/sh

#
# MIT License
#
# Copyright (c) 2019-2023 Pawel Kolodziejski
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

echo
echo "Checking sudo privileges..."
sudo echo "Accepted, continuing..."
if [ $? != 0 ]; then
	echo "The sudo priviliges check failed, aborting..."
	echo
	exit 1
fi
echo

BASE_PATH=/srv/chroot
DISTRO_BASE=debian
DISTRO_NAME=buster
DISTRO_ARCH=amd64
DISTRO_URL=
if [ "$1" != "" ]; then
	DISTRO_CACHE_FILE=`readlink -f $1`
fi
DISTRO_CACHE_CREATED=
PROJECT=mem
BRANCH=master
USER_NAME=aquadran
USER_ID=1000
NUM_THREADS=`grep -c '^processor' /proc/cpuinfo`
QT_VERSION=5.15.10
QT_VERSION_BASE=`echo $QT_VERSION | cut -d'.' -f 1,2`
PACKAGES_ADD=bash,build-essential,nasm,git,perl,python,wget,ca-certificates,libx11-dev,libsdl2-dev,\
libopenal-dev,libfontconfig1-dev,libssl-dev,libxkbcommon-dev,libxkbcommon-x11-dev,libxcomposite-dev,\
libx11-xcb-dev,libglu1-mesa-dev,libxrender-dev,libxi-dev,fuse,file,appstream,zip

BASE_CHROOT="${BASE_PATH}/$PROJECT-$DISTRO_BASE-$DISTRO_ARCH"

if [ ! -f .stamp-debootstrap ]; then
	# Debootstrap
	sudo rm -rf "$BASE_CHROOT"
	DEBOOTSTRAP_ARGS="--arch $DISTRO_ARCH --variant=minbase"
	if [ "$PACKAGES_ADD" != "" ]; then
		DEBOOTSTRAP_ARGS="$DEBOOTSTRAP_ARGS --include=$PACKAGES_ADD"
	fi
	if [ "$DISTRO_CACHE_FILE" != "" ] && [ ! -f "$DISTRO_CACHE_FILE" ]; then
		DEBOOTSTRAP_ARGS="$DEBOOTSTRAP_ARGS --make-tarball=${DISTRO_CACHE_FILE}"
		DISTRO_CACHE_CREATED="yes"
	fi
	if [ "$DISTRO_CACHE_FILE" != "" ] && [   -f "$DISTRO_CACHE_FILE" ]; then
		DEBOOTSTRAP_ARGS="$DEBOOTSTRAP_ARGS --unpack-tarball=${DISTRO_CACHE_FILE}"
	fi
	sudo debootstrap ${DEBOOTSTRAP_ARGS} $DISTRO_NAME $BASE_CHROOT $DISTRO_URL
	if [ $? != 0 ]; then
		echo "The debootstrap step failed, aborting..."
		echo
		exit 1
	fi
	if [ "$DISTRO_CACHE_CREATED" = "yes" ]; then
		echo "The debootstrap cache created, exiting..."
		echo
		exit 0
	fi
	touch .stamp-debootstrap
fi

sudo -v
CHROOT_CMD_ROOT="schroot --user=root --directory=/root --chroot=$PROJECT-$DISTRO_BASE-$DISTRO_ARCH -- bash -c "
CHROOT_CMD_USER="schroot --user=$USER_NAME --directory=/home/$USER_NAME --chroot=$PROJECT-$DISTRO_BASE-$DISTRO_ARCH -- bash -c "

if [ ! -f .stamp-backports ]; then
	$CHROOT_CMD_ROOT "echo \"deb http://deb.debian.org/debian $DISTRO_NAME-backports main\" >> /etc/apt/sources.list;
apt-get --assume-yes update"
	$CHROOT_CMD_ROOT "apt-get -y install libxcb.*-dev"
	touch .stamp-backports
fi

if [ ! -f .stamp-appimage ]; then
	$CHROOT_CMD_ROOT "mknod -m 666 /dev/fuse c 10 229;
wget https://github.com/AppImage/AppImageKit/releases/download/13/appimagetool-x86_64.AppImage;
chmod +x appimagetool-x86_64.AppImage;
mv appimagetool-x86_64.AppImage /usr/local/bin/appimagetool"
	$CHROOT_CMD_ROOT "wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage;
chmod +x linuxdeploy-x86_64.AppImage;
mv linuxdeploy-x86_64.AppImage /usr/local/bin/linuxdeploy"
	touch .stamp-appimage
fi

if [ ! -f .stamp-user ]; then
	# Add user
	$CHROOT_CMD_ROOT "/usr/sbin/addgroup --gid $USER_ID $USER_NAME;
/usr/sbin/adduser --uid $USER_ID --gid $USER_ID --shell /bin/bash --disabled-password --gecos \"\" $USER_NAME"
	touch .stamp-user
fi

sudo -v
if [ ! -f .stamp-sources ]; then
	# Get source code
	$CHROOT_CMD_USER "rm -rf /home/$USER_NAME/builds/sources; mkdir -p /home/$USER_NAME/builds/sources;
cd /home/$USER_NAME/builds/sources;
wget https://download.qt.io/archive/qt/$QT_VERSION_BASE/$QT_VERSION/single/qt-everywhere-opensource-src-${QT_VERSION}.tar.xz;
tar xf qt-everywhere-opensource-src-$QT_VERSION.tar.xz;
rm -f qt-everywhere-opensource-src-$QT_VERSION.tar.xz;
git clone https://github.com/MassEffectModder/MassEffectModder.git;
cd /home/$USER_NAME/builds/sources/MassEffectModder;
git checkout $BRANCH"
	touch .stamp-sources
fi

if [ ! -f .stamp-qt-prepare ]; then
	sudo mount --bind /proc $BASE_CHROOT/proc
	$CHROOT_CMD_USER "cd /home/$USER_NAME/builds/sources/qt-everywhere-src-$QT_VERSION; ./configure \
-prefix /usr -static -release -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg -system-freetype \
-iconv -no-icu -glib -no-cups -no-gif -no-ico -qt-harfbuzz -no-eglfs -no-gbm -no-tiff -no-webp \
-no-mimetype-database -no-feature-relocatable -no-opengl -make libs -nomake examples -nomake tests \
-skip qt3d -skip qtactiveqt -skip qtcanvas3d -skip qtcharts -skip qtconnectivity -skip qtdatavis3d \
-skip qtdeclarative -skip qtdoc -skip gamepad -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia \
-skip qtnetworkauth -skip qtpurchasing -skip qtquickcontrols -skip qtquickcontrols2 -skip qtremoteobjects \
-skip qtscript -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech -skip qttools \
-skip qttranslations -skip qtvirtualkeyboard -skip qtwebengine -skip qtwebglplugin -skip qtwebsockets \
-skip qtwebview -skip qtxmlpatterns -confirm-license -opensource \
-system-freetype -qt-doubleconversion -qpa xcb -xcb -xcb-native-painting"
	sudo umount $BASE_CHROOT/proc
	touch .stamp-qt-prepare
fi

sudo -v
if [ ! -f .stamp-qt-build ]; then
	sudo mount --bind /proc $BASE_CHROOT/proc
	$CHROOT_CMD_USER "cd /home/$USER_NAME/builds/sources/qt-everywhere-src-$QT_VERSION; make -j$NUM_THREADS"
	$CHROOT_CMD_ROOT "cd /home/$USER_NAME/builds/sources/qt-everywhere-src-$QT_VERSION; make install"
	$CHROOT_CMD_USER "rm -rf /home/$USER_NAME/builds/sources/qt-everywhere-src-$QT_VERSION"
	sudo umount $BASE_CHROOT/proc
	touch .stamp-qt-build
fi

sudo -v
if [ -f .stamp-sources ]; then
	# Update source code
	sudo mount --bind /proc $BASE_CHROOT/proc
	$CHROOT_CMD_USER "cd /home/$USER_NAME/builds/sources/MassEffectModder; git clean -fdx; git pull;
mkdir -p /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModder-sandbox-Release;
cd /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModder-sandbox-Release;
qmake ../MassEffectModder/MassEffectModder.pro; make -j$NUM_THREADS;
cd /home/$USER_NAME/builds/sources/MassEffectModder/MassEffectModder; ./deploy_gui_linux.sh;
mkdir -p /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModderNoGui-sandbox-Release;
cd /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModderNoGui-sandbox-Release;
qmake ../MassEffectModder/MassEffectModderNoGui.pro; make -j$NUM_THREADS
cd /home/$USER_NAME/builds/sources/MassEffectModder/MassEffectModder; ./deploy_nogui_linux.sh;"
	sudo umount $BASE_CHROOT/proc
fi
