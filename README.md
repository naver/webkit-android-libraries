## WebKitLibraries for Android
Source archive of the build requirements for the Android port of WebKit.

## How to Build
Requires NDK r12 or later and CMake 3.4 or later.
With NDK and CMake installed, do something like below:

For Windows,
```
mkdir build
cd build
cmake -G "Visual Studio 14 2015" -DWEBKIT_LIBRARIES_DIR=<path-to-webkit-libraries> ..
cmake --build .
```
For Linux,
```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../android.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DANDROID_TOOLCHAIN_NAME=clang -DANDROID_ABI="armeabi-v7a with NEON" -DWEBKIT_LIBRARIES_DIR=<path-to-webkit-libraries> ..
cmake --build .
```

`<path-to-webkit-libraries>` is typically the location of /WebKitLibraries under WebKit source tree.

## About Libraries
See [NOTICE](NOTICE) for details.
