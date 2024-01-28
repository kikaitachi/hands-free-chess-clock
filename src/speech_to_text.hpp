#ifndef SPEECH_TO_TEXT_H_
#define SPEECH_TO_TEXT_H_

#include <chrono>
#include <cstring>
#include <functional>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <thread>
#include "whisper.h"

using namespace std::chrono_literals;

constexpr unsigned int window_sample_count = WHISPER_SAMPLE_RATE * 10;

class SpeechToText {
 public:
  SpeechToText();

  unsigned int getSampleRate();

  void start(std::function<void(const std::string speech)> handler);

  void add_audio(const float *samples, int count);

 private:
  whisper_context* ctx;
  float window[window_sample_count];
  std::mutex window_mutex;

  void infer(std::function<void(const std::string speech)> handler);
};

#endif  // SPEECH_TO_TEXT_H_