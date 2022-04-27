#include <stdio.h>
#include <stdlib.h> /* exit functions */
#include <string.h>
#include <sys/wait.h> /* wait */
#include <unistd.h>   /* read, write, pipe, _exit */

#define ReadEnd 0
#define WriteEnd 1
#define MAX_ARGS 10

void report_and_exit(const char *msg) {
  perror(msg);
  exit(-1); /** failure **/
}

int main(int argc, char *argv[]) {
  int pipeFDs[MAX_ARGS][2]; /* two file descriptors */
  char buf;

  if (argc < 2) {
    printf("USAGE: %s <executable file 1> <executable file 2>\n", argv[0]);
    exit(1);
  }

  for (int i = 0; i < MAX_ARGS; i++) {
    if (pipe(pipeFDs[i]) == -1) {
      report_and_exit("pipe");
    }
  }

  pid_t cpid1 = fork();
  if (cpid1 == 0) {
    dup2(pipeFDs[0][WriteEnd], STDOUT_FILENO);
    execlp(argv[1], argv[1], NULL);
    exit(0);
  }

  close(pipeFDs[0][WriteEnd]);
  wait(NULL);

  for (int i = 2; i < argc; i++) {
    int readFrom = i - 2;
    int writeTo = (i - 1);
    pid_t cpid2 = fork();
    if (cpid2 == 0) {
      dup2(pipeFDs[writeTo][WriteEnd], STDOUT_FILENO);
      dup2(pipeFDs[readFrom][ReadEnd], STDIN_FILENO);
      execlp(argv[i], argv[i], NULL);
      exit(0);
    }
    wait(NULL);
    close(pipeFDs[writeTo][WriteEnd]);
  }
  while (read(pipeFDs[argc - 2][ReadEnd], &buf, 1) > 0)
    write(STDOUT_FILENO, &buf, sizeof(buf));

  return 0;
}
