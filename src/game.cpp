#include "game.hpp"

void Game::reset(unsigned int time_ms, unsigned int increment_ms) {
  this->time_ms = time_ms;
  this->increment_ms = increment_ms;
  playing = true;
}
