#include "chess_engine.hpp"
#include "game.hpp"
#include "logger.hpp"
#include <chrono>
#include <map>
#include <thread>

using namespace std::chrono_literals;

Game::Game(std::string device)
    : text_to_speech(24000, device), video_capture(
      [&]() {
        logger::info("Move started");
      },
      [&](SquareChange changes[64]) {
        logger::info("Move finished");
        return consider_move(changes);
      }
    ) {
}

void Game::stop_blinking() {
  display.blink_white(BLINK_RATE_NOBLINK);
  display.blink_black(BLINK_RATE_NOBLINK);
}

void Game::ready() {
  display.set_white("ALL");
  display.set_black("SET");
  stop_blinking();
}

void Game::start(unsigned int time_ms, unsigned int increment_ms) {
  time_white_ms = time_black_ms = time_ms;
  this->increment_ms = increment_ms;
  white_turn = true;
  position.reset();
  std::string time = format_time(time_ms);
  stop_blinking();
  display.set_white(time);
  display.set_black(time);

  std::thread clock_update_thread(&Game::update_clock, this);
  clock_update_thread.detach();

  text_to_speech.say("game on");
  video_capture.start_game();
}

std::string Game::consider_move(SquareChange changes[64]) {
  logger::info("6 best candidate squares: %s, %s, %s, %s, %s, %s",
    chess::index2string(changes[0].index).c_str(),
    chess::index2string(changes[1].index).c_str(),
    chess::index2string(changes[2].index).c_str(),
    chess::index2string(changes[3].index).c_str(),
    chess::index2string(changes[4].index).c_str(),
    chess::index2string(changes[5].index).c_str());
  std::list<chess::Move> moves = position.generate_legal_moves();
  std::map<int, chess::Move> candidates;
  for (auto & move : moves) {
    int from = -1;
    int to = -1;
    int score = 0;
    for (int i = 0; i < 6; i++) {
      if (move.from == changes[i].index) {
        from = move.from;
        score += (5 - i) * 2;
      }
      if (move.to == changes[i].index) {
        to = move.to;
        score += (5 - i) * 2;
      }
    }
    bool candidate = from != -1 && to != -1;
    if (candidate) {
      score += 20;
    }
    bool dest_shadow = false;
    if (to != -1) {
      int x = to % 8;
      if (x == 7) {
        score += 1;
      } else {
        int shadow = to + 1;
        for (int i = 0; i < 6; i++) {
          if (changes[i].index == shadow) {
            dest_shadow = true;
            break;
          }
        }
      }
    }
    if (dest_shadow) {
      score += 5;
    } else {
      score -= 5;
    }
    bool src_shadow = false;
    if (from != -1) {
      int x = from % 8;
      if (x == 7) {
        score += 1;
      } else {
        int shadow = from + 1;
        for (int i = 0; i < 6; i++) {
          if (changes[i].index == shadow) {
            src_shadow = true;
            break;
          }
        }
      }
    }
    if (src_shadow) {
      score += 5;
    } else {
      score -= 5;
    }
    logger::info("Move: %s %d %s", move.to_string().c_str(),
      score, candidate ? " candidate" : "");
    if (candidate) {
      candidates.insert({score, move});
    }
  }
  if (candidates.size() > 0) {
    chess::Move most_likely_move = candidates.rbegin()->second;
    chess::GameResult result = position.move(most_likely_move);
    text_to_speech.say(result.message);
    if (result.winner != chess::Winner::None) {
      stop();
    } else {
      switch_clock();
    }
    return most_likely_move.to_string();
  }
  return "";
}

void Game::on_game_over() {
  playing = false;
  video_capture.stop_game();
}

void Game::stop() {
  playing = false;
  text_to_speech.say("stopped");
}

void Game::resume() {
  text_to_speech.say("resumed");
  std::thread clock_update_thread(&Game::update_clock, this);
  clock_update_thread.detach();
  video_capture.resume_game();
}

void Game::switch_clock() {
  if (white_turn) {
    time_white_ms += increment_ms;
    display.set_white(format_time(time_white_ms));
  } else {
    time_black_ms += increment_ms;
    display.set_black(format_time(time_black_ms));
  }
  white_turn = !white_turn;
}

void Game::update_clock() {
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
        display.blink_white(BLINK_RATE_1HZ);
        text_to_speech.say("time is up");
        on_game_over();
      }
      display.set_white(format_time(time_white_ms));
    } else {
      if (time_black_ms > millis) {
        time_black_ms -= millis;
      } else {
        time_black_ms = 0;
        display.blink_black(BLINK_RATE_1HZ);
        text_to_speech.say("time is up");
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

void Game::shutdown() {
  stop_blinking();
  display.set_white("TURN");
  display.set_black("OFF");
}
