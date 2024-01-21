#include "command_parser.hpp"
#include "logger.hpp"

CommandParser::CommandParser()
    : start_command_syntax(
        "start ([0-9]+)(-minute| minute| minutes) game",
        std::regex_constants::ECMAScript | std::regex_constants::icase) {
}

bool CommandParser::recognised(std::string text) {
  std::smatch matches;
  if (std::regex_search(text, matches, start_command_syntax)) {
    logger::debug("Match found");
    std::string number = matches[1].str();
    logger::debug("Time part: %s", number.c_str());
    try {
      time = std::stoi(number) * 60 * 1000;
    } catch (std::exception e) {
      logger::error("%s can't be parsed as number: %s", number.c_str(), e.what());
    }
    increment = 0;
    return true;
  }
  return false;
}

unsigned int CommandParser::getTime() {
  return time;
}

unsigned int CommandParser::getIncrement() {
  return increment;
}
