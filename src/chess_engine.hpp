#ifndef CHESS_ENGINE_H_
#define CHESS_ENGINE_H_

#include <array>
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

  std::string to_string() const;
};

class Position {
 public:
  Position();

  /**
   * Set position to initial state.
   */
  void reset();

  std::list<Move> generate_legal_moves();

  GameResult move(const Move& move);

  bool equal(const Position& other);

  std::array<Figure, 64> pieces;
  std::array<bool, 64> color;
  std::array<bool, 64> moved;
  bool white_turn;
  int passing_pawn;
  int move_number;

 private:
  /**
   * Does include moves prohibited by check.
   */
  std::list<Move> generate_possible_moves(bool white_turn);

  bool is_king_attacked(bool white_turn);

  std::list<Position> prev_positions;

  void make_move(const Move& move);
};

}

#endif  // CHESS_ENGINE_H_
