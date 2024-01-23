#ifndef GAME_H_
#define GAME_H_

#include "display.hpp"
#include <chrono>

class Game {
 public:
  bool playing = false;

  void reset(unsigned int time_ms, unsigned int increment_ms);

 private:
  unsigned int time_white_ms;
  unsigned int time_black_ms;
  unsigned int increment_ms;
  Display display;
  std::chrono::steady_clock::time_point last_clock_change;

  std::string format_time(unsigned int time_ms);
};

#endif  // GAME_H_
