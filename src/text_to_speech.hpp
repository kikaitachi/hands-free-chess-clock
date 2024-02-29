#ifndef TEXT_TO_SPEECH_H_
#define TEXT_TO_SPEECH_H_

#include "audio_playback.hpp"
#include "process.hpp"

class TextToSpeech {
 public:
  TextToSpeech(unsigned int sample_rate, std::string device, Process& piper);

  void say(std::string text);

 private:
  unsigned int sample_rate;
  std::string device;
  Process& piper;

  void play();
};

#endif  // TEXT_TO_SPEECH_H_
