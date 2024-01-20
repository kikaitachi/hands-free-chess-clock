module;

import logger;

#include <regex>
#include <string>

export module command_parser;

export class CommandParser {
 public:
  CommandParser()
      : start_command_syntax(
          "start ([0-9]+)(-minute| minute| minutes) game",
          std::regex_constants::ECMAScript | std::regex_constants::icase) {
  }

  /**
   * Return true if given text was recognised as a valid command.
   * Only then getTime & getIncrement methods return meaningful values.
   */
  bool recognised(std::string text) {
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

  /**
   * Get total time in milliseconds.
   */
  unsigned int getTime() {
    return time;
  }

  /**
   * Get increment in milliseconds.
   */
  unsigned int getIncrement() {
    return increment;
  }

 private:
  unsigned int time;
  unsigned int increment;
  std::regex start_command_syntax;
};
