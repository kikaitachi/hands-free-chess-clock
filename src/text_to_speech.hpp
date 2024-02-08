#ifndef TEXT_TO_SPEECH_H_
#define TEXT_TO_SPEECH_H_

#include "audio_playback.hpp"

class TextToSpeech {
 public:
  TextToSpeech(std::string device);

  void say(std::string text);

 private:
  unsigned int sample_rate = 24000;
  AudioPlayback audio_playback;

  void play(std::string text);
};

#endif  // TEXT_TO_SPEECH_H_
