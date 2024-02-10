#ifndef AUDIO_PLAYBACK_H_
#define AUDIO_PLAYBACK_H_

#include <alsa/asoundlib.h>
#include <string>

class AudioPlayback {
 public:
  AudioPlayback(unsigned int sample_rate, std::string device);
  void open();
  void play(char *buffer, int count);
  void close();

 private:
  snd_pcm_t *handle;
  unsigned int sample_rate;
  std::string device;
};

#endif  // AUDIO_PLAYBACK_H_
