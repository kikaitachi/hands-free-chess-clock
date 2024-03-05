#ifndef UCI_H_
#define UCI_H_

#include "chess_engine.hpp"
#include "process.hpp"
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>

class UniversalChessInterface {
 public:
  UniversalChessInterface(
    char const *argv[],
    std::function<void(const std::string best_move)> on_best_move
  );

  /**
   * Request to calculate the best move.
   * Result will be returned via callback provided in constructor.
   */
  void best_move(const chess::Position& position);

  virtual std::optional<double> get_score(const chess::Position& position);

 protected:
  Process process;
  void send_position(const chess::Position& position);
  virtual void process_line(std::string line);

 private:
  std::function<void(const std::string best_move)> on_best_move;

  void read();
};

class Stockfish: public UniversalChessInterface {
 public:
  Stockfish(
    char const *argv[],
    std::function<void(const std::string best_move)> on_best_move
  );
  virtual std::optional<double> get_score(const chess::Position& position) override;

 protected:
  virtual void process_line(std::string line) override;

 private:
  std::mutex score_mutex;
  std::condition_variable score_found;
  std::optional<double> score;
};

std::unique_ptr<UniversalChessInterface> create_uci(
  std::string command,
  std::function<void(const std::string best_move)> on_best_move);

#endif  // UCI_H_
