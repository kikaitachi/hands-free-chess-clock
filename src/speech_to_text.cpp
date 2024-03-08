#include "speech_to_text.hpp"
#include "logger.hpp"
#include <chrono>

void log_callback(enum ggml_log_level level, const char * text, void * user_data) {
  logger::debug("whisper: %.*s", strlen(text) - 1, text);
}

SpeechToText::SpeechToText() {
  whisper_log_set(log_callback, nullptr);
  struct whisper_context_params cparams;
  cparams.use_gpu = true;
  ctx = whisper_init_from_file_with_params("models/ggml-small.en.bin", cparams);
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

void SpeechToText::add_audio(std::vector<float>& audio, int start, int end) {
  queue.push(new std::vector<float>(audio.begin() + start, audio.begin() + end));
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
  on_ready();
  for ( ; ; ) {
    queue.drain([&](auto entry) {
      std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
      if (whisper_full(ctx, params, entry->data(), entry->size()) != 0) {
          logger::error("Whisper failed to process audio");
      }
      const int n_segments = whisper_full_n_segments(ctx);
      std::string result;
      for (int i = 0; i < n_segments; i++) {
        result += whisper_full_get_segment_text(ctx, i);
      }
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      unsigned int millis = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      logger::debug("Recognised %zu samples as '%s' in %dms", entry->size(), result.c_str(), millis);
      on_speech(result);
    });
    std::this_thread::sleep_for(10ms);
  }
}
