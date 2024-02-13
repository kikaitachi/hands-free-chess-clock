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

TEST(PositionTest, ScholarsMate) {
  chess::Position position;

  // 1
  chess::GameResult result = position.move(move("e2e4"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "e2e4");

  result = position.move(move("e7e5"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "e7e5");

  // 2
  result = position.move(move("f1c4"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "f1c4");

  result = position.move(move("b8c6"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "b8c6");

  // 3
  result = position.move(move("d1h5"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "d1h5");

  result = position.move(move("g8f6"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "g8f6");

  // 4
  result = position.move(move("h5f7"));
  EXPECT_EQ(result.winner, chess::Winner::White);
  EXPECT_EQ(result.message, "checkmate");
}

TEST(PositionTest, Repetition) {
  chess::Position position;

  chess::GameResult result = position.move(move("b1c3"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "b1c3");

  result = position.move(move("b8c6"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "b8c6");

  result = position.move(move("c3b1"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "c3b1");

  result = position.move(move("c6b8"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "c6b8");

  result = position.move(move("b1c3"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "b1c3");

  result = position.move(move("b8c6"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "b8c6");

  result = position.move(move("c3b1"));
  EXPECT_EQ(result.winner, chess::Winner::None);
  EXPECT_EQ(result.message, "c3b1");

  result = position.move(move("c6b8"));
  EXPECT_EQ(result.winner, chess::Winner::Draw);
  EXPECT_EQ(result.message, "repetition");
}
