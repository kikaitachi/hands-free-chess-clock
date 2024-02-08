#ifndef AUDIO_CAPTURE_H_
#define AUDIO_CAPTURE_H_

#include <alsa/asoundlib.h>
#include <string>

class AudioPlayback {
 public:
  AudioPlayback(unsigned int sample_rate, std::string device);
  ~AudioPlayback();

 private:
  snd_pcm_t *handle;
};

#endif  // AUDIO_CAPTURE_H_
