#include <stdio.h>
#include <stdlib.h> /* exit functions */
#include <string.h>
#include <sys/wait.h> /* wait */
#include <unistd.h>   /* read, write, pipe, _exit */

#define ReadEnd 0
#define WriteEnd 1
#define MAX_ARGS 5

void report_and_exit(const char *msg) {
  perror(msg);
  exit(-1); /** failure **/
}

int main(int argc, char *argv[]) {
  int pipeFDs[2]; /* two file descriptors */
  char buf;

  if (argc != 3) {
    printf("USAGE: %s <executable file 1> <executable file 2>\n", argv[0]);
    exit(1);
  }

  if (pipe(pipeFDs) < 0) report_and_exit("pipeFD");
  pid_t cpid1 = fork(); /* fork a child process */

  if (cpid1 == 0) {
    close(pipeFDs[ReadEnd]); /* child reads, doesn't write */
    dup2(pipeFDs[WriteEnd], STDOUT_FILENO);
    execlp(argv[1], argv[1], NULL);
    exit(0);
  } else {
    close(pipeFDs[WriteEnd]); /* parent reads, doesn't write */
    pid_t cpid2 = fork();
    if (cpid2 == 0) {
      dup2(pipeFDs[ReadEnd], STDIN_FILENO);
      execlp(argv[2], argv[2], NULL);
      exit(0);
    }
    close(pipeFDs[ReadEnd]);
    exit(0);
  }

  return 0;
}
