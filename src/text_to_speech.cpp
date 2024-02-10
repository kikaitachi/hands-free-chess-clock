#include "text_to_speech.hpp"
#include <forward_list>
#include <fstream>
#include <thread>

using namespace std::chrono_literals;

TextToSpeech::TextToSpeech(std::string device)
    : audio_playback(sample_rate, device) {
}

void TextToSpeech::say(std::string text) {
  audio_playback.open();
  std::thread play_thread(&TextToSpeech::play, this, text);
  play_thread.detach();
}

void TextToSpeech::play(std::string text) {
  std::this_thread::sleep_for(250ms);
  std::forward_list<std::string> files;
  if (text.length() == 4) {
    files.push_front(text.substr(2, 4));
    files.push_front(text.substr(0, 2));
  } else {
    files.push_front(text);
  }
  char buffer[sample_rate];
  for (auto& file_name : files) {
    std::ifstream file("audio/" + file_name + ".wav", std::ios::binary);
    file.seekg(44);  // Skip header
    int count;
    do {
      file.read(buffer, sizeof(buffer));
      count = file.gcount();
      audio_playback.play(buffer, count);
    } while (count > 0);
  }
  audio_playback.close();
}
