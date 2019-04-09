# glue
[A Physically Based Renderer](https://glue.mustafaisik.net/)

# build
Make sure that your compiler fully supports C++17. For example, if you are using g++, you need at least version 7.
* ```git clone --recurse-submodules https://github.com/isikmustafa/glue.git```
* ```cd glue/build```

* Release Build for Windows-MinGW:
  * ```mingw-release.bat```
  * ```glue.exe ../sample_input/cbox.xml```
* Release Build for Unix-like:
  * ```source linux-release.sh```
  * ```./glue ../sample_input/cbox.xml```
* For others:
  * Tweak build scripts