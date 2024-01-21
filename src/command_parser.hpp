#ifndef COMMAND_PARSER_H_
#define COMMAND_PARSER_H_

#include <regex>
#include <string>

class CommandParser {
 public:
  CommandParser();

  /**
   * Return true if given text was recognised as a valid command.
   * Only then getTime & getIncrement methods return meaningful values.
   */
  bool recognised(std::string text);

  /**
   * Get total time in milliseconds.
   */
  unsigned int getTime();

  /**
   * Get increment in milliseconds.
   */
  unsigned int getIncrement();

 private:
  unsigned int time;
  unsigned int increment;
  std::regex start_command_syntax;
};

#endif  // COMMAND_PARSER_H_
