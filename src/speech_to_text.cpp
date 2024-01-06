module;

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "whisper.h"

export module speech_to_text;

constexpr unsigned int window_sample_count = WHISPER_SAMPLE_RATE * 8;

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

  void infer(const float *samples, int count) {
    //fprintf(stderr, "Inferring...\n");
    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    //params.print_progress = true;
    params.n_threads = 8;
    params.suppress_blank = true;
    params.suppress_non_speech_tokens = true;
    memcpy(window, &window[count], (window_sample_count - count) * sizeof(float));
    memcpy(&window[window_sample_count - count], samples, count * sizeof(float));
    if (whisper_full(ctx, params, window, window_sample_count) != 0) {
        fprintf(stderr, "Whisper failed to process audio\n");
        exit(1);
    }
    const int n_segments = whisper_full_n_segments(ctx);
    for (int i = 0; i < n_segments; ++i) {
      const char * text = whisper_full_get_segment_text(ctx, i);
      printf("%s", text);
    }
    printf("\n");
  }
 private:
  whisper_context* ctx;
  float window[window_sample_count];
};
