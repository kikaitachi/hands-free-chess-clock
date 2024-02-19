# Hands Free Chess Clock

[![Last build result](https://github.com/kikaitachi/hands-free-chess-clock/workflows/CI/badge.svg)](https://github.com/kikaitachi/hands-free-chess-clock/actions)
[![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-%23FE5196?logo=conventionalcommits&logoColor=white)](https://conventionalcommits.org)

A chess clock which doesn't need to be touched. It listens to voice commands
to start a game and automatically switches which clock is stopped and which is
running by watching a game using the camera.

## Example hardware

![Hands Free Chess Clock](clock.webp)

Main components used to build clock are listed in [AI Innovation Challenge](https://www.hackster.io/contests/SparkFun-NVIDIA-AI-Innovation-Challenge) contest page.

## Project goals

This is engineering not research project, i.e. objective is to provide usable
solution on certain hardware using a particular chess board or boards and not
solve generic chess board and position detection problem from arbitrary angle
and lighting conditions.

Therefore it is almost certain that project will not work out of the box on
your hardware using your chess set without tweaking the code or configuration.
Bug reports are welcome and we want to expand support for more diverse setups.

## Reporting a bug

Software will store a lot of information (mostly images) in the autogenerated
`debug` folder. This folder is recreated at the start of the game so will only
contain information about the last game. Please include contents of that
directory in bug reports to help diagnosing the root cause of the bug.

## Building & running

Install build tools and dependencies:
```
sudo apt-get install g++ cmake ninja-build libasound2-dev libopencv-dev
```

Build (and run unit tests):
```
./build.sh
```

Run:
```
./run.sh [audio_input [audio_output]]
```
Where `audio_input` is ALSA capture device name, for example `plughw:DEV=0,CARD=C920` and
`audio_output` is ALSA playback device name. Both default to `default` if not specified.

## Using

Clock is controlled by the following voice commands:
* `start x minute(s) game [with y second increment]` - starts a new game. All chess pieces must be at their initial squares. After starting game video camera will observe the board and automatically switch relevant clock after each move.
* `stop the game` - will stop the clock.
* `continue game` - will resume clock.

## Future work

* Improve promotion experience. Most of hobby chess sets don't have spare queens so players might put something else to represent queen which could be recognized as under promotion. Solution potentially could be adding a new voice command `promoting to piece_name`.
* Put camera on at least 1 axis gimbal so it can find chess board automatically if it is out of view.

## Credits

This project directly or indirectly uses these open source projects:
* [Advanced Linux Sound Architecture project](https://github.com/alsa-project/alsa-lib)
* [Bluetooth Audio ALSA Backend](https://github.com/arkq/bluez-alsa) (to enable BT on Jetson [see](https://docs.nvidia.com/jetson/archives/r34.1/DeveloperGuide/text/SD/Communications/EnablingBluetoothAudio.html))
* [CMake](https://github.com/Kitware/CMake)
* [Open Neural Network Exchange Runtime](https://github.com/microsoft/onnxruntime)
* [Open Source Computer Vision Library](https://github.com/opencv/opencv)
* [Silero Voice Activity Detector](https://github.com/snakers4/silero-vad)
* [SparkFun Alphanumeric Display Library](https://github.com/sparkfun/SparkFun_Alphanumeric_Display_Arduino_Library)
* [whisper.cpp](https://github.com/ggerganov/whisper.cpp)

Thanks to [Nvidia](https://www.nvidia.com/) for providing [Jetson Orin Nano Developer Kit](https://developer.nvidia.com/embedded/learn/get-started-jetson-orin-nano-devkit) via [AI Innovation Challenge](https://www.hackster.io/contests/SparkFun-NVIDIA-AI-Innovation-Challenge) contest.
