#include "game.hpp"

void Game::reset(unsigned int time_ms, unsigned int increment_ms) {
  time_white_ms = time_black_ms = time_ms;
  this->increment_ms = increment_ms;
  playing = true;
  last_clock_change = std::chrono::steady_clock::now();
  std::string time = format_time(time_ms);
  display.set_white(time);
  display.set_black(time);
}

std::string Game::format_time(unsigned int time_ms) {
  if (time_ms / 1000 / 60 > 99) {
    // Show only minutes
    std::string result = std::to_string(time_ms / 1000 / 60);
    result.insert(result.begin(), 4 - result.size(), ' ');
    return result;
  }
  std::string minutes = std::to_string(time_ms / 1000 / 60);
  minutes.insert(minutes.begin(), 2 - minutes.size(), ' ');
  std::string seconds = std::to_string(time_ms / 1000 % 60);
  seconds.insert(seconds.begin(), 2 - seconds.size(), '0');
  return minutes + ":" + seconds;
}
