#include "logger.hpp"
#include "uci.hpp"
#include <fstream>
#include <cstdlib>

UniversalChessInterface::UniversalChessInterface(std::string executable)
    : executable(executable) {
}

std::string UniversalChessInterface::best_move(chess::Position& position) {
  std::ofstream commands;
  commands.open("/tmp/commands.txt");
  commands << "position startpos moves";
  for (auto& move : position.moves) {
    commands << " ";
    commands << move.to_string();
  }
  commands << "\ngo depth 10\n";
  commands.close();

  std::string command = "cat /tmp/commands.txt | " + executable + " | grep bestmove > /tmp/best-move.txt";
  logger::debug("Calling UCI: %s", command.c_str());
  std::system(command.c_str());
  std::ifstream file("/tmp/best-move.txt");
  std::string line;
  if (getline(file, line)) {
    std::size_t pos = line.find(" ");
    if (pos == std::string::npos) {
      return "invalid format";
    }
    return line.substr(pos + 1);
  }
  return "empty file";
}
