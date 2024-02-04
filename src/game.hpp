#ifndef GAME_H_
#define GAME_H_

#include "chess_engine.hpp"
#include "display.hpp"
#include "video_capture.hpp"
#include <chrono>
#include <functional>

class Game {
 public:
  bool playing = false;

  void ready();
  void start(
    unsigned int time_ms, unsigned int increment_ms,
    std::function<void()> on_game_over
  );
  bool consider_move(SquareChange changes[64]);
  void stop();
  void resume(std::function<void()> on_game_over);
  void switch_clock();

 private:
  unsigned int time_white_ms;
  unsigned int time_black_ms;
  unsigned int increment_ms;
  bool white_turn;
  chess::Position position;
  Display display;
  std::chrono::steady_clock::time_point last_clock_change;

  std::string format_time(unsigned int time_ms);
  void update_clock(std::function<void()> on_game_over);
  void stop_blinking();
};

#endif  // GAME_H_
