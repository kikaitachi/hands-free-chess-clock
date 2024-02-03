#ifndef CHESS_ENGINE_H_
#define CHESS_ENGINE_H_

namespace chess {

enum Figure {
  King,
  Queen,
  Rook,
  Knight,
  Bishop,
  Pawn,
};

class Position {
 public:
  Position();

  int figures[64];
  bool color[64];
  bool moved[64];
};

}

#endif  // CHESS_ENGINE_H_
