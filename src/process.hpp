#ifndef PROCESS_H_
#define PROCESS_H_

#include <string>
#include <vector>

class Process {
 public:
  Process(std::vector<std::string> command);

  /**
   * Terminate process.
   */
  ~Process();

  void write_line(std::string line);

  int read_fd;
  int write_fd;
};

#endif  // PROCESS_H_
