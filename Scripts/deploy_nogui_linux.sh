build_path=$(cd ../build-MassEffectModderNoGui-*-Release; pwd)
cp $build_path/MassEffectModder/MassEffectModderNoGui .
version=`grep -E "^VERSION" ../MassEffectModder/MassEffectModder/Program/Version.pri | grep -o -E "[0-9]+"`
zip MassEffectModderNoGui-Linux-v$version.zip MassEffectModderNoGui -9
rm MassEffectModderNoGui
