#include "logger.hpp"
#include "uci.hpp"
#include <cstdio>
#include <regex>
#include <thread>

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

void UniversalChessInterface::send_position(const chess::Position& position) {
  std::string start_position = "position startpos moves";
  for (auto& move : position.moves) {
    start_position += " ";
    start_position += move.to_string();
  }
  process.write_line(start_position);
}

void UniversalChessInterface::best_move(const chess::Position& position) {
  send_position(position);
  logger::info("Initiating best move search");
  process.write_line("go infinite");
  std::this_thread::sleep_for(1s);
  process.write_line("stop");
}

std::optional<double> UniversalChessInterface::get_score(
    const chess::Position& position,
    const std::chrono::milliseconds timeout) {
  return std::nullopt;
}

Stockfish::Stockfish(
    char const *argv[],
    std::function<void(const std::string best_move)> on_best_move
  ) : UniversalChessInterface(argv, on_best_move) {
}

void Stockfish::process_line(std::string line) {
  UniversalChessInterface::process_line(line);
  std::regex eval_syntax("Final evaluation +([+--0-9.]+) ");
  std::smatch matches;
  if (std::regex_search(line, matches, eval_syntax)) {
    std::string value = matches[1].str();
    {
      std::lock_guard guard(score_mutex);
      score = std::atof(value.c_str());
    }
    score_found.notify_all();
  }
}

std::optional<double> Stockfish::get_score(
    const chess::Position& position,
    const std::chrono::milliseconds timeout) {
  send_position(position);
  std::unique_lock<std::mutex> lock(score_mutex);
  score = std::nullopt;
  process.write_line("eval");
  if (score_found.wait_for(lock, timeout) == std::cv_status::no_timeout) {
    return score;
  }
  logger::warn("Timeout while waiting to get position score");
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
