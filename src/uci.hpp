#ifndef UCI_H_
#define UCI_H_

#include "chess_engine.hpp"
#include <string>

class UniversalChessInterface {
 public:
  UniversalChessInterface(std::string executable);
  std::string best_move(chess::Position& position);

 private:
  std::string executable;
};

#endif  // UCI_H_
