module;

#include "whisper.h"

export module speech_to_text;

export class SpeechToText {
 public:
  SpeechToText() {
    struct whisper_context_params cparams;
    cparams.use_gpu = true;
    ctx = whisper_init_from_file_with_params("base", cparams);
  }
 private:
  whisper_context* ctx;
};
