#include "chess_engine.hpp"

using namespace chess;

Position::Position() {
  for (int i = 0; i < 64; i++) {
    color[i] = i < 32;
    moved[i] = false;
  }
}
