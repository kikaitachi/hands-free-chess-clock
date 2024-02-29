#!/bin/bash

set -e

build_dir=build
model_file="${build_dir}/_deps/whisper.cpp-src/models/ggml-small.en.bin"
silero_model="models/silero_vad.onnx"
piper_onnx="models/en_US-amy-medium.onnx"
piper_json="${piper_onnx}.json"

mkdir -p models
if [ ! -f "${silero_model}" ]; then
  curl -s -S https://raw.githubusercontent.com/snakers4/silero-vad/master/files/silero_vad.onnx > "${silero_model}"
fi
if [ ! -f "${piper_onnx}" ]; then
  curl -s -S https://huggingface.co/rhasspy/piper-voices/resolve/v1.0.0/en/en_US/amy/medium/en_US-amy-medium.onnx > "${piper_onnx}"
fi
if [ ! -f "${piper_json}" ]; then
  curl -s -S https://huggingface.co/rhasspy/piper-voices/resolve/v1.0.0/en/en_US/amy/medium/en_US-amy-medium.onnx.json > "${piper_json}"
fi

if [ ! -d "${build_dir}" ]; then
  cmake -B "${build_dir}" -G Ninja
fi

cmake --build "${build_dir}"

if [ ! -f "${model_file}" ]; then
  bash "${build_dir}/_deps/whisper.cpp-src/models/download-ggml-model.sh" small.en
fi
