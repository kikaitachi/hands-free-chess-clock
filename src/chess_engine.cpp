#include "chess_engine.hpp"
#include <cmath>
#include <forward_list>
#include <map>

std::string chess::index2string(int index) {
  int row = index / 8;
  int col = index % 8;
  return std::string(1, 'a' + col) + std::to_string(row + 1);
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
    { 1,  0, 8}, { 0,  1, 8}, { 0, -1, 8}, {-1,  0, 8},
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

std::string Move::to_string() const {
  return index2string(from) + index2string(to) + figure2notation(promoted);
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
}

std::list<Move> Position::generate_legal_moves() {
  std::list<Move> legal_moves;
  for (auto & move : generate_possible_moves(white_turn)) {
    Position position(*this);
    position.make_move(move);
    bool allowed = !position.is_king_attacked(white_turn);
    // Disallow castling when king is under check or goes through checked cell
    if (pieces[move.to] == King && abs(move.from - move.to) == 2) {
      if (is_king_attacked(white_turn)) {
        allowed = false;
      } else {
        position = Position(*this);
        position.move({move.from, (move.from + move.to) / 2, Empty});
        if (position.is_king_attacked(white_turn))
          allowed = false;
      }
    }
    if (allowed) {
      // TODO: generate all possible promotions
      legal_moves.push_front(move);
    }
  }
  return legal_moves;
}

std::list<Move> Position::generate_possible_moves(bool white_turn) {
  std::list<Move> moves;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      int from = y * 8 + x;
      Figure piece = pieces[from];
      if (color[from] != white_turn) {
        continue;
      }
      if (piece == Pawn) {
        int dir = color[from] ? 1 : -1;  // Direction of pawn move depending on color
        int to = from + dir * 8;  // One square ahead
        // Stroke to the right
        if (x < 7 && (pieces[to + 1] != Empty && color[to + 1] != white_turn || from + 1 == passing_pawn)) {
          moves.emplace_front(from, to + 1, Empty);
        }
        // Stroke to the left
        if (x > 0 && (pieces[to - 1] != Empty && color[to - 1] != white_turn || from - 1 == passing_pawn)) {
          moves.emplace_front(from, to - 1, Empty);
        }
        // Move one square forward
        if (pieces[to] == Empty) {
          moves.emplace_front(from, to, Empty);
        }
        // Move two squares forward
        if (!moved[from] && pieces[to] == Empty && pieces[to + dir * 8] == Empty) {
          moves.emplace_front(from, to + dir * 8, Empty);
        }
      } else {
        if (piece == King && !moved[from]) {
          // Short castling
          if (pieces[from + 1] == Empty && pieces[from + 2] == Empty && !moved[from + 3]) {
            moves.emplace_front(from, from + 2, Empty);
          }
          // Long castling
          if (pieces[from - 1] == Empty && pieces[from - 2] == Empty && pieces[from - 3] == Empty && !moved[from - 4]) {
            moves.emplace_front(from, from - 2, Empty);
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
                moves.emplace_front(from, to, Empty);
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
  return moves;
}

bool Position::is_king_attacked(bool white_turn) {
  for (auto & move : generate_possible_moves(!white_turn)) {
    if (pieces[move.to] == King && color[move.to] == white_turn) {
      return true;
    }
  }
  return false;
}

void Position::make_move(const Move& move) {
  Figure piece = pieces[move.from];
  if (piece == Pawn || pieces[move.to] != Empty) {
    prev_positions.clear();
  } else {
    prev_positions.push_front(*this);
  }
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
  make_move(move);
  // TODO: check for insufficient material to win
  // Checkmate or stalemate?
  if (generate_legal_moves().empty()) {
    if (is_king_attacked(white_turn)) {
      return {white_turn ? Winner::Black : Winner::White, "checkmate"};
    }
    return {Winner::Draw, "stalemate"};
  }
  // Draw by repetition?
  for (auto & prev_position : prev_positions) {
    if (prev_position == *this) {
      return {Winner::Draw, "repetition"};
    }
  }
  // 50 moves rule
  if (prev_positions.size() == 100) {
    return {Winner::Draw, "50 move rule"};
  }
  return {Winner::None, move.to_string().substr(0, 4)};
}
