#include "logger.hpp"
#include "process.hpp"
#include <unistd.h>

Process::Process(std::vector<std::string> command) {
  int stdin_pipe[2], stdout_pipe[2];
  if (pipe(stdin_pipe) == -1) {
    logger::last("Failed to create stdin pipe for process %s", command.front().c_str());
    return;
  }
  if (pipe(stdout_pipe) == -1) {
    logger::last("Failed to create stdout pipe for process %s", command.front().c_str());
    return;
  }
  pid_t child_pid = fork();
  if (child_pid == -1) {
    logger::last("Failed to fork for process %s", command.front().c_str());
    return;
  }
  if (child_pid != 0) {  // This is parent process
    close(stdout_pipe[1]);
    close(stdin_pipe[0]);
    read_fd = stdout_pipe[0];
    write_fd = stdin_pipe[1];
  } else { // This is child process
    close(stdout_pipe[0]);
    close(stdin_pipe[1]);
    if (stdin_pipe[0] != 0) {
      dup2(stdin_pipe[0], 0);
      close(stdin_pipe[0]);
    }
    if (stdout_pipe[1] != 1) {
      dup2(stdout_pipe[1], 1);
      close(stdout_pipe[1]);
    }
    char * argv[command.size() + 1];
    for (int i = 0; i < command.size(); i++) {
      argv[i] = const_cast<char*>(command[i].c_str());
    }
    argv[command.size()] = nullptr;
    if (execv(command.front().c_str(), argv) == -1) {
      logger::last("Failed to execute process %s", command.front().c_str());
      exit(127);
    }
  }
}

Process::~Process() {
  close(write_fd);
  close(read_fd);
}

void Process::write_line(std::string line) {
  if (write(write_fd, (line + "\n").c_str(), line.size() + 1) == -1) {
    logger::last("Failed to write '%s' to process", line.c_str());
  }
}
