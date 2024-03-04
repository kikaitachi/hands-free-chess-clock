#ifndef PROCESS_H_
#define PROCESS_H_

#include <string>

class Process {
 public:
  Process(char const *argv[]);
  void write_line(std::string line);

  int read_fd;
  int write_fd;
};

#endif  // PROCESS_H_
