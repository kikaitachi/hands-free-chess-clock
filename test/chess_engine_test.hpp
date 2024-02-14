#include <gtest/gtest.h>
#include "../src/chess_engine.hpp"
#include <initializer_list>

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

class Move {
 public:
  std::string move;
  chess::Winner winner = chess::Winner::None;
  std::string message = move;
};

void test_game(std::initializer_list<Move> moves) {
  chess::Position position;
  for (auto& m : moves) {
    chess::GameResult result = position.move(move(m.move));
    EXPECT_EQ(result.winner, m.winner);
    EXPECT_EQ(result.message, m.message);
  }
}

TEST(PositionTest, InitialPosition) {
  chess::Position position;
  EXPECT_TRUE(position.white_turn);
  EXPECT_EQ(position.generate_legal_moves().size(), 20);
}

TEST(PositionTest, ScholarsMate) {
  test_game({
    {"e2e4"},
    {"e7e5"},
    {"f1c4"},
    {"b8c6"},
    {"d1h5"},
    {"g8f6"},
    {"h5f7", chess::Winner::White, "checkmate"},
  });
}

TEST(PositionTest, Repetition) {
  test_game({
    {"b1c3"},
    {"b8c6"},
    {"c3b1"},
    {"c6b8"},
    {"b1c3"},
    {"b8c6"},
    {"c3b1"},
    {"c6b8", chess::Winner::Draw, "repetition"},
  });
}
