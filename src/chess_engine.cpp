#include "chess_engine.hpp"
#include <cmath>

std::string chess::index2string(int index) {
  int row = index / 8;
  int col = index % 8;
  return std::string(1, 'a' + col) + std::to_string(row + 1);
}

using namespace chess;

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

std::string Move::to_string() {
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
}

std::forward_list<Move> Position::generate_legal_moves() {
  std::forward_list<Move> possible_moves = generate_possible_moves(white_turn);
  // TODO: filter out moves impossible because of checks and add promotions
  return possible_moves;
}

std::forward_list<Move> Position::generate_possible_moves(bool white_turn) {
  std::forward_list<Move> moves;
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
      }
    }
  }
  return moves;
}

bool Position::is_king_attacked() {
  for (auto & move : generate_possible_moves(!white_turn)) {
    if (pieces[move.to] == King && color[move.to] == white_turn) {
      return true;
    }
  }
  return false;
}

void Position::move(const Move& move) {
  passing_pawn = 0;
  Figure piece = pieces[move.from];
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
}
