#include <stdio.h>
#include <stdlib.h> /* exit functions */
#include <string.h>
#include <sys/wait.h> /* wait */
#include <unistd.h>   /* read, write, pipe, _exit */

#define ReadEnd 0
#define WriteEnd 1

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1); /** failure **/
}

int main() {
  int pipeFDs[2];                                     /* two file descriptors */
  char buf;                                           /* 1-byte buffer */
  const char* msg = "Nature's first green is gold\n"; /* bytes to write */

  if (pipe(pipeFDs) < 0) report_and_exit("pipeFD");
  pid_t cpid = fork();                   /* fork a child process */
  if (cpid < 0) report_and_exit("fork"); /* check for failure */

  if (0 == cpid) { /*** child ***/ /* child process */
    printf("Child process: \n");
    close(pipeFDs[WriteEnd]); /* child reads, doesn't write */

    while (read(pipeFDs[ReadEnd], &buf, 1) >
           0) /* read until end of byte stream */
      write(STDOUT_FILENO, &buf, sizeof(buf)); /* echo to the standard output */

    close(pipeFDs[ReadEnd]); /* close the ReadEnd: all done */
    _exit(0);                /* exit and notify parent at once  */
  }                          /* child ends */
  else {                     /*** parent ***/
    printf("Parent process: \n");
    close(pipeFDs[ReadEnd]); /* parent writes, doesn't read */
    printf("Writing to pipe: %s\n ", msg);
    write(pipeFDs[WriteEnd], msg,
          strlen(msg));       /* write the bytes to the pipe */
    close(pipeFDs[WriteEnd]); /* done writing: generate eof */

    wait(NULL); /* wait for child to exit */
    exit(0);    /* exit normally */
  }
  return 0;
}
