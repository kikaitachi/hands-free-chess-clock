#include <gtest/gtest.h>
#include "../src/uci.hpp"

TEST(UCITest, InitialPosition) {
  char const *uci_engine_argv[] = {
    "/usr/games/stockfish", nullptr
  };
  UniversalChessInterface uci(uci_engine_argv);

  chess::Position position;

  auto result = uci.evaluate_moves(position, position.generate_legal_moves());

  EXPECT_EQ(result.size(), 20);
  EXPECT_LT(result.front().score.value, result.back().score.value);

  // After e2e4 white has slight advantage
  position.move("e2e4");
  result = uci.evaluate_moves(position, position.generate_legal_moves());
  EXPECT_LT(result.back().score.value, 0);

  // After a2c3 black has slight disadvantage
  position.reset();
  position.move("c2c4");
  result = uci.evaluate_moves(position, position.generate_legal_moves());
  EXPECT_LT(result.back().score.value, 0);
}
