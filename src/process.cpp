#include "logger.hpp"
#include "process.hpp"
#include <unistd.h>

Process::Process(const char *path, char const *argv[]) {
  int stdin_pipe[2], stdout_pipe[2];
  if (pipe(stdin_pipe) == -1) {
    logger::last("Failed to create stdin pipe for process %s", path);
    return;
  }
  if (pipe(stdout_pipe) == -1) {
    logger::last("Failed to create stdout pipe for process %s", path);
    return;
  }
  pid_t child_pid = fork();
  if (child_pid == -1) {
    logger::last("Failed to fork for process %s", path);
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
    if (execv(path, const_cast<char**>(argv)) == -1) {
      logger::last("Failed to execute process %s", path);
    }
  }
}
