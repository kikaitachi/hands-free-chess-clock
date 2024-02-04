#ifndef CHESS_ENGINE_H_
#define CHESS_ENGINE_H_

#include <forward_list>
#include <string>

namespace chess {

enum Figure {
  Empty,
  King,
  Queen,
  Rook,
  Knight,
  Bishop,
  Pawn,
};

class Move {
 public:
  int from;
  int to;
  Figure promoted;

  std::string to_string();
};

class Position {
 public:
  Position();
  friend bool operator==(const Position& lhs, const Position& rhs) = default;

  Figure pieces[64];
  bool color[64];
  bool moved[64];
  bool white_turn;
  int passing_pawn;

 private:
  /**
   * Does include moves prohibited by check.
   */
  std::forward_list<Move> generate_possible_moves();
};

}

#endif  // CHESS_ENGINE_H_
