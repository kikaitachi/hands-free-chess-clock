#ifndef UCI_H_
#define UCI_H_

#include "chess_engine.hpp"
#include "process.hpp"
#include <functional>
#include <memory>
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
  void best_move(chess::Position& position);

  virtual std::optional<double> score();

 protected:
  Process process;
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
  virtual std::optional<double> score() override;

 protected:
  virtual void process_line(std::string line) override;
};

std::unique_ptr<UniversalChessInterface> create_uci(
  std::string command,
  std::function<void(const std::string best_move)> on_best_move);

#endif  // UCI_H_
