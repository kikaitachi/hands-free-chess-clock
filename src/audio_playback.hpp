#ifndef AUDIO_PLAYBACK_H_
#define AUDIO_PLAYBACK_H_

#include <alsa/asoundlib.h>
#include <string>

class AudioPlayback {
 public:
  AudioPlayback(unsigned int sample_rate, std::string device);
  ~AudioPlayback();
  void play(char *buffer, int count);

 private:
  snd_pcm_t *handle;
};

#endif  // AUDIO_PLAYBACK_H_
