build_path=$(cd ../MassEffectModder/build/*-Release; pwd)
top_path=$build_path/deploy
bundle_path=$top_path/MassEffectModder.Appdir
mkdir -p $bundle_path/usr/bin
cp $build_path/MassEffectModder/MassEffectModder $bundle_path/usr/bin
cp ../MassEffectModder/MassEffectModder/Resources/MEM.png $bundle_path/MassEffectModder.png
version=`grep -E "^VERSION" ../MassEffectModder/MassEffectModder/Program/Version.pri | grep -o -E "[0-9]+"`
echo "#!/bin/sh
SELF=\$(readlink -f \"\$0\")
HERE=\${SELF%/*}
export PATH=\"\${HERE}/usr/bin/:\${HERE}/usr/sbin/:\${HERE}/bin/:\${HERE}/sbin/\${PATH:+:\$PATH}\"
export LD_LIBRARY_PATH=\"\${HERE}/usr/lib/:\${HERE}/lib/:\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}\"
EXEC=\$(grep -e '^Exec=.*' \"\${HERE}\"/*.desktop | head -n 1 | cut -d \"=\" -f 2 | cut -d \" \" -f 1)
exec \"\${EXEC}\" \"\$@\"" > $bundle_path/AppRun
chmod +x $bundle_path/AppRun
echo "[Desktop Entry]
Name=MassEffectModder
Exec=MassEffectModder
Icon=MassEffectModder
Type=Application
Categories=Utility;" > $bundle_path/MassEffectModder.desktop
linuxdeploy --appdir $bundle_path
appimagetool $bundle_path MassEffectModder.AppImage
zip MassEffectModder-Linux-v$version.zip MassEffectModder.AppImage -0
rm MassEffectModder.AppImage
