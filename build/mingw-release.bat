cd ..
mkdir sample_output
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..
cd 3rd/tinyxml2
mingw32-make -j 8
copy /B libtinyxml2.dll "../.."
cd ../..
mingw32-make -j 8