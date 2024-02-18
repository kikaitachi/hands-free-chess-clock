#include "audio_capture.hpp"
#include "logger.hpp"
#include "vad.hpp"

AudioCapture::AudioCapture(unsigned int sample_rate, std::string device)
    : sample_rate(sample_rate) {
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

void AudioCapture::start(std::function<void(std::vector<float>& audio, int start, int end)> handler) {
  int err;
  if ((err = snd_pcm_prepare(capture_handle)) < 0) {
    fprintf(stderr, "Can't prepare audio interface for use (%s)\n", snd_strerror(err));
    exit(1);
  }

  VoiceActivityDetector vad(sample_rate);
  unsigned int sample_count = vad.window_size_samples;

  char buffer[snd_pcm_format_width(format) / 8 * channels * sample_count];
  std::vector<float> audio;
  for ( ; ; ) {
    if ((err = snd_pcm_readi(capture_handle, buffer, sample_count)) != sample_count) {
      logger::error("Read from audio interface failed: %s", snd_strerror(err));
      exit(1);
    }

    std::vector<float> input;
    input.assign((float*)buffer, (float*)buffer + sizeof(buffer) / 4);
    audio.insert(audio.end(), input.begin(), input.end());
    bool triggered = vad.triggered;
    vad.predict(input);
    if (triggered != vad.triggered) {
      if (vad.triggered) {
        logger::info("Speech started");
      } else {
        logger::info("Speech finished");
        for (auto& speech : vad.speeches) {
          logger::info("Adding speech %d-%d to queue", speech.start, speech.end);
          // Bad results with short audio, pad until at least 24000 samples if available
          if (speech.end - speech.start < 24000) {
            speech.start = speech.end - 24000;
            if (speech.start < 0) {
              speech.start = 0;
              speech.end = 24000;
              if (speech.end > audio.size()) {
                speech.end = audio.size();
              }
            }
          }
          handler(audio, speech.start, speech.end);
        }
        vad.reset();
        audio.clear();
      }
    }
  }
}
