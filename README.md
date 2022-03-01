
# SCREEN RECORDER

Screen recorder library project for the PDS course at Politecnico di Torino.

## Dependencies

- `C++17+`
- `CMake 3.18+`
- `FFmpeg libraries (libav) 4.4.1+`
- `QT5 libraries 5.15+`

## Prerequisites

Building the test requires [CMake](https://cmake.org/), [FFmppeg](https://ffmpeg.org/) (libav) and [QT5](https://www.qt.io/) libraries installed.

### Linux - Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install cmake
sudo apt-get install libavcodec-dev libavformat-dev libavfilter-dev libavutil-dev libavdevice-dev libswscale-dev libswresample-dev pkg-config -y
sudo apt-get install qt5-default qt5-qmake qtmultimedia5-dev build-essential
```

If you face problem with the package `qt5-default` try with the following command 
```
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
```

### Windows
The simplest way to have all the dependencies needed for this project is to use package managers as `vcpkg`.

### MacOS
The simplest way to have all the dependencies needed for this project is to use package managers as `brew`.

## Build Instructions 
Run the script `build.sh` or alternatively the following commands:
```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```
## Description
This screen recorder allow users to develop a simple application with the following features:
- define the area to be recorded
- select whether the audio should be captured or not
- activate and stop the recording process
- temporarily pause and subsequently resume the recording process
- define the file that will contain the final recording

## Documentation
### Project Structure
```
ScreenRecorder
.
|__lib
|    |__include
|    |  |__ScreenRecorder.h
|    |     ...
|    |     Common.h
|    |__src
|       |__ScreenRecorder.cpp
|          ...
|          SettingsConf.cpp
|
|__media
|   |__outputFile.mp4
|__test
    |__src
        |__main.cpp
```
### Usage
- `void open_();`
This function takes care of getting from the user the initial configuration. It also sets up everything needed for audio/video input and output.  
- `void start_();`
This function is called to start the recording process.
- `void pause_();`
  This function is called to pause the recording process after it has been started.
- `void restart_();`
  This function is called to resume the recording process after it has been paused.
- `void stop_();`
  This function is called to stop the recording process after it has been started.
- `string getVideoInfo(); `
This function returns information about the actual state of the recording process.
- `string getAudioInfo();`
This function returns information about sample rate, bit rate and channels of the audio part of the recording (if present). 
- `string getRecState();`
This function returns information about width, height, fps of the video part of the recording.

## Compatibility
- Linux ***[tested]***
- Windows ***[tested]***
- MacOS ***[NOT tested]***

---
## Contributors 
- [Galliano Giacomo](https://github.com/giacomo-galliano)
- [Gulotta Dario Paolo](https://github.com/DarkoBiersack)

