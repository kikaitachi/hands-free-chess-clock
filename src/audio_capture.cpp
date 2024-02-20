#include "audio_capture.hpp"
#include "logger.hpp"
#include "vad.hpp"

AudioCapture::AudioCapture(unsigned int sample_rate, std::string device)
    : sample_rate(sample_rate) {
  int err;
  if ((err = snd_pcm_open(&capture_handle, device.c_str(), SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    logger::error("Can't open audio device %s (%s)", device.c_str(), snd_strerror(err));
    return;
  }
  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
    logger::error("Can't allocate hardware parameter structure (%s)", snd_strerror(err));
    return;
  }
  if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
    logger::error("Can't initialize hardware parameter structure (%s)", snd_strerror(err));
    return;
  }
  if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    logger::error("Can't set access type (%s)", snd_strerror(err));
    return;
  }
  if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format)) < 0) {
    logger::error("Can't set sample format (%s)", snd_strerror(err));
    return;
  }
  if ((err = snd_pcm_hw_params_set_rate(capture_handle, hw_params, sample_rate, 0)) < 0) {
    logger::error("Can't set sample rate (%s)", snd_strerror(err));
    return;
  }
  if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, channels)) < 0) {
    logger::error("Can't set channel count (%s)", snd_strerror(err));
    return;
  }
  if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
    logger::error("Can't set parameters (%s)", snd_strerror(err));
    return;
  }
  snd_pcm_hw_params_free(hw_params);
}

void AudioCapture::start(std::function<void(std::vector<float>& audio, int start, int end)> handler) {
  int err;
  if ((err = snd_pcm_prepare(capture_handle)) < 0) {
    logger::error("Can't prepare audio interface for use (%s)", snd_strerror(err));
    return;
  }

  VoiceActivityDetector vad(sample_rate);

  std::vector<float> audio;
  std::vector<float> input(vad.window_size_samples);
  for ( ; ; ) {
    if ((err = snd_pcm_readi(capture_handle, input.data(), input.size())) != input.size()) {
      logger::error("Read from audio interface failed: %s", snd_strerror(err));
      return;
    }

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
