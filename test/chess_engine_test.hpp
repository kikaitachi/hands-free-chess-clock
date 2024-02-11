#include <gtest/gtest.h>
#include "../src/chess_engine.hpp"

TEST(PositionTest, InitialMoves) {
  chess::Position position;
  EXPECT_EQ(position.generate_legal_moves().size(), 20);
}
