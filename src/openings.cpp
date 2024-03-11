#include "logger.hpp"
#include "openings.hpp"
#include <chrono>
#include <fstream>

using namespace chess;

Openings::Openings(std::string path) {
  std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
  int opening_count = 0;
  int move_count = 0;
  for (char db = 'a'; db <= 'e'; db++) {
    std::string file_name = path + "/";
    file_name += db;
    file_name += ".tsv";
    std::ifstream file(file_name);
    if (file.is_open()) {
      std::string line;
      std::getline(file, line);  // Discard the first line as it is a header
      while (std::getline(file, line)) {
        opening_count++;
        chess::Position position;
        line = line.substr(4);  // Discard ECO classification
        std::size_t index = line.find_first_of('\t');
        std::string name = line.substr(0, index);
        Node* curr = root;
        for (int i = index + 1, end = line.size(); i < end; ) {
          index = line.find_first_of(' ', i);
          std::string san_notation = line.substr(i, index == line.npos ? end - i : index - i);
          i += san_notation.size() + 1;
          if (!san_notation.ends_with('.')) {  // Filter out move numbers
            move_count++;
            std::string uci_notation = position.move_san(san_notation);
            if (auto pos = curr->branches.find(uci_notation); pos != curr->branches.end()) {
              curr = pos->second;
            } else {
              Node* node = new Node();
              curr->branches.emplace(uci_notation, node);
              curr = node;
            }
          }
        }
        curr->name = name;
      }
    } else {
      logger::error("Can't open file: %s", file_name.c_str());
    }
  }
  std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
  logger::info("Loaded %d openings and %d moves in %dms", opening_count, move_count,
    (int)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());

}

std::optional<std::string> Openings::find(chess::Position position) {
  Node* curr = root;
  for (auto & position_move : position.moves) {
    if (curr == nullptr) {
      return std::nullopt;
    }
    std::string uci = position_move.to_string();
    for (auto& [opening_move, next] : curr->branches) {
      if (opening_move == uci) {
        curr = next;
        if (position_move == position.moves.back()) {
          if (curr != nullptr && curr->name) {
            return curr->name;
          }
        }
        goto found;
      }
    }
    return std::nullopt;
   found:
  }
  return std::nullopt;
}
