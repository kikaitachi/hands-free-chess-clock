#include "text_to_speech.hpp"
#include <forward_list>
#include <fstream>
#include <thread>

TextToSpeech::TextToSpeech(std::string device)
    : audio_playback(sample_rate, device) {
}

void TextToSpeech::say(std::string text) {
  std::thread play_thread(&TextToSpeech::play, this, text);
  play_thread.detach();
}

void TextToSpeech::play(std::string text) {
  std::forward_list<std::string> files;
  if (text.length() == 4) {
    files.push_front(text.substr(0, 2));
    files.push_front(text.substr(2, 4));
  } else {
    files.push_front(text);
  }
  for (auto& file_name : files) {
    std::ifstream file("audio/" + file_name + ".wav", std::ios::binary);
    char buffer[sample_rate];
    file.read(buffer, 44);  // Skip header
    int count;
    do {
      file.read(buffer, sizeof(buffer));
      count = file.gcount();
      audio_playback.play(buffer, count);
    } while (count > 0);
  }
}
