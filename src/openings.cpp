#include "chess_engine.hpp"
#include "logger.hpp"
#include "openings.hpp"
#include <fstream>
#include <regex>

using namespace openings;

Openings::Openings() {
  for (char db = 'a'; db <= 'e'; db++) {
    std::string file_name = "openings/";
    file_name += db;
    file_name += ".tsv";
    std::ifstream file(file_name);
    if (file.is_open()) {
      std::string line;
      std::getline(file, line);  // Discard the first line as it is a header
      while (std::getline(file, line)) {
        chess::Position position;
        line = line.substr(4);  // Discard ECO classification
        std::size_t index = line.find_first_of('\t');
        std::string name = line.substr(0, index);
        line = line.substr(index + 1);
        std::regex words_regex("[^\\s]+");
        auto words_begin = std::sregex_iterator(line.begin(), line.end(), words_regex);
        auto words_end = std::sregex_iterator();
        Node* curr = root;
        for (std::sregex_iterator i = words_begin; i != words_end; i++) {
          std::smatch match = *i;
          std::string san_notation = match.str();
          if (!san_notation.ends_with('.')) {  // Filter out move numbers
            std::string uci_notation = position.move_san(san_notation);
            if (auto pos = curr->branches.find(uci_notation); pos != curr->branches.end()) {
              curr = pos->second;
            } else {
              Node* node = new Node();
              curr->branches.emplace(uci_notation, node);
              curr = node;
            }
            if (std::distance(i, words_end) == 1) {  // Last move in opening
              curr->name = name;
            }
          }
        }
      }
    } else {
      logger::error("Can't open file: %s", file_name.c_str());
    }
  }
}
