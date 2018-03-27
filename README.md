# glue
Yet another path tracer with more experimental purpose.

# build
* It works with Visual Studio 2017 without a problem.
* Download [assimp 3.3.1](http://assimp.sourceforge.net/main_downloads.html) and [tinyxml2](https://github.com/leethomason/tinyxml2). Generate respective .lib and .dll files.
* Making [png++](https://www.nongnu.org/pngpp) work is a bit painful. [This](https://gist.github.com/UnaNancyOwen/3ee723cb31019a490f351301ea069c15) is enough for you to get going.
* Put generated __assimp-vc140-mt.lib__, __libpng16.lib__ and __tinyxml2.lib__ files under 3rd/libs directory.
* Open __glue.vcxproj__ and build.
