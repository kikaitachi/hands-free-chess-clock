#include "chess_engine.hpp"
#include <algorithm>
#include <cmath>
#include <forward_list>
#include <map>
#include <stdexcept>

std::string chess::index2string(int index) {
  int row = index / 8;
  int col = index % 8;
  return std::string(1, 'a' + col) + std::to_string(row + 1);
}

int chess::string2index(std::string cell) {
  return (cell[1] - (int)'1') * 8 + (cell[0] - (int)'a');
}

using namespace chess;

class PieceMove {
 public:
  int x;
  int y;
  int distance;
};

static std::map<Figure, std::forward_list<PieceMove>> lookup_table({
  {Rook, {
    { 1,  0, 7}, { 0,  1, 7}, { 0, -1, 7}, {-1,  0, 7},
  }},
  {Knight, {
    { 1,  2, 1}, { 1, -2, 1}, { 2,  1, 1}, { 2, -1, 1},
		{-1,  2, 1}, {-1, -2, 1}, {-2,  1, 1}, {-2, -1, 1},
  }},
  {Bishop, {
    { 1,  1, 7}, { 1, -1, 7}, {-1, -1, 7}, {-1,  1, 7},
  }},
  {Queen, {
    {-1, -1, 7}, {-1,  0, 7}, {-1,  1, 7}, { 0,  1, 7},
    { 1,  1, 7}, { 1,  0, 7}, { 1, -1, 7}, { 0, -1, 7},
  }},
  {King, {
    {-1, -1, 1}, {-1,  0, 1}, {-1,  1, 1}, { 0,  1, 1},
    { 1,  1, 1}, { 1,  0, 1}, { 1, -1, 1}, { 0, -1, 1},
  }},
});

static std::string figure2notation(Figure figure) {
  switch (figure) {
    case King:
      return "K";
    case Queen:
      return "Q";
    case Rook:
      return "R";
    case Knight:
      return "N";
    case Bishop:
      return "B";
    default:
      return "";
  }
}

Move::Move(int from, int to, Figure promoted)
    : from(from), to(to), promoted(promoted) {
}

std::string Move::to_string() const {
  return index2string(from) + index2string(to) + figure2notation(promoted);
}

bool Move::operator==(const Move &right) const {
  if (to != right.to) {
    return false;
  }
  if (from != right.from) {
    return false;
  }
  if (promoted != right.promoted) {
    return false;
  }
  return true;
}

bool EvaluatedMove::operator<(EvaluatedMove const &other) const {
  const Score& s1 = this->score;
  const Score& s2 = other.score;
  if (s1.unit == chess::ScoreUnit::MateIn && s2.unit != chess::ScoreUnit::MateIn) {
    return false;
  }
  if (s1.unit != chess::ScoreUnit::MateIn && s2.unit == chess::ScoreUnit::MateIn) {
    return true;
  }
  if (s1.unit == chess::ScoreUnit::MateIn && s2.unit == chess::ScoreUnit::MateIn) {
    return s1.value > s2.value;
  }
  return s1.value < s2.value;
}

Position::Position() {
  reset();
}

void Position::reset() {
  pieces[0] = pieces[7] = pieces[56] = pieces[63] = Rook;
  pieces[1] = pieces[6] = pieces[57] = pieces[62] = Knight;
  pieces[2] = pieces[5] = pieces[58] = pieces[61] = Bishop;
  pieces[3] = pieces[59] = Queen;
  pieces[4] = pieces[60] = King;
  for (int i = 0; i < 8; i++) {
    pieces[8 + i] = pieces[64 - 16 + i] = Pawn;
    pieces[16 + i] = pieces[24 + i] = pieces[64 - 24 + i] = pieces[64 - 32 + i] = Empty;
  }
  for (int i = 0; i < 64; i++) {
    color[i] = i < 32;
    moved[i] = false;
  }
  white_turn = true;
  passing_pawn = 0;
  move_number = 1;
  prev_positions.clear();
  prev_positions.push_front(*this);
  moves.clear();
}

