#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char *FILEPATH = "file.txt";

/**
 * @brief read from fin, closes the fin and returns the first line
 *
 * @param fin
 * @return char*
 */

void myfscanf(FILE *fin, char *buff) {
  int index = 0;
  while ((buff[index] = fgetc(fin)) != ' ') {
    index++;
  }
  buff[index] = '\0';
}

void readLineFILE(FILE *fin, bool isChild) {
  size_t maxChar = 100;

  char *process = isChild ? "child" : "parent";

  char c[255];
  printf("%s started from %ld\n", process, ftell(fin));
  // fgets(c, maxChar, fin);
  myfscanf(fin, c);
  printf("%s: %s index: %ld\n", process, c, ftell(fin));
}

/**
 * @brief forks a process and read file from the child class only
 *
 */
void forkAndRead() {
  FILE *fin = fopen(FILEPATH, "r");  // filepath created before fork
  pid_t p = fork();

  if (p == 0) {
    readLineFILE(fin, true);
  } else {
    readLineFILE(fin, false);
  }

  // fclose(fin);
}

int main() {
  forkAndRead();
  return 0;
}
// Observations
//  while one is reading, other one cannot read
