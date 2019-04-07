# glue
[A Physically Based Renderer](https://glue.mustafaisik.net/)

# build
* ```git clone --recurse-submodules https://github.com/isikmustafa/glue.git```
* ```cd glue/build```

* Release Build for Windows-MinGW:
  * ```mingw-release.bat```
  * ```glue.exe ../sample_input/cbox.xml```
* Release Build for Linux:
  * ```source linux-release.sh```
  * ```./glue ../sample_input/cbox.xml```
  
* For others:
  * Tweak build scripts