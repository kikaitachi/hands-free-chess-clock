module;

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "whisper.h"

export module speech_to_text;

constexpr unsigned int prev_sample_count = 0; // WHISPER_SAMPLE_RATE / 5;  // 200ms

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
    //params.prompt_tokens = prompt_tokens.data();
    //params.prompt_n_tokens = prompt_tokens.size();
    float buffer[count + prev_sample_count];
    memcpy(buffer, prev, prev_sample_count * sizeof(float));
    memcpy(&buffer[prev_sample_count], samples, count * sizeof(float));
    memcpy(prev, &buffer[count - prev_sample_count], prev_sample_count * sizeof(float));
    if (whisper_full(ctx, params, buffer, count + prev_sample_count) != 0) {
        fprintf(stderr, "Whisper failed to process audio\n");
        exit(1);
    }
    const int n_segments = whisper_full_n_segments(ctx);
    //fprintf(stderr, "Got %d segments\n", n_segments);
    for (int i = 0; i < n_segments; ++i) {
      const char * text = whisper_full_get_segment_text(ctx, i);
      //printf("%d: %s\n", i, text);
      printf("%s", text);
      fflush(stdout);
      prompt_tokens.clear();
      const int n_segments = whisper_full_n_segments(ctx);
      for (int i = 0; i < n_segments; ++i) {
          const int token_count = whisper_full_n_tokens(ctx, i);
          for (int j = 0; j < token_count; ++j) {
              prompt_tokens.push_back(whisper_full_get_token_id(ctx, i, j));
          }
      }
    }
  }
 private:
  whisper_context* ctx;
  float prev[prev_sample_count];
  std::vector<whisper_token> prompt_tokens;
};
