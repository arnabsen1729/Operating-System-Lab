/**
 * @file p1.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @brief
 * @date 2022-04-01
 *
 */

#include <stdio.h>
#include <stdlib.h> /* exit functions */
#include <string.h>
#include <sys/wait.h> /* wait */
#include <unistd.h>   /* read, write, pipe, _exit */

#define ReadEnd 0
#define WriteEnd 1

void report_and_exit(const char *msg) {
  perror(msg);
  exit(-1); /** failure **/
}

// function to count the characters, lines and words in a file
void display_stats(FILE *fd) {
  // using the logic
  // # of words = # of spaces or newlines + 1
  // # of lines = # of newlines + 1
  int chars = 0, lines = 0, words = 0;
  char c;
  while ((c = fgetc(fd)) != EOF) {
    chars++;
    if (c == '\n') lines++;
    if (c == ' ' || c == '\n') words++;
  }
  printf("characters = %d\nlines = %d\nwords = %d\n", chars, lines, words);
}
int main(int argc, char *argv[]) {
  int pipeFDs[2]; /* two file descriptors */
  char buf;

  if (pipe(pipeFDs) < 0) report_and_exit("pipeFD");
  pid_t cpid = fork(); /* fork a child process */

  if (cpid == 0) {
    close(pipeFDs[WriteEnd]); /* child reads, doesn't write */
    char filename[100];
    int index = 0;

    while (read(pipeFDs[ReadEnd], &buf, 1) > 0) {
      // we are using space as a deliminator between different files
      if (buf == ' ') {
        // we have reached the end of the filename
        filename[index] = '\0';
        printf("Statistics of file %s:\n", filename);
        printf("-------------------\n");

        // open the file and display the statistics
        FILE *fd = fopen(filename, "r");
        if (fd == NULL) report_and_exit("fopen :");
        display_stats(fd);
        printf("\n");

        // reset the index and filename
        index = 0;
      } else {
        filename[index] = buf;
        index++;
      }
    }

    close(pipeFDs[ReadEnd]);
    exit(0);
  } else {
    close(pipeFDs[ReadEnd]); /* parent writes, doesn't read */

    // going through the filenames and writing them to the pipe
    for (int i = 1; i < argc; i++) {
      write(pipeFDs[WriteEnd], argv[i], strlen(argv[i]));

      // deliminator to indicate the end of the filename
      write(pipeFDs[WriteEnd], " ", 1);
    }
    close(pipeFDs[WriteEnd]); /* done writing: generate eof */
    wait(NULL);               /* wait for child to exit */
    exit(0);                  /* exit normally */
  }
  return 0;
}
