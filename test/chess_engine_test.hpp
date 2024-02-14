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

TEST(PositionTest, Stalemate) {
  test_game({
    {"e2e3"},
    {"a7a5"},
    {"d1h5"},
    {"a8a6"},
    {"h5a5"},
    {"h7h5"},
    {"h2h4"},
    {"a6h6"},
    {"a5c7"},
    {"f7f6"},
    {"c7d7"},
    {"e8f7"},
    {"d7b7"},
    {"d8d3"},
    {"b7b8"},
    {"d3h7"},
    {"b8c8"},
    {"f7g6"},
    {"c8e6", chess::Winner::Draw, "stalemate"},
  });
}

TEST(PositionTest, InsufficientMaterial) {
  test_game({
    {"e2e4"},
    {"d7d5"},
    {"e4d5"},
    {"d8d5"},
    {"f1d3"},
    {"d5a2"},
    {"d3h7"},
    {"a2b1"},
    {"h7g8"},
    {"b1c2"},
    {"g8f7"},
    {"e8f7"},
    {"a1a7"},
    {"c2c1"},
    {"a7b7"},
    {"h8h2"},
    {"b7b8"},
    {"h2g2"},
    {"d1c1"},
    {"g2g1"},
    {"h1g1"},
    {"a8b8"},
    {"c1c7"},
    {"b8b2"},
    {"c7c8"},
    {"b2d2"},
    {"c8f8"},
    {"f7f8"},
    {"g1g7"},
    {"d2f2"},
    {"g7e7"},
    {"g8e7"},
    {"e1f2", chess::Winner::Draw, "insufficient material"},
  });
}
