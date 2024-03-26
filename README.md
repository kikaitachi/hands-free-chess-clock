# Hands Free Chess Clock

[![Last build result](https://github.com/kikaitachi/hands-free-chess-clock/workflows/CI/badge.svg)](https://github.com/kikaitachi/hands-free-chess-clock/actions)
[![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-%23FE5196?logo=conventionalcommits&logoColor=white)](https://conventionalcommits.org)

A chess clock which doesn't need to be touched. It listens to voice commands
to start a game and automatically switches which clock is stopped and which is
running by watching a game using the camera.

## Example hardware

[<img src="clock.webp">](https://youtu.be/G-w0O12VTRc)

Components:
* [NVIDIA Jetson Orin Nano Developer Kit](https://marketplace.nvidia.com/en-us/robotics-edge/)
* [SparkFun Qwiic Alphanumeric Display - Purple](https://www.sparkfun.com/products/16918)
* [SparkFun Qwiic Cable - 100mm](https://www.sparkfun.com/products/14427)
* [Logitech C920 HD WebCam](https://www.logitech.com/en-gb/products/webcams/c920-pro-hd-webcam.960-001055.html)
* [Mini External USB Stereo Speaker](https://www.sparkfun.com/products/18343)
* Frame is made mostly from [eitech](https://www.eitech.de/) construction set parts, rubber feet are from old printer rollers, camera mounting plate is 3D printed


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
sudo apt-get install g++ cmake ninja-build libasound2-dev libopencv-dev stockfish
```

`stockfish` is not needed for main functionality. It is used only if you want training mode to find out the best moves, who is winning, etc. It also can be replaced by any UCI compatible engine, see bellow.

Build (and run unit tests):
```
./build.sh
```

Run:
```
./run.sh [audio_input [audio_output [uci_engine]]]
```
Where `audio_input` is ALSA capture device name, for example, `plughw:DEV=0,CARD=C920`.`audio_output` is ALSA playback device name, for example, `plughw:CARD=UACDemoV10,DEV=0`. Both audio parameters defaults to `default` if not specified.
The last `uci_engine` parameter specifies executable (with full path) supporting [Universal Chess Interface (UCI)](https://en.wikipedia.org/wiki/Universal_Chess_Interface). If not specified defaults to [/usr/games/stockfish](https://github.com/official-stockfish/Stockfish).

## Using

Clock is controlled by the following voice commands:
* `start x minute(s) game [with y second increment]` - starts a new game. All chess pieces must be at their initial squares. After starting game video camera will observe the board and automatically switch relevant clock after each move.
* `stop the game` - will stop the clock.
* `continue game` - will resume clock.
* `shutdown` - will halt computer running the clock. Handy when you don't have keyboard or remote terminal for a safe shutdown. /etc/sudoers file must be [modified accordingly](https://askubuntu.com/questions/168879/shutdown-from-terminal-without-entering-password).
* `please tell best move` - will use external chess engine to evaluate current position and will tell what it thinks the best move is.
* `what is worst move` - will use external chess engine to evaluate current position and will tell what it thinks the worst move is.
* `who is winning` - will use external chess engine to evaluate current position and will tell who is winning.

## Future work

* Improve promotion experience. Most of hobby chess sets don't have spare queens so players might put something else to represent queen which could be recognized as under promotion. Solution potentially could be adding a new voice command `promoting to piece_name`.
* Put camera on at least 1 axis gimbal so it can find chess board automatically if it is out of view.

## Credits

This project directly or indirectly uses these open source projects:
* [Advanced Linux Sound Architecture project](https://github.com/alsa-project/alsa-lib)
* [Chess opening names](https://github.com/lichess-org/chess-openings)
* [CMake](https://github.com/Kitware/CMake)
* [Open Neural Network Exchange Runtime](https://github.com/microsoft/onnxruntime)
* [Open Source Computer Vision Library](https://github.com/opencv/opencv)
* [Piper](https://github.com/rhasspy/piper)
* [Silero Voice Activity Detector](https://github.com/snakers4/silero-vad)
* [SparkFun Alphanumeric Display Library](https://github.com/sparkfun/SparkFun_Alphanumeric_Display_Arduino_Library)
* [Stockfish](https://github.com/official-stockfish/Stockfish)
* [whisper.cpp](https://github.com/ggerganov/whisper.cpp)

Thanks to [Nvidia](https://www.nvidia.com/) for providing [Jetson Orin Nano Developer Kit](https://developer.nvidia.com/embedded/learn/get-started-jetson-orin-nano-devkit) via [AI Innovation Challenge](https://www.hackster.io/contests/SparkFun-NVIDIA-AI-Innovation-Challenge) contest and to [hackster.io](https://www.hackster.io/) and [SparkFun](https://www.sparkfun.com/) for organizing and sponsoring it.
