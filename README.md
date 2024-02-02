# Hands Free Chess Clock

![Last build result](https://github.com/kikaitachi/hands-free-chess-clock/workflows/CI/badge.svg)

## Building & running

Install dependencies:
```
sudo apt-get install cmake ninja libasound2-dev libopencv-dev
```

Create build configuration:
```
cmake -B build -G Ninja
```

Build:
```
cmake --build build
```

Run:
```
build/bin/hands-free-chess-clock
```

## Credits

This project directly or indirectly uses these open source projects:
* [Advanced Linux Sound Architecture project](https://github.com/alsa-project/alsa-lib)
* [CMake](https://github.com/Kitware/CMake)
* [Open Source Computer Vision Library](https://github.com/opencv/opencv)
* [SparkFun Alphanumeric Display Library](https://github.com/sparkfun/SparkFun_Alphanumeric_Display_Arduino_Library)
* [whisper.cpp](https://github.com/ggerganov/whisper.cpp)

Thanks to [Nvidia](https://www.nvidia.com/) for providing [Jetson Orin Nano Developer Kit](https://developer.nvidia.com/embedded/learn/get-started-jetson-orin-nano-devkit) via [AI Innovation Challenge](https://www.hackster.io/contests/SparkFun-NVIDIA-AI-Innovation-Challenge) contest.
