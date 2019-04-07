cd ..
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..
cd 3rd/tinyxml2
mingw32-make -j8
copy /B libtinyxml2.dll "../.."
cd ../..
mingw32-make -j8