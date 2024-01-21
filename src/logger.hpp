#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>

namespace logger {

  void debug(const std::string format, ...);
  void info(const std::string format, ...);
  void error(const std::string format, ...);
  void last(const std::string format, ...);
}

#endif  // LOGGER_H_
