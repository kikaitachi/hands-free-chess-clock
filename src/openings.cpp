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
      while (std::getline(file, line)) {
        // TODO: implement
      }
    } else {
      logger::error("Can't open file: %s", file_name.c_str());
    }
  }
}
