#include "logger.hpp"
#include "mpscq.hpp"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <string>
#include <thread>

using namespace std::chrono_literals;

namespace logger {

enum level { level_debug,
                    level_info,
                    level_warn,
                    level_error };

class LogEntry {
 public:
  LogEntry(char level, std::string message)
      : level(level), message(message) {
     clock_gettime(CLOCK_REALTIME, &timestamp);
  }

  void print() {
    struct tm time;
    localtime_r(&timestamp.tv_sec, &time);
    fprintf(
        stderr,
        "%02d-%02d-%02d %02d:%02d:%02d.%'09ld %c %s\n",
        time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
        time.tm_hour, time.tm_min, time.tm_sec,
        timestamp.tv_nsec, level, message.c_str());
  }

  char level;
  timespec timestamp;
  std::string message;
};

mpscq::Queue<LogEntry> entries;

static level current_level = level_debug;

void set_level(level log_level) {
  current_level = log_level;
}

level get_level() {
  return current_level;
}

static bool synchronous_logging = true;

void configure(bool synchronous) {
  synchronous_logging = synchronous;
  if (synchronous) {
    std::thread logging_thread([]() {
      for ( ; ; ) {
        entries.drain([](auto entry) {
          entry->print();
        });
        std::this_thread::sleep_for(100ms);
      }
    });
    logging_thread.detach();
  }
}

#define log(level, message)          \
  va_list argptr;                    \
  va_start(argptr, format);          \
  log_entry(level, message, argptr); \
  va_end(argptr)

void log_entry(const char level, const std::string format, va_list argptr) {
  char message[1024];
  int len = vsnprintf(message, sizeof(message), format.c_str(), argptr);
  if (synchronous_logging) {
    LogEntry(level, std::string(message, len)).print();
  } else {
    entries.push(new LogEntry(level, std::string(message, len)));
  }
}

void debug(const std::string format, ...) {
  if (current_level <= level_debug) {
    log('D', format);
  }
}

void warn(std::string format, ...) {
  if (current_level <= level_warn) {
    log('W', format);
  }
}

void error(const std::string format, ...) {
  if (current_level <= level_error) {
    log('E', format);
  }
}

void info(const std::string format, ...) {
  if (current_level <= level_info) {
    log('I', format);
  }
}

void last(const std::string format, ...) {
  std::string error(strerror(errno));
  log('L', format + ": " + error);
}

}  // namespace logger
