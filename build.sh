#!/bin/bash

set -e

build_dir=build
model_file="${build_dir}/_deps/whisper.cpp-src/models/ggml-small.en.bin"
silero_model="models/silero_en_v5.onnx"

mkdir -p models
if [ ! -f "${silero_model}" ]; then
  curl -s -S https://models.silero.ai/models/en/en_v5.onnx > "${silero_model}"
fi

if [ ! -d "${build_dir}" ]; then
  cmake -B "${build_dir}" -G Ninja
fi

cmake --build "${build_dir}"

if [ ! -f "${model_file}" ]; then
  bash "${build_dir}/_deps/whisper.cpp-src/models/download-ggml-model.sh" small.en
fi
