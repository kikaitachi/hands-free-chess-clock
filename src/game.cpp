#include "chess_engine.hpp"
#include "game.hpp"
#include "logger.hpp"
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

void Game::stop_blinking() {
  display.blink_white(BLINK_RATE_NOBLINK);
  display.blink_black(BLINK_RATE_NOBLINK);
}

void Game::ready() {
  display.set_white("ALL");
  display.set_black("SET");
  stop_blinking();
}

void Game::start(
    unsigned int time_ms, unsigned int increment_ms,
    std::function<void()> on_game_over
  ) {
  time_white_ms = time_black_ms = time_ms;
  this->increment_ms = increment_ms;
  white_turn = true;
  std::string time = format_time(time_ms);
  stop_blinking();
  display.set_white(time);
  display.set_black(time);

  std::thread clock_update_thread(&Game::update_clock, this, on_game_over);
  clock_update_thread.detach();
}

bool Game::consider_move(SquareChange changes[64]) {
  logger::info("6 best candidate squares: %s, %s, %s, %s, %s, %s",
    chess::coords2string(changes[0].y, changes[0].x).c_str(),
    chess::coords2string(changes[1].y, changes[1].x).c_str(),
    chess::coords2string(changes[2].y, changes[2].x).c_str(),
    chess::coords2string(changes[3].y, changes[3].x).c_str(),
    chess::coords2string(changes[4].y, changes[4].x).c_str(),
    chess::coords2string(changes[5].y, changes[5].x).c_str());
  return true;
}

void Game::stop() {
  playing = false;
}

void Game::resume(std::function<void()> on_game_over) {
  std::thread clock_update_thread(&Game::update_clock, this, on_game_over);
  clock_update_thread.detach();
}

void Game::switch_clock() {
  white_turn = !white_turn;
}

void Game::update_clock(std::function<void()> on_game_over) {
  playing = true;
  last_clock_change = std::chrono::steady_clock::now();
  while (playing) {
    std::this_thread::sleep_for(50ms);
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    unsigned int millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_clock_change).count();
    if (white_turn) {
      if (time_white_ms > millis) {
        time_white_ms -= millis;
      } else {
        time_white_ms = 0;
        playing = false;
        display.blink_white(BLINK_RATE_1HZ);
        on_game_over();
      }
      display.set_white(format_time(time_white_ms));
    } else {
      if (time_black_ms > millis) {
        time_black_ms -= millis;
      } else {
        time_black_ms = 0;
        playing = false;
        display.blink_black(BLINK_RATE_1HZ);
        on_game_over();
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
  if (time_ms / 1000 > 9) {
    // Show minutes and seconds
    std::string minutes = std::to_string(time_ms / 1000 / 60);
    minutes.insert(minutes.begin(), 2 - minutes.size(), ' ');
    std::string seconds = std::to_string(time_ms / 1000 % 60);
    seconds.insert(seconds.begin(), 2 - seconds.size(), '0');
    return minutes + ":" + seconds;
  }
  // Show only seconds
  std::string seconds = std::to_string(time_ms / 1000 % 60);
  std::string deciseconds = std::to_string(time_ms / 100 % 10);
  return "  " + seconds + "." + deciseconds;
}
