module;

import logger;

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

export module speech_to_text;

using namespace std::chrono_literals;

constexpr unsigned int window_sample_count = WHISPER_SAMPLE_RATE * 10;

export class SpeechToText {
 public:
  SpeechToText() {
    struct whisper_context_params cparams;
    cparams.use_gpu = true;
    ctx = whisper_init_from_file_with_params("build/_deps/whisper.cpp-src/models/ggml-tiny.en.bin", cparams);
  }

  unsigned int getSampleRate() {
    return WHISPER_SAMPLE_RATE;
  }

  void start(std::function<void(const std::string speech)> handler) {
    std::thread inference_thread(&SpeechToText::infer, this, handler);
    inference_thread.detach();
  }

  void add_audio(const float *samples, int count) {
    std::lock_guard<std::mutex> guard(window_mutex);
    memmove(window, &window[count], (window_sample_count - count) * sizeof(float));
    memmove(&window[window_sample_count - count], samples, count * sizeof(float));
  }

 private:
  whisper_context* ctx;
  float window[window_sample_count];
  std::mutex window_mutex;

  void infer(std::function<void(const std::string speech)> handler) {
    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_BEAM_SEARCH);
    params.n_threads = 4;
    params.suppress_blank = true;
    params.suppress_non_speech_tokens = true;
    params.language = "en";
    for ( ; ; ) {
      logger::debug("Whisper start");
      float window_copy[window_sample_count];
      window_mutex.lock();
      memcpy(window_copy, window, window_sample_count * sizeof(float));
      window_mutex.unlock();
      if (whisper_full(ctx, params, window_copy, window_sample_count) != 0) {
          fprintf(stderr, "Whisper failed to process audio\n");
          exit(1);
      }
      const int n_segments = whisper_full_n_segments(ctx);
      std::string result;
      logger::debug("Got %d segments", n_segments);
      for (int i = 0; i < n_segments; i++) {
        result += whisper_full_get_segment_text(ctx, i);
      }
      std::this_thread::sleep_for(100ms);
      handler(result);
    }
  }
};
