#ifndef UCI_H_
#define UCI_H_

#include "chess_engine.hpp"
#include "process.hpp"
#include <functional>
#include <optional>

class UniversalChessInterface {
 public:
  UniversalChessInterface(
    Process& process,
    std::function<void(const std::string best_move)> on_best_move
  );

  /**
   * Request to calculate the best move.
   * Result will be returned via callback provided in constructor.
   */
  void best_move(chess::Position& position);

  std::optional<double> score();

 private:
  Process& process;
  std::function<void(const std::string best_move)> on_best_move;

  void read();
};

#endif  // UCI_H_
