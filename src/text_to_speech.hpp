#ifndef TEXT_TO_SPEECH_H_
#define TEXT_TO_SPEECH_H_

#include "audio_playback.hpp"

class TextToSpeech {
 public:
  TextToSpeech(unsigned int sample_rate, std::string device);

 private:
  AudioPlayback audio_playback;
};

#endif  // TEXT_TO_SPEECH_H_
