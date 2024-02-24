#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>

namespace logger {
  /**
   * Configure logger.
   */
  void configure(bool synchronous);

  /**
   * Log a message with a debug level.
   */
  void debug(const std::string format, ...);

  /**
   * Log a message with a info level.
   */
  void info(const std::string format, ...);

  /**
   * Log a message with a warning level.
   */
  void warn(const std::string format, ...);

  /**
   * Log a message with a error level.
   */
  void error(const std::string format, ...);

  /**
   * Log a message with a error level and append error from the last call.
   */
  void last(const std::string format, ...);
}

#endif  // LOGGER_H_
