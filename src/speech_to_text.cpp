#include "speech_to_text.hpp"
#include "logger.hpp"
#include <chrono>

SpeechToText::SpeechToText() {
  struct whisper_context_params cparams;
  cparams.use_gpu = true;
  ctx = whisper_init_from_file_with_params("build/_deps/whisper.cpp-src/models/ggml-small.en.bin", cparams);
  clear_audio();
}

unsigned int SpeechToText::getSampleRate() {
  return WHISPER_SAMPLE_RATE;
}

void SpeechToText::start(
    std::function<void()> on_ready,
    std::function<void(const std::string speech)> on_speech
  ) {
  std::thread inference_thread(&SpeechToText::infer, this, on_ready, on_speech);
  inference_thread.detach();
}

void SpeechToText::add_audio(const float *samples, int count) {
  std::lock_guard<std::mutex> guard(window_mutex);
  memmove(window, &window[count], (window_sample_count - count) * sizeof(float));
  memmove(&window[window_sample_count - count], samples, count * sizeof(float));
}

void SpeechToText::clear_audio() {
  std::lock_guard<std::mutex> guard(window_mutex);
  memset(window, 0, window_sample_count * sizeof(float));
}

void SpeechToText::infer(
    std::function<void()> on_ready,
    std::function<void(const std::string speech)> on_speech
  ) {
  whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_BEAM_SEARCH);
  params.n_threads = 4;
  params.suppress_blank = true;
  params.suppress_non_speech_tokens = true;
  params.language = "en";
  for ( ; ; ) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
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
    for (int i = 0; i < n_segments; i++) {
      result += whisper_full_get_segment_text(ctx, i);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    unsigned int millis = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    logger::debug("Recognised '%s' in %dms", result.c_str(), millis);
    if (!loaded) {
      loaded = true;
      on_ready();
    }
    std::this_thread::sleep_for(100ms);
    on_speech(result);
  }
}
