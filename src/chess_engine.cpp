#include "chess_engine.hpp"

std::string chess::coords2string(int row, int col) {
  return std::string(1, 'a' + col) + std::to_string(row + 1);
}

using namespace chess;

static std::string index2notation(int index) {
  return coords2string(index / 8, index % 8);
}

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
  return index2notation(from) + index2notation(to) + figure2notation(promoted);
}

Position::Position() {
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

std::forward_list<Move> Position::generate_possible_moves() {
  std::forward_list<Move> moves;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      int from = y * 8 + x;
      Figure piece = pieces[from];
      if (color[from] != white_turn) {
        continue;
      }
      switch (piece) {
        Pawn: {
          int dir = color[from] ? -1 : 1;  // Direction of pawn move depending on color
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
          break;
        }
      }
    }
  }
  return moves;
}
