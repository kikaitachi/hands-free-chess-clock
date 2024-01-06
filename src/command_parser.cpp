module;

import logger;

#include <regex>
#include <string>

export module command_parser;

export class CommandParser {
 public:
  CommandParser()
      : command_syntax(
          "set (chess|just|shaft) (clock|block|lock|look|go off|go up) to (.*).",
          std::regex_constants::ECMAScript | std::regex_constants::icase) {
  }

  /**
   * Return true if given text was recognised as a valid command.
   * Only then getTime & getIncrement methods return meaningful values.
   */
  bool recognised(std::string text) {
    std::smatch matches;
    if (std::regex_search(text, matches, command_syntax)) {
      std::string time_part = matches[3].str();
      auto pos = time_part.find_first_of(' ');
      if (pos != time_part.npos) {
        std::string number = time_part.substr(0, pos);
        try {
          time = std::stoi(number) * 60 * 1000;
        } catch (std::exception e) {
          logger::error("%s can't be parsed as number: %s", number.c_str(), e.what());
        }
        increment = 0;
        return true;
      }
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
  std::regex command_syntax;
};