std::vector<Move> Position::generate_legal_moves() {
  std::vector<Move> legal_moves;
  generate_possible_moves(white_turn, [&](Move move) {
    Position position(*this);
    position.make_move(move);
    if (position.is_king_attacked(white_turn)) {
      return false;
    }
    // Disallow castling when king is under check or goes through checked cell
    if (pieces[move.to] == King && abs(move.from - move.to) == 2) {
      if (is_king_attacked(white_turn)) {
        return false;
      }
      position = Position(*this);
      position.make_move(Move(move.from, (move.from + move.to) / 2, Empty));
      if (position.is_king_attacked(white_turn)) {
        return false;
      }
    }
    if (pieces[move.from] == Pawn && (move.to < 8 || move.to >= 8 * (8 - 1))) {
      legal_moves.emplace_back(move.from, move.to, Queen);
      legal_moves.emplace_back(move.from, move.to, Rook);
      legal_moves.emplace_back(move.from, move.to, Bishop);
      legal_moves.emplace_back(move.from, move.to, Knight);
    } else {
      legal_moves.emplace_back(move);
    }
    return false;
  });
  return legal_moves;
}

bool Position::generate_possible_moves(bool white_turn,
    std::function<bool(Move move)> on_move) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      int from = y * 8 + x;
      Figure piece = pieces[from];
      if (piece == Empty || color[from] != white_turn) {
        continue;
      }
      if (piece == Pawn) {
        int dir = color[from] ? 1 : -1;  // Direction of pawn move depending on color
        int to = from + dir * 8;  // One square ahead
        // Stroke to the right
        if (x < 7 && (pieces[to + 1] != Empty && color[to + 1] != white_turn || from + 1 == passing_pawn)) {
          if (on_move({from, to + 1, Empty})) {
            return true;
          }
        }
        // Stroke to the left
        if (x > 0 && (pieces[to - 1] != Empty && color[to - 1] != white_turn || from - 1 == passing_pawn)) {
          if (on_move({from, to - 1, Empty})) {
            return true;
          }
        }
        // Move one square forward
        if (pieces[to] == Empty) {
          if (on_move({from, to, Empty})) {
            return true;
          }
        }
        // Move two squares forward
        if (!moved[from] && pieces[to] == Empty && pieces[to + dir * 8] == Empty) {
          if (on_move({from, to + dir * 8, Empty})) {
            return true;
          }
        }
      } else {
        if (piece == King && !moved[from]) {
          // Short castling
          if (pieces[from + 1] == Empty && pieces[from + 2] == Empty && !moved[from + 3]) {
            if (on_move({from, from + 2, Empty})) {
              return true;
            }
          }
          // Long castling
          if (pieces[from - 1] == Empty && pieces[from - 2] == Empty && pieces[from - 3] == Empty && !moved[from - 4]) {
            if (on_move({from, from - 2, Empty})) {
              return true;
            }
          }
        }
        auto iterator = lookup_table.find(piece);
        if (iterator != lookup_table.end()) {
          for (auto & piece_move : iterator->second) {
            for (int i = 1; i <= piece_move.distance; i++) {
              int to_x = x + piece_move.x * i;
              if (to_x < 0 || to_x > 7) {
                break;
              }
              int to_y = y + piece_move.y * i;
              if (to_y < 0 || to_y > 7) {
                break;
              }
              int to = to_y * 8 + to_x;
              if (pieces[to] == Empty || color[to] != white_turn) {
                if (on_move({from, to, Empty})) {
                  return true;
                }
              }
              if (pieces[to] != Empty) {
                // Path is not clear for longer distance moves
                break;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

bool Position::is_king_attacked(bool white_turn) {
  return generate_possible_moves(!white_turn, [&](Move move) {
    return pieces[move.to] == King && color[move.to] == white_turn;
  });
}

void Position::make_move(const Move& move) {
  Figure piece = pieces[move.from];
  passing_pawn = 0;
  if (piece == King) {
    if (move.from - move.to == 2) {
      // Queen side castling
      pieces[move.to + 1] = Rook;
      int rook_coord = move.from < 8 ? 0 : 8 * (8 - 1);
      pieces[rook_coord] = Empty;
      color[move.to + 1] = color[rook_coord];
    } else if (move.to - move.from == 2) {
      // King side castling
      pieces[move.to - 1] = Rook;
      int rook_coord = (move.from < 8 ? 8 : 64) - 1;
      pieces[rook_coord] = Empty;
      color[move.to - 1] = color[rook_coord];
    }
  } else if (piece == Pawn) {
    // Set passing pawn coordinates
    if (abs(move.from - move.to) == 8 * 2)
      passing_pawn = move.to;
    // Passing pawn stroke
    else if (abs(move.from - move.to) != 8 && pieces[move.to] == Empty)
      pieces[move.to + (move.from - move.to > 0 ? 8 : -8)] = Empty;
    // Promotion
    else if (move.to < 8 || move.to >= 8 * (8 - 1))
      piece = move.promoted;
  }
  // Make move
  pieces[move.to] = piece;
  pieces[move.from] = Empty;
  color[move.to] = color[move.from];
  moved[move.from] = moved[move.to] = true;
  // Switch turn
  white_turn = !white_turn;
  if (white_turn) {
    move_number++;
  }
}

GameResult Position::move(const Move& move) {
  moves.push_back(move);
  if (pieces[move.from] == Pawn || pieces[move.to] != Empty) {
    prev_positions.clear();
  } else {
    prev_positions.push_front(*this);
  }
  make_move(move);
  int piece_count = 0;
  for (auto& piece : pieces) {
    if (piece != Figure::Empty) {
      piece_count++;
    }
  }
  if (piece_count == 2) {
    return {Winner::Draw, "insufficient material"};
  }
  // Checkmate or stalemate?
  if (generate_legal_moves().empty()) {
    if (is_king_attacked(white_turn)) {
      return {white_turn ? Winner::Black : Winner::White, "checkmate"};
    }
    return {Winner::Draw, "stalemate"};
  }
  // Draw by repetition?
  int count = 0;
  for (auto & prev_position : prev_positions) {
    if (prev_position.equal(*this)) {
      count++;
    }
  }
  if (count == 3) {
    return {Winner::Draw, "repetition"};
  }
  // 50 moves rule
  if (prev_positions.size() == 100) {
    return {Winner::Draw, "50 move rule"};
  }
  return {Winner::None, move.to_string().substr(0, 4)};
}

Move uci2move(std::string move) {
  return {
    string2index(move.substr(0, 2)),
    string2index(move.substr(2, 4)),
    chess::Figure::Empty
  };
}

GameResult Position::move(std::string uci_notation) {
  return move(uci2move(uci_notation));
}

std::string Position::move_san(const std::string san) {
  if (san == "O-O") {
    std::string uci_notation = white_turn ? "e1g1" : "e8g8";
    make_move(uci2move(uci_notation));
    return uci_notation;
  }
  if (san == "O-O-O") {
    std::string uci_notation = white_turn ? "e1c1" : "e8c8";
    make_move(uci2move(uci_notation));
    return uci_notation;
  }
  std::string san_notation = san;
  san_notation.erase(std::remove(san_notation.begin(), san_notation.end(), '#'), san_notation.end());
  san_notation.erase(std::remove(san_notation.begin(), san_notation.end(), '='), san_notation.end());
  san_notation.erase(std::remove(san_notation.begin(), san_notation.end(), '+'), san_notation.end());
  san_notation.erase(std::remove(san_notation.begin(), san_notation.end(), 'x'), san_notation.end());
  std::vector<Move> moves = generate_legal_moves();
  for (const auto& m : moves) {
    std::string uci_notation = m.to_string();
    std::string to = san_notation.substr(san_notation.size() - 2);
    if (!uci_notation.ends_with(to)) {
      continue;
    }
    std::string figure = figure2notation(pieces[m.from]);
    if (figure.empty()) {
      if (!std::islower(san_notation.at(0))) {
        continue;
      }
    } else if (!san_notation.starts_with(figure)) {
      continue;
    }
    std::string from = san_notation
      .substr(0, san_notation.size() - 2)
      .substr(figure.size());
    if (from.size() == 2 && !uci_notation.starts_with(from)) {
      continue;
    }
    if (from.size() == 1) {
      if (std::isdigit(from.at(0))) {
        if (uci_notation.at(1) != from.at(0)) {
          continue;
        }
      } else {
        if (uci_notation.at(0) != from.at(0)) {
          continue;
        }
      }
    }
    make_move(m);
    return uci_notation;
  }
  throw std::runtime_error("Can't convert move: " + san);
}

bool Position::equal(const Position& other) {
  return pieces == other.pieces &&
    color == other.color;
}

std::optional<Position> Position::previous() {
  if (moves.empty()) {
    return std::nullopt;
  }
  Position position;
  for (auto& move : moves) {
    if (move != moves.back()) {
      position.move(move);
    }
  }
  return std::optional(position);
}
