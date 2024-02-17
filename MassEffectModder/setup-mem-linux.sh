#!/bin/sh

#
# MIT License
#
# Copyright (c) 2019-2024 Pawel Kolodziejski
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

BASE_PATH=/opt/stuff/schroot
DISTRO_BASE=debian
DISTRO_NAME=bookworm
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
QT_VERSION=6.6.2
QT_VERSION_BASE=`echo $QT_VERSION | cut -d'.' -f 1,2`
PACKAGES_ADD=bash,build-essential,ninja-build,clang,nasm,git,perl,python3,wget,\
ca-certificates,libx11-dev,libsdl2-dev,libopenal-dev,libfontconfig1-dev,libssl-dev,\
libxkbcommon-dev,libxkbcommon-x11-dev,libxcomposite-dev,libx11-xcb-dev,\
libglu1-mesa-dev,libxrender-dev,libxi-dev,fuse,file,appstream,zip

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
	$CHROOT_CMD_ROOT "apt-get -y install libxcb.*-dev cmake/$DISTRO_NAME-backports"
	touch .stamp-backports
fi

if [ ! -f .stamp-appimage ]; then
	$CHROOT_CMD_ROOT "mknod /dev/fuse c 10 229; chmod 666 /dev/fuse;
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
wget https://download.qt.io/archive/qt/$QT_VERSION_BASE/$QT_VERSION/single/qt-everywhere-src-${QT_VERSION}.tar.xz;
tar xf qt-everywhere-src-$QT_VERSION.tar.xz;
rm -f qt-everywhere-src-$QT_VERSION.tar.xz;
rm qt-everywhere-src-$QT_VERSION/qtbase/cmake/FindWrapZSTD.cmake;
touch qt-everywhere-src-$QT_VERSION/qtbase/cmake/FindWrapZSTD.cmake;
git clone https://github.com/MassEffectModder/MassEffectModder.git;
cd /home/$USER_NAME/builds/sources/MassEffectModder;
git checkout $BRANCH"
	touch .stamp-sources
fi

if [ ! -f .stamp-qt-prepare ]; then
	sudo mount --bind /proc $BASE_CHROOT/proc
	$CHROOT_CMD_USER "cd /home/$USER_NAME/builds/sources/qt-everywhere-src-$QT_VERSION; \
cmake \
-Wno-dev --log-level=STATUS -G Ninja -DBUILD_SHARED_LIBS=OFF -DFEATURE_static_runtime=ON \
-DINPUT_zlib=qt -DINPUT_pcre=qt -DINPUT_libpng=qt -DINPUT_libjpeg=qt -DINPUT_doubleconversion=qt \
-DINPUT_harfbuzz=qt -DINPUT_opengl=OFF -DFEATURE_dbus=OFF -DFEATURE_icu=OFF -DFEATURE_cups=OFF \
-DFEATURE_gif=OFF -DFEATURE_ico=OFF -DFEATURE_eglfs=OFF -DFEATURE_gbm=OFF -DFEATURE_tiff=OFF \
-DFEATURE_webp=OFF -DFEATURE_journald=OFF -DFEATURE_syslog=OFF -DFEATURE_mimetype-database=OFF \
-DFEATURE_slog2=OFF -DFEATURE_feature-relocatable=OFF -DFEATURE_opengl=OFF \
-DQT_BUILD_TESTS=OFF -DQT_BUILD_EXAMPLES=OFF \
-DBUILD_qt3d=OFF -DBUILD_qt5compat=OFF -DBUILD_qtactiveqt=OFF -DBUILD_qtcoap=OFF -DBUILD_qtcharts=OFF \
-DBUILD_qtconnectivit=OFF -DBUILD_qtdatavis3d=OFF -DBUILD_qtdeclarative=OFF -DBUILD_qtdoc=OFF \
-DBUILD_qtgrpc=OFF -DBUILD_qtgraphs=OFF -DBUILD_qthttpserver=OFF -DBUILD_qtgraphicaleffects=OFF \
-DBUILD_qttools=ON -DBUILD_qtlocation=OFF -DBUILD_qtlottie=OFF -DBUILD_qtmqtt=OFF -DBUILD_qtmultimedia=OFF \
-DBUILD_qtnetworkauth=OFF -DBUILD_qtopcua=OFF -DBUILD_qtpositioning=OFF -DBUILD_qtquick3d=OFF \
-DBUILD_qtquick3dphysics=OFF -DBUILD_qtquickeffectmaker=OFF -DBUILD_qtquicktimeline=OFF \
-DBUILD_qtremoteobjects=OFF -DBUILD_qtscxml=OFF -DBUILD_qtsensors=OFF -DBUILD_qtserialbus=OFF \
-DBUILD_qtserialport=OFF -DBUILD_qtshadertools=OFF -DBUILD_qtspeech=OFF -DBUILD_qtsvg=OFF \
-DBUILD_qttools=OFF -DBUILD_qttranslations=OFF -DBUILD_qtvirtualkeyboard=OFF -DBUILD_qtwayland=OFF \
-DBUILD_qtwebchannel=OFF -DBUILD_qtwebengine=OFF -DBUILD_qtwebsockets=OFF -DBUILD_qtwebview=OFF \
-DQT_QMAKE_TARGET_MKSPEC=linux-clang -DFEATURE_glib=ON -DFEATURE_system_freetype=ON \
-DFEATURE_system_fontconfig=ON \
-DINPUT_qpa=xcb -DFEATURE_xcb=ON -DFEATURE_xcb-native-painting=ON \
-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr"
	sudo umount $BASE_CHROOT/proc
	touch .stamp-qt-prepare
fi

sudo -v
if [ ! -f .stamp-qt-build ]; then
	sudo mount --bind /proc $BASE_CHROOT/proc
	$CHROOT_CMD_USER "cd /home/$USER_NAME/builds/sources/qt-everywhere-src-$QT_VERSION; cmake --build . --parallel $NUM_THREADS"
	$CHROOT_CMD_ROOT "cd /home/$USER_NAME/builds/sources/qt-everywhere-src-$QT_VERSION; cmake --install ."
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

sudo umount $BASE_CHROOT/tmp/.mount_appima*
sudo umount $BASE_CHROOT/tmp/.mount_linux*
