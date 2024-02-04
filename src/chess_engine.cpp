#include "chess_engine.hpp"

using namespace chess;

static std::string index2notation(int index) {
  int row = index / 8;
  int col = index % 8;
  return std::to_string('a' + col) + std::to_string(row + 1);
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
  figure[0] = figure[7] = figure[56] = figure[63] = Rook;
  figure[1] = figure[6] = figure[57] = figure[62] = Knight;
  figure[2] = figure[5] = figure[58] = figure[61] = Bishop;
  figure[3] = figure[59] = Queen;
  figure[4] = figure[60] = King;
  for (int i = 0; i < 8; i++) {
    figure[8 + i] = figure[64 - 16 + i] = Pawn;
    figure[16 + i] = figure[24 + i] = figure[64 - 24 + i] = figure[64 - 32 + i] = Empty;
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
  // TODO: implement
  return moves;
}
