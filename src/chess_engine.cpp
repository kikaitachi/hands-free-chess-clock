#include "chess_engine.hpp"

using namespace chess;

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
}
