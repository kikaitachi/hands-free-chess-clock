#ifndef CHESS_ENGINE_H_
#define CHESS_ENGINE_H_

#include <array>
#include <list>
#include <optional>
#include <string>
#include <vector>

namespace chess {

std::string index2string(int index);

int string2index(std::string cell);

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

/**
 * Type of move score.
 */
enum class ScoreUnit {
  /**
   * Unit of 0.01 pawn value.
   */
  Centipawn,

  /**
   * Indicates that checkmate can be forced in given number of moves.
   */
  MateIn
};

/**
 * Score of the move.
 */
class Score {
 public:
  /**
   * Value of the move.
   */
  int value;

  /**
   * Unit of value.
   */
  ScoreUnit unit;

  /**
   * At which depth this score was calculated.
   */
  int depth;
};

class Move {
 public:
  Move(int from, int to, Figure promoted);

  int from;
  int to;
  Figure promoted;

  std::string to_string() const;

  bool operator==(const Move &right) const;
};

/**
 * Move which was evaluated by chess engine and has a score.
 */
class EvaluatedMove {
 public:
  Move move;
  Score score;

  bool operator<(EvaluatedMove const &other) const;
};

class Position {
 public:
  /**
   * Create a new position with all chess pieces their initial positions.
   */
  Position();

  /**
   * Set position to initial state.
   */
  void reset();

  /**
   * Return all legal moves in this position.
   */
  std::vector<Move> generate_legal_moves();

  /**
   * Make a legal move. Method doesn't validate move. If illegal move is
   * provided behaviour is undefined.
   */
  GameResult move(const Move& move);

  GameResult move(std::string uci_notation);

  /**
   * Make a move specified by Standard Algebraic Notation (SAN) and return
   * move in Universal Chess Interface (UCI) notation. This method will not
   * check for game ending conditions. It is intended just for converting
   * SAN to UCI notation.
   */
  std::string move_san(const std::string san);

  /**
   * Compare current position to the given one.
   */
  bool equal(const Position& other);

  /**
   * Return previous position or nullopt if position is initial.
   */
  std::optional<Position> previous();

  std::array<Figure, 64> pieces;
  std::array<bool, 64> color;
  std::array<bool, 64> moved;
  bool white_turn;
  int passing_pawn;
  int move_number;

  /**
   * All moves made leading to this position.
   */
  std::list<Move> moves;

 private:
  /**
   * Does include moves prohibited by check.
   */
  std::list<Move> generate_possible_moves(bool white_turn);

  bool is_king_attacked(bool white_turn);

  /**
   * Used only for determine game ending conditions doesn't contain all
   * previous positions.
   */
  std::list<Position> prev_positions;

  void make_move(const Move& move);
};

}

#endif  // CHESS_ENGINE_H_
