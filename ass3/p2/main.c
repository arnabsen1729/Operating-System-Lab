/**
 * @file main.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @brief
 * Consider pass command line arguments to the executables
 * @date 2022-01-28
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

const int MAX_ARGS = 10;

/**
 * @brief Fork is a wrapper on the fork() which handles the error cases.
 *
 * @return pid_t
 */
pid_t Fork() {
  pid_t pid;

  if ((pid = fork()) < 0) {
    // pid is less than 0, fork() failed
    perror("Fork error");
    exit(EXIT_FAILURE);
  }
  return pid;
}

int main(int argc, char *argv[]) {
  int argIndex = 1;

  while (argIndex < argc) {
    char *execArgs[MAX_ARGS];
    execArgs[0] = argv[argIndex];  // executable name
    int execArgIndex = 1;
    while (execArgIndex < MAX_ARGS && argIndex + 1 < argc &&
           argv[argIndex + 1][0] == '-') {
      execArgs[execArgIndex++] = argv[++argIndex];
    }
    execArgs[execArgIndex] = NULL;

    pid_t pid = Fork();
    if (pid == 0) {
      if (execvp(execArgs[0], execArgs) == -1) {
        perror("[ERROR] execve failed");
        exit(EXIT_FAILURE);
      }
    }

    argIndex++;
  }

  exit(EXIT_SUCCESS);
}
