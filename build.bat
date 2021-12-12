@ECHO OFF

mkdir build 2> NUL
cd build

cmake ..
cmake --build .