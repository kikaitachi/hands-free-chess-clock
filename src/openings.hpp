#ifndef OPENINGS_H_
#define OPENINGS_H_

#include "chess_engine.hpp"
#include <map>
#include <optional>

namespace openings {

class Node {
 public:
  std::optional<std::string> name;
  std::map<std::string, Node*> branches;
};

class Openings {
 public:
  Openings();
  std::optional<std::string> find(chess::Position position);

 private:
  Node* root = new Node();
};

}

#endif  // OPENINGS_H_
