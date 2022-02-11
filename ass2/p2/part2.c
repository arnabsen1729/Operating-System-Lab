/**
 * @file part2.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @brief To check if P1 closes the file abc using fp will P2 be able to read
 * from it
 * @date 2022-01-21
 */
#include <stdio.h>
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

  // Whichever process first gets executed will close the file.
  fclose(fp);  // closes the file after reading a word
}

int main() {
  FILE *fp = fopen(FILENAME, "r");  // open file for reading

  if (fp == NULL) {
    // in case file not found
    printf("ERROR: file doesn't exist\n");
    return 1;
  }

  pid_t pid = fork();  // create a child process P2

  if (pid == 0) {
    // child process P2
    printf("P2 reading from fp: \n");
    printf("before p2 read %d\n", ftell(fp));
    readWordAndPrint(fp);
    printf("after p2 read %d\n", ftell(fp));

  } else {
    // parent process P1
    printf("P1 reading from fp: \n");
    printf("before p2 read %d\n", ftell(fp));
    readWordAndPrint(fp);
    printf("after p2 read %d\n", ftell(fp));
  }
  return 0;
}
