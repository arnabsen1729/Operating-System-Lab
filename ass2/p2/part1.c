/**
 * @file part1.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @brief To check if P2 can read from the file abc using fp
 * @date 2022-01-21
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

const char *FILENAME = "abc";

/**
 * @brief reads a word using the file pointer passed and prints it to stdout
 *
 * @param FILE* fp the file pointer to read from
 */
void readWordAndPrint(FILE *fp) {
  char buff[255];
  fscanf(fp, "%s", buff);
  printf("%s\n", buff);
}

int main() {
  FILE *fp = fopen(FILENAME, "r");  // open file for reading
  int abc = 100;
  if (fp == NULL) {
    // in case file not found
    printf("ERROR: file doesn't exist\n");
    return 1;
  }

  printf("%x %ld\n", fp, ftell(fp));
  pid_t pid = fork();  // create a child process P2
  if (pid == 0) {
    // child process P2
    // printf("%x %d\n", &abc, abc);
    printf("%x %ld\n", fp, ftell(fp));
    printf("P2 reading from fp: ");
    fflush(stdout);
    readWordAndPrint(fp);
    exit(0);
  }
  // parent process P1

  printf("%x %ld\n", fp, ftell(fp));
  printf("P1 reading from fp: ");
  fflush(stdout);
  readWordAndPrint(fp);

  return 0;
}
