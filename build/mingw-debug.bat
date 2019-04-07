cd ..
mkdir sample_output
mkdir debug
cd debug
cmake -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" ..
cd 3rd/tinyxml2
mingw32-make -j 8
copy /B libtinyxml2d.dll "../.."
cd ../..
mingw32-make -j 8