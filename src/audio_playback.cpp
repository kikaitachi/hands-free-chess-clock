#include "audio_playback.hpp"
#include "logger.hpp"

char* replace_char(char* str, char find, char replace) {
  char *current_pos = strchr(str,find);
  while (current_pos) {
    *current_pos = replace;
    current_pos = strchr(current_pos,find);
  }
  return str;
}

AudioPlayback::AudioPlayback(unsigned int sample_rate, std::string device)
    : sample_rate(sample_rate), device(device) {
  void** hints;
  snd_device_name_hint(-1, "pcm", &hints);
  for (int i = 0; ; i++) {
    if (hints[i] == nullptr) {
      break;
    }
    char* name = snd_device_name_get_hint(hints[i], "NAME");
    char* desc = snd_device_name_get_hint(hints[i], "DESC");
    char* ioid = snd_device_name_get_hint(hints[i], "IOID");
    if (ioid != nullptr) {
      logger::debug("%s sound device: %s - %s", ioid, name, replace_char(desc, '\n', ' '));
      free(ioid);
    }
    free(name);
    free(desc);
  }
  snd_device_name_free_hint(hints);
}

void AudioPlayback::open() {
  int err;
  if ((err = snd_pcm_open(&handle, device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    logger::error("Can't open playback device '%s': %s",
      device.c_str(), snd_strerror(err));
    exit(1);
  }
  if ((err = snd_pcm_set_params(handle,
      SND_PCM_FORMAT_S16_LE,
      SND_PCM_ACCESS_RW_INTERLEAVED,
      1,
      sample_rate,
      1,
      500000)) < 0) {   /* 0.5sec */
    logger::error("Can't configure playback device '%s': %s",
      device.c_str(), snd_strerror(err));
    exit(1);
  }
}

void AudioPlayback::close() {
  int err = snd_pcm_drain(handle);
  if (err < 0) {
    logger::error("snd_pcm_drain failed: %s", snd_strerror(err));
  }
  snd_pcm_close(handle);
}

void AudioPlayback::play(char *buffer, int count) {
  snd_pcm_sframes_t frames = snd_pcm_writei(handle, buffer, count / 2);
  if (frames < 0) {
    frames = snd_pcm_recover(handle, frames, 0);
  }
  if (frames < 0) {
    logger::error("snd_pcm_writei failed: %s", snd_strerror(frames));
  }
}
