module;

#include <stdio.h>
#include <stdlib.h>
#include <string>
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
    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
      fprintf(stderr, "Can't allocate hardware parameter structure (%s)\n", snd_strerror(err));
      exit (1);
    }
    if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
      fprintf(stderr, "Can't initialize hardware parameter structure (%s)\n", snd_strerror(err));
      exit (1);
    }
    if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
      fprintf(stderr, "Can't set access type (%s)\n", snd_strerror(err));
      exit(1);
    }
    if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format)) < 0) {
      fprintf(stderr, "Can't set sample format (%s)\n", snd_strerror(err));
      exit (1);
    }
  }

  void start() {
    snd_pcm_start(capture_handle);
  }
 private:
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
};
