#ifndef PROCESS_H_
#define PROCESS_H_

class Process {
 public:
  Process(const char *path, char *const argv[]);

  int read_fd;
  int write_fd;
};

#endif  // PROCESS_H_
