#include "command_parser.hpp"
#include "logger.hpp"
#include <vector>

static std::vector<std::string> numbers = {
  "zero",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
  "twelve",
};

CommandParser::CommandParser()
    : start_command_syntax(
        "start ([0-9]+)(?:-minute| minute| minutes) game(?: with ([0-9]+)(?:-second| second| seconds) increment)?",
        std::regex_constants::ECMAScript | std::regex_constants::icase),
      stop_command_syntax(
        "stop the game",
        std::regex_constants::ECMAScript | std::regex_constants::icase),
      resume_command_syntax(
        "continue game",
        std::regex_constants::ECMAScript | std::regex_constants::icase),
      shutdown_command_syntax(
        "shut ?down",
        std::regex_constants::ECMAScript | std::regex_constants::icase) {
}

Command CommandParser::recognised(std::string text) {
  for (int i = 0; i < numbers.size(); i++) {
    text = std::regex_replace(text, std::regex(numbers[i]), std::to_string(1));
  }
  std::smatch matches;
  if (std::regex_search(text, matches, start_command_syntax)) {
    logger::debug("%d matches found", matches.size());
    std::string number = matches[1].str();
    logger::debug("Time part: %s", number.c_str());
    try {
      time = std::stoi(number) * 60 * 1000;
    } catch (std::exception e) {
      logger::error("%s can't be parsed as number: %s", number.c_str(), e.what());
      return NO_COMMAND;
    }
    increment = 0;
    if (matches.size() > 2) {
      number = matches[2].str();
      logger::debug("Increment part: %s", number.c_str());
      if (!number.empty()) {
        try {
          increment = std::stoi(number) * 1000;
        } catch (std::exception e) {
          logger::error("%s can't be parsed as number: %s", number.c_str(), e.what());
          return NO_COMMAND;
        }
      }
    }
    return START_GAME;
  } else if (std::regex_search(text, matches, stop_command_syntax)) {
    return STOP_GAME;
  } else if (std::regex_search(text, matches, resume_command_syntax)) {
    return RESUME_GAME;
  } else if (std::regex_search(text, matches, shutdown_command_syntax)) {
    return SHUTDOWN;
  }
  return NO_COMMAND;
}

unsigned int CommandParser::getTime() {
  return time;
}

unsigned int CommandParser::getIncrement() {
  return increment;
}
