#ifndef GAME_H_
#define GAME_H_

#include "chess_engine.hpp"
#include "display.hpp"
#include "text_to_speech.hpp"
#include "video_capture.hpp"
#include <chrono>
#include <functional>

class Game {
 public:
  Game();
  bool playing = false;

  void ready();
  void start(unsigned int time_ms, unsigned int increment_ms);
  std::string consider_move(SquareChange changes[64]);
  void stop();
  void resume();

 private:
  unsigned int time_white_ms;
  unsigned int time_black_ms;
  unsigned int increment_ms;
  bool white_turn;
  chess::Position position;
  Display display;
  TextToSpeech text_to_speech;
  VideoCapture video_capture;
  std::chrono::steady_clock::time_point last_clock_change;

  std::string format_time(unsigned int time_ms);
  void switch_clock();
  void update_clock();
  void stop_blinking();
  void on_game_over();
};

#endif  // GAME_H_
