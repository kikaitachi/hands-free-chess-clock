#ifndef UCI_H_
#define UCI_H_

#include "chess_engine.hpp"
#include "process.hpp"
#include <condition_variable>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

using namespace std::chrono_literals;

class UniversalChessInterface {
 public:
  UniversalChessInterface(std::string command);

  std::vector<chess::EvaluatedMove> evaluate_moves(
    const chess::Position& position,
    const std::vector<chess::Move>& moves,
    const int depth = 10,
    const std::chrono::milliseconds timeout = 3s);

 protected:
  Process process;
  std::mutex score_mutex;
  std::condition_variable score_found;
  std::optional<chess::Score> score;
  void send_position(const chess::Position& position);
  virtual void process_line(std::string line);

 private:
  void read();
};

#endif  // UCI_H_
