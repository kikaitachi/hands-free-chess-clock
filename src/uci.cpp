#include "logger.hpp"
#include "uci.hpp"
#include <cstdio>
#include <thread>

using namespace std::chrono_literals;

UniversalChessInterface::UniversalChessInterface(
      char const *argv[],
      std::function<void(const std::string best_move)> on_best_move
    ) : process(argv), on_best_move(on_best_move) {
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
    process_line(line);
  }
  fclose(input);
}

void UniversalChessInterface::process_line(std::string line) {
  if (line.starts_with("bestmove ")) {
    on_best_move(line.substr(9, 4));
  }
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

Stockfish::Stockfish(
    char const *argv[],
    std::function<void(const std::string best_move)> on_best_move
  ) : UniversalChessInterface(argv, on_best_move) {
}

void Stockfish::process_line(std::string line) {
  UniversalChessInterface::process_line(line);
  if (line.starts_with("Final evaluation")) {
    // TODO: implement
  }
}

std::optional<double> Stockfish::score() {
  process.write_line("eval");
  // TODO: wait until evaluation is done
  return std::nullopt;
}

std::unique_ptr<UniversalChessInterface> create_uci(
    std::string command,
    std::function<void(const std::string best_move)> on_best_move) {
  char const *uci_engine_argv[] = {
    command.c_str(), nullptr
  };
  if (command.ends_with("stockfish")) {
    return std::make_unique<Stockfish>(uci_engine_argv, on_best_move);
  }
  return std::make_unique<UniversalChessInterface>(uci_engine_argv, on_best_move);
}
