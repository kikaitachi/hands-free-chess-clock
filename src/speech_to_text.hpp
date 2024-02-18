#ifndef SPEECH_TO_TEXT_H_
#define SPEECH_TO_TEXT_H_

#include <chrono>
#include <cstring>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <thread>
#include "mpscq.hpp"
#include "whisper.h"

using namespace std::chrono_literals;

constexpr unsigned int window_sample_count = WHISPER_SAMPLE_RATE * 10;

class SpeechToText {
 public:
  SpeechToText();

  /**
   * Get sample rate. Only samples with this rate should be provided to
   * add_audio method.
   */
  unsigned int getSampleRate();

  /**
   * Start recognizing speech.
   * on_ready will be called only once after the first recognition to inform
   * that model is loaded and next recognition event will be faster.
   * on_speech is called on every successful recognition event.
   */
  void start(
    std::function<void()> on_ready,
    std::function<void(const std::string speech)> on_speech
  );

  /**
   * Add more audio.
   */
  void add_audio(std::vector<float>& audio, int start, int end);

 private:
  whisper_context* ctx;
  mpscq::Queue<std::vector<float>> queue;

  void infer(
    std::function<void()> on_ready,
    std::function<void(const std::string speech)> on_speech
  );
};

#endif  // SPEECH_TO_TEXT_H_
