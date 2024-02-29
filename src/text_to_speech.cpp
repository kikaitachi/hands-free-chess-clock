#include "logger.hpp"
#include "text_to_speech.hpp"
#include <thread>

using namespace std::chrono_literals;

TextToSpeech::TextToSpeech(unsigned int sample_rate, std::string device, Process& piper)
    : sample_rate(sample_rate), device(device), piper(piper) {
  std::thread play_thread(&TextToSpeech::play, this);
  play_thread.detach();
}

void TextToSpeech::say(std::string text) {
  if (write(piper.write_fd, (text + "\n").c_str(), text.size() + 1) == -1) {
    logger::last("Failed to write '%s' to piper", text.c_str());
  }
}

void TextToSpeech::play() {
  AudioPlayback audio_playback(sample_rate, device);
  audio_playback.open();
  for ( ; ; ) {
    char buffer[1024];
    ssize_t result = read(piper.read_fd, buffer, sizeof(buffer));
    if (result == -1) {
      logger::last("Failed to read from piper");
      return;
    }
    audio_playback.play(buffer, result);
  }
}
