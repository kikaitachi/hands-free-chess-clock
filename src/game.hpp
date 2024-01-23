#ifndef GAME_H_
#define GAME_H_

class Game {
 public:
  bool playing = false;

  void reset(unsigned int time_ms, unsigned int increment_ms);

 private:
  unsigned int time_ms;
  unsigned int increment_ms;
};

#endif  // GAME_H_
