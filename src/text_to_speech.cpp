#include "text_to_speech.hpp"
#include <forward_list>
#include <fstream>
#include <thread>

using namespace std::chrono_literals;

TextToSpeech::TextToSpeech(unsigned int sample_rate, std::string device)
    : sample_rate(sample_rate), device(device) {
  std::thread play_thread(&TextToSpeech::play, this);
  play_thread.detach();
}

void TextToSpeech::say(std::string text) {
  queue.push(new std::string(text));
}

void TextToSpeech::play() {
  AudioPlayback audio_playback(sample_rate, device);
  for ( ; ; ) {
    queue.drain([&](auto entry) {
      audio_playback.open();
      std::this_thread::sleep_for(250ms);
      std::forward_list<std::string> files;
      if (entry->length() == 4) {
        files.push_front(entry->substr(2, 4));
        files.push_front(entry->substr(0, 2));
      } else {
        files.push_front(*entry);
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
    });
    std::this_thread::sleep_for(10ms);
  }
}
