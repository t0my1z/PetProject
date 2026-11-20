1 Commands to build in power shell
Remove-Item -Recurse -Force build
Remove-Item -Recurse -Force install
cmake -B build
cmake --build build --config Debug/Release
cmake --install build --prefix ./install --config Release

