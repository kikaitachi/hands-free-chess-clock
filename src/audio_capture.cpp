module;

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

export module audio_capture;

export class AudioCapture {
 public:
  AudioCapture(std::string device = "hw:0") {
    int err;
    if ((err = snd_pcm_open(&capture_handle, device.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) {
      fprintf(stderr, "Can't open audio device %s (%s)\n", device.c_str(), snd_strerror(err));
      exit(1);
    }
  }

  void start() {
  }
 private:
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
};
