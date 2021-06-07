#!/bin/sh

#
# MIT License
#
# Copyright (c) 2019-2021 Pawel Kolodziejski
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
USER_NAME=aquadran

DISTRO_BASE=debian
DISTRO_ARCH=amd64

PROJECT=mem
BRANCH=master

NUM_THREADS=`grep -c '^processor' /proc/cpuinfo`

BASE_CHROOT=$BASE_PATH/$PROJECT-$DISTRO_BASE-$DISTRO_ARCH

CHROOT_CMD_USER="schroot --user=$USER_NAME --directory=/home/$USER_NAME --chroot=$PROJECT-$DISTRO_BASE-$DISTRO_ARCH -- bash -c "

sudo mount --bind /proc $BASE_CHROOT/proc
$CHROOT_CMD_USER "cd /home/$USER_NAME/builds/sources/MassEffectModder; git clean -fdx; git pull; git checkout $BRANCH;
mkdir -p /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModder-sandbox-Release;
cd /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModder-sandbox-Release;
qmake ../MassEffectModder/MassEffectModder.pro; make -j$NUM_THREADS;
cd /home/$USER_NAME/builds/sources/MassEffectModder/MassEffectModder; ./deploy_gui_linux.sh;
mkdir -p /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModderNoGui-sandbox-Release;
cd /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModderNoGui-sandbox-Release;
qmake ../MassEffectModder/MassEffectModderNoGui.pro; make -j$NUM_THREADS
cd /home/$USER_NAME/builds/sources/MassEffectModder/MassEffectModder; ./deploy_nogui_linux.sh;
mkdir -p /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModderWrappers-sandbox-Release;
cd /home/$USER_NAME/builds/sources/MassEffectModder/build-MassEffectModderWrappers-sandbox-Release;
qmake ../MassEffectModder/MassEffectModderWrappers.pro; make -j$NUM_THREADS;"
sudo umount $BASE_CHROOT/proc
