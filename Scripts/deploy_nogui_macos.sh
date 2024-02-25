build_path=$(cd ../build-MassEffectModderNoGui-*-Release/MassEffectModder; pwd)
cp $build_path/MassEffectModderNoGui .
version=`grep -E "^VERSION" ../MassEffectModder/MassEffectModder/Program/Version.pri | grep -o -E "[0-9]+"`
codesign --force --sign "Apple Development: Pawel Kolodziejski" MassEffectModderNoGui
zip MassEffectModderNoGui-macOS-v$version.zip MassEffectModderNoGui -9
rm MassEffectModderNoGui
