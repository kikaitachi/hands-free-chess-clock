# Hands Free Chess Clock

![Last build result](https://github.com/kikaitachi/hands-free-chess-clock/workflows/CI/badge.svg)

## Building & running

Install build tools dependencies:
```
sudo apt-get install cmake ninja-build libasound2-dev libopencv-dev
```

Create build configuration (you need to run this only once):
```
cmake -B build -G Ninja
```

Build:
```
cmake --build build
```

Download Whisper model file (you need to run this only once):
```
bash build/_deps/whisper.cpp-src/models/download-ggml-model.sh small.en
```

Run:
```
build/bin/hands-free-chess-clock
```

## Using

Clock is controlled by the following voice commands:
* `start x minute(s) game` - starts a new game. All chess pieces must be at their initial squares. After starting game video camera will observe the board and automatically switch relevant clock after each move.
* `stop the game` - will stop the clock.
* `continue game` - will resume clock.
* `switch the clock` - manually switch which clock is active. Intended for debugging only.

## Credits

This project directly or indirectly uses these open source projects:
* [Advanced Linux Sound Architecture project](https://github.com/alsa-project/alsa-lib)
* [CMake](https://github.com/Kitware/CMake)
* [Open Source Computer Vision Library](https://github.com/opencv/opencv)
* [SparkFun Alphanumeric Display Library](https://github.com/sparkfun/SparkFun_Alphanumeric_Display_Arduino_Library)
* [whisper.cpp](https://github.com/ggerganov/whisper.cpp)

Thanks to [Nvidia](https://www.nvidia.com/) for providing [Jetson Orin Nano Developer Kit](https://developer.nvidia.com/embedded/learn/get-started-jetson-orin-nano-devkit) via [AI Innovation Challenge](https://www.hackster.io/contests/SparkFun-NVIDIA-AI-Innovation-Challenge) contest.
