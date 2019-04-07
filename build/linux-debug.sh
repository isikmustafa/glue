cd ..
mkdir sample_output
mkdir debug
cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cd 3rd/tinyxml2
make -j 8
cd ../..
make -j 8
