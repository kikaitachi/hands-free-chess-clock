#ifndef COMMAND_PARSER_H_
#define COMMAND_PARSER_H_

#include <regex>
#include <string>

enum Command {
  NO_COMMAND,
  START_GAME,
  STOP_GAME,
  RESUME_GAME,
  SHUTDOWN,
  BEST_MOVE,
  WORST_MOVE,
  WHO_IS_WINNING,
};

class CommandParser {
 public:
  CommandParser();

  /**
   * Return true if given text was recognised as a valid command.
   * Only then getTime & getIncrement methods return meaningful values.
   */
  Command recognised(std::string text);

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
  std::regex stop_command_syntax;
  std::regex resume_command_syntax;
  std::regex shutdown_command_syntax;
  std::regex best_move_command_syntax;
  std::regex worst_move_command_syntax;
  std::regex who_is_winning_command_syntax;
};

#endif  // COMMAND_PARSER_H_
