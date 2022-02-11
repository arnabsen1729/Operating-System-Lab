/**
 * @file part1.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @brief To check if P1 closes the file (fclose()) does it get closed for P2 as
 * well
 * @date 2022-01-21
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

const char *FILENAME = "abc";

/**
 * @brief writes a word passed as argument into the file whose fp is passed.
 *
 * @param FILE* fp  filepointer to the file
 * @param char* word  word to be written into the file
 */
void writeWord(FILE *fp, char *word) {
  fprintf(fp, "%s\n", word);
  printf("%s\n", word);
  fflush(fp);

  // Whichever process first gets executed will close the file.
  fclose(fp);  // closes the file after reading a word
}

int main() {
  FILE *fp = fopen(FILENAME, "w");  // open file for writing

  pid_t pid = fork();  // create a child process P2

  if (pid == 0) {
    // child process P2
    printf("P2 writing to fp: \n");
    printf("before p2 write %d\n", ftell(fp));
    writeWord(fp, "World");
    printf("after p2 write %d\n", ftell(fp));

  } else {
    // parent process P1
    printf("P1 writing to fp: \n");
    printf("before p1 write %d\n", ftell(fp));
    writeWord(fp, "Hello");
    printf("after p1 write %d\n", ftell(fp));
  }
  return 0;
}
