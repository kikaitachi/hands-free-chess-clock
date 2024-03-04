#include "logger.hpp"
#include "uci.hpp"
#include <cstdio>
#include <thread>

using namespace std::chrono_literals;

UniversalChessInterface::UniversalChessInterface(
      Process& process,
      std::function<void(const std::string best_move)> on_best_move
    ) : process(process), on_best_move(on_best_move) {
  std::thread read_thread(&UniversalChessInterface::read, this);
  read_thread.detach();
}

void UniversalChessInterface::read() {
  FILE *input = fdopen(process.read_fd, "r");
  char buffer[1024];
  while (fgets(buffer, sizeof(buffer), input) != NULL) {
    std::string line(buffer);
    if (line.ends_with("\n")) {
      line = line.substr(0, line.size() - 1);
    }
    logger::debug("uci: %s", line.c_str());
    if (line.starts_with("bestmove ")) {
      on_best_move(line.substr(9, 4));
    }
  }
  fclose(input);
}

void UniversalChessInterface::best_move(chess::Position& position) {
  std::string start_position = "position startpos moves";
  for (auto& move : position.moves) {
    start_position += " ";
    start_position += move.to_string();
  }
  process.write_line(start_position);
  logger::info("Initiating best move search");
  process.write_line("go infinite");
  std::this_thread::sleep_for(1s);
  process.write_line("stop");
}

std::optional<double> UniversalChessInterface::score() {
  return std::nullopt;
}
