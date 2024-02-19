#!/bin/bash

if [ -f "build/bin/hands-free-chess-clock" ]; then
  build/bin/hands-free-chess-clock "$@"
elif [ -f "build/hands-free-chess-clock" ]; then
  build/hands-free-chess-clock "$@"
else
  echo "ERROR: can't find binary, did't you forget to run ./build.sh first?"
fi
