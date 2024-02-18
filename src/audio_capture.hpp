#ifndef AUDIO_CAPTURE_H_
#define AUDIO_CAPTURE_H_

#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <alsa/asoundlib.h>

class AudioCapture {
 public:
  AudioCapture(unsigned int sample_rate, std::string device);

  void start(std::function<void(std::vector<float>& audio, int start, int end)> handler);
 private:
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_FLOAT_LE;
  unsigned int channels = 1;
  unsigned int sample_rate;
};

#endif  // AUDIO_CAPTURE_H_
