#include "game.hpp"
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

void Game::ready() {
  display.set_white("ALL");
  display.set_black("SET");
  display.blink_white(BLINK_RATE_NOBLINK);
  display.blink_black(BLINK_RATE_NOBLINK);
}

void Game::start(unsigned int time_ms, unsigned int increment_ms) {
  time_white_ms = time_black_ms = time_ms;
  this->increment_ms = increment_ms;
  white_turn = true;
  playing = true;
  last_clock_change = std::chrono::steady_clock::now();
  std::string time = format_time(time_ms);
  display.set_white(time);
  display.set_black(time);

  std::thread clock_update_thread(&Game::update_clock, this);
  clock_update_thread.detach();
}

void Game::stop() {
  playing = false;
}

void Game::switch_clock() {
  white_turn = !white_turn;
}

void Game::update_clock() {
  while (playing) {
    std::this_thread::sleep_for(100ms);
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    unsigned int millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_clock_change).count();
    if (white_turn) {
      if (time_white_ms > millis) {
        time_white_ms -= millis;
      } else {
        time_white_ms = 0;
        playing = false;
      }
      display.set_white(format_time(time_white_ms));
    } else {
      if (time_black_ms > millis) {
        time_black_ms -= millis;
      } else {
        time_black_ms = 0;
        playing = false;
      }
      display.set_black(format_time(time_black_ms));
    }
    last_clock_change = now;
  }
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
