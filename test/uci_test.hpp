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
}
