module;

#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <alsa/asoundlib.h>

export module audio_capture;

export class AudioCapture {
 public:
  AudioCapture(unsigned int sample_rate, std::string device = "plughw:DEV=0,CARD=0") : sample_rate(sample_rate) {
    int err;
    if ((err = snd_pcm_open(&capture_handle, device.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) {
      fprintf(stderr, "Can't open audio device %s (%s)\n", device.c_str(), snd_strerror(err));
      exit(1);
    }
    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
      fprintf(stderr, "Can't allocate hardware parameter structure (%s)\n", snd_strerror(err));
      exit(1);
    }
    if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
      fprintf(stderr, "Can't initialize hardware parameter structure (%s)\n", snd_strerror(err));
      exit(1);
    }
    if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
      fprintf(stderr, "Can't set access type (%s)\n", snd_strerror(err));
      exit(1);
    }
    if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format)) < 0) {
      fprintf(stderr, "Can't set sample format (%s)\n", snd_strerror(err));
      exit(1);
    }
    if ((err = snd_pcm_hw_params_set_rate(capture_handle, hw_params, sample_rate, 0)) < 0) {
			fprintf (stderr, "Can't set sample rate (%s)\n", snd_strerror(err));
			exit(1);
		}
		if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, channels)) < 0) {
			fprintf(stderr, "Can't set channel count (%s)\n", snd_strerror(err));
			exit(1);
		}
		if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
			fprintf(stderr, "Can't set parameters (%s)\n", snd_strerror(err));
			exit(1);
		}
		snd_pcm_hw_params_free(hw_params);
  }

  void start(std::function<void(float* samples, unsigned int count)> handler) {
    int err;
    if ((err = snd_pcm_prepare(capture_handle)) < 0) {
      fprintf(stderr, "Can't prepare audio interface for use (%s)\n", snd_strerror(err));
      exit(1);
    }
    unsigned int sample_count = sample_rate * 1;
    char buffer[snd_pcm_format_width(format) / 8 * channels * sample_count];
    for ( ; ; ) {
      if ((err = snd_pcm_readi(capture_handle, buffer, sample_count)) != sample_count) {
        fprintf(stderr, "Read from audio interface failed: %s\n", snd_strerror (err));
        exit(1);
      }
      handler((float*)buffer, sample_count);
    }
  }
 private:
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_FLOAT_LE;
  unsigned int channels = 1;
  unsigned int sample_rate;
};
