#include <gtest/gtest.h>
#include "../src/chess_engine.hpp"

int cell2index(std::string cell) {
  return (cell[1] - (int)'1') * 8 + (cell[0] - (int)'a');
}

chess::Move move(std::string move) {
  return {
    cell2index(move.substr(0, 2)),
    cell2index(move.substr(2, 4)),
    chess::Figure::Empty
  };
}

TEST(PositionTest, InitialPosition) {
  chess::Position position;
  EXPECT_TRUE(position.white_turn);
  EXPECT_EQ(position.generate_legal_moves().size(), 20);
}

TEST(PositionTest, Move) {
  chess::Position position;
  chess::GameResult result = position.move(move("d2d4"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "d2d4");
}
