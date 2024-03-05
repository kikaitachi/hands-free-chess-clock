#include "chess_engine.hpp"
#include "logger.hpp"
#include "openings.hpp"
#include <fstream>

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
        logger::debug("%s: %s", name.c_str(), line.c_str());
        // TODO: implement
      }
    } else {
      logger::error("Can't open file: %s", file_name.c_str());
    }
  }
}
