/**
 * @file main.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @brief
 * Write a complete, user-friendly, well-documented and nicely-indented C
 * program which when executed as "./a.out executable1 executable2 ...
 * executableN" creates N child processes where the 1st child process 1st
 * executes the 1st executable file (given by executable1). 2nd executes the 2nd
 * executable file (given by executable2) and so on.
 * @date 2022-01-28
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

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
  for (int i = 1; i < argc; i++) {
    // iterating through the arguments which contains the executable file paths
    pid_t pid = Fork();

    if (pid == 0) {
      // child process
      printf("executing %d: %s\n", i, argv[i]);
      if (execvp(argv[i], (char *[]){argv[i], NULL}) == -1) {
        perror("[ERROR] execve failed");
        exit(EXIT_FAILURE);
      }
      exit(EXIT_SUCCESS);
    }
  }
  exit(EXIT_SUCCESS);
}
