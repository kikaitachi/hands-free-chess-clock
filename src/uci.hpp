#ifndef UCI_H_
#define UCI_H_

#include <string>

class UniversalChessInterface {
 public:
  UniversalChessInterface(std::string executable);

 private:
  std::string executable;
};

#endif  // UCI_H_
