#include "logger.hpp"
#include "uci.hpp"
#include <cstdio>
#include <regex>
#include <thread>

UniversalChessInterface::UniversalChessInterface(char const *argv[])
    : process(argv) {
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
  if (line.starts_with("info depth")) {
    size_t index = line.find("score");
    if (index != line.npos) {
      chess::ScoreUnit unit = line.at(index + 6) == 'c' ?
        chess::ScoreUnit::Centipawn : chess::ScoreUnit::MateIn;
      size_t value_start = line.find(' ', index + 6);
      size_t value_end = line.find(' ', value_start + 1);
      std::lock_guard guard(score_mutex);
      score = {
        std::atoi(line.substr(value_start + 1, value_end - value_start - 1).c_str()),
        unit,
        std::atoi(line.substr(11, line.find(' ', 12) - 11).c_str())
      };
      score_found.notify_all();
    }
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

std::vector<chess::EvaluatedMove> UniversalChessInterface::evaluate_moves(
    const chess::Position& position,
    const std::vector<chess::Move>& moves,
    const int depth,
    const std::chrono::milliseconds timeout) {
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  end += timeout;
  send_position(position);
  std::vector<chess::EvaluatedMove> result;
  for (auto& move : moves) {
    process.write_line("go depth " + std::to_string(depth) + " searchmoves " + move.to_string());
    for ( ; ; ) {
      std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
      std::unique_lock<std::mutex> lock(score_mutex);
      score.reset();
      if (score_found.wait_for(lock, end - now) == std::cv_status::no_timeout) {
        if (score.has_value()) {
          if (score.value().depth == depth) {
            result.emplace(result.end(), move, score.value());
            break;
          }
        } else {
          return result;
        }
      } else {
        return result;
      }
    }
  }
  std::stable_sort(result.begin(), result.end());
  return result;
}

std::unique_ptr<UniversalChessInterface> create_uci(std::string command) {
  char const *uci_engine_argv[] = {
    command.c_str(), nullptr
  };
  return std::make_unique<UniversalChessInterface>(uci_engine_argv);
}
