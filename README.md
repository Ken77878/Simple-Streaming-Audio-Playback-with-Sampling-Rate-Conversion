# Simple Streaming Audio Playback Application with Sampling-Rate Conversion(wav, ogg)

## Features
This application can playback multiple audio files simultaneously.

## Requrements
cross-platform application(This application was tested in Windows7 and Ubuntu18.04.)

This application works normally only in little-endian machines.

* CMake 3.10.2

## Build

You need to be able to execute "git clone" using ssh protocol
because libogg and libvorbis are cloned.

Make a Directory for build in the highest level possible not to exceed path length limit
particularly in Windows.

And go to that directory.

### Windows build command example:

#### To build 32bit application in debug version

**cmake relative-pass-to-code-folder -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 15 2017"**

**cmake --build . --config Debug**

#### To build 64bit application in release version

**cmake relative-pass-to-code-folder -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 15 2017 Win64"**

**cmake --build . --config Release**

### Linux build command example:

#### To build in debug version

**cmake relative-pass-to-code-folder -DCMAKE_BUILD_TYPE=Debug**

**make**

#### To build in debug version

**cmake relative-pass-to-code-folder -DCMAKE_BUILD_TYPE=Release**

**make**

## Usage
Please edit the path constants of audio files in audioTest.cpp(audioPlayer/audioPlayer/code/audioTestShared/OS/audioTest.cpp). 






