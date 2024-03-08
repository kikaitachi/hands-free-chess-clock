#include <gtest/gtest.h>
#include "../src/openings.hpp"
#include "../src/logger.hpp"

chess::Openings openings("../test/openings");

TEST(OpeningsTest, UnknownOpening) {
  chess::Position position;
  position.move("a2a3");

  std::optional<std::string> name = openings.find(position);

  EXPECT_FALSE(name.has_value() == true);
}

TEST(OpeningsTest, SicilianDefense) {
  chess::Position position;
  position.move("e2e4");
  position.move("c7c5");

  std::optional<std::string> name = openings.find(position);

  EXPECT_TRUE(name.has_value() == true);
  EXPECT_EQ(name.value(), "Sicilian Defense");
}
