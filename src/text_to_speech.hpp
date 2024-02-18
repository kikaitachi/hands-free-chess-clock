#ifndef TEXT_TO_SPEECH_H_
#define TEXT_TO_SPEECH_H_

#include "audio_playback.hpp"
#include "mpscq.hpp"

class TextToSpeech {
 public:
  TextToSpeech(unsigned int sample_rate, std::string device);

  void say(std::string text);

 private:
  unsigned int sample_rate;
  std::string device;
  mpscq::Queue<std::string> queue;

  void play();
};

#endif  // TEXT_TO_SPEECH_H_
