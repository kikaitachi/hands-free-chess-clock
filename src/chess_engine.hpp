#ifndef CHESS_ENGINE_H_
#define CHESS_ENGINE_H_

#include <forward_list>
#include <list>
#include <string>

namespace chess {

std::string index2string(int index);

enum Figure {
  Empty,
  King,
  Queen,
  Rook,
  Knight,
  Bishop,
  Pawn,
};

enum class Winner {
  None,
  Draw,
  White,
  Black
};

class GameResult {
 public:
  Winner winner;
  std::string message;
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

  /**
   * Set position to initial state.
   */
  void reset();

  friend bool operator==(const Position& lhs, const Position& rhs) = default;
  std::forward_list<Move> generate_legal_moves();

  GameResult move(const Move& move);

  Figure pieces[64];
  bool color[64];
  bool moved[64];
  bool white_turn;
  int passing_pawn;
  int move_number;

 private:
  /**
   * Does include moves prohibited by check.
   */
  std::forward_list<Move> generate_possible_moves(bool white_turn);

  bool is_king_attacked();

  std::list<Position> prev_positions;
};

}

#endif  // CHESS_ENGINE_H_
