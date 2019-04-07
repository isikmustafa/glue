cd ..
mkdir sample_output
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=Release ..
cd 3rd/tinyxml2
make -j 8
cd ../..
make -j 8
