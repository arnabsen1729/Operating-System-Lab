#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void green() { printf("\033[0;32m"); }
void reset() { printf("\033[0m"); }

void prompt() {
  green();
  // get current working directory
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  printf("%s> ", cwd);
  reset();
}

void fork_and_exec(char *cmd, char *args[]) {
  pid_t pid = fork();
  if (pid == 0) {
    // child process
    execvp(cmd, args);
    exit(0);
  } else if (pid < 0) {
    // error
    perror("fork");
    exit(1);
  } else {
    // parent process
    int status;
    waitpid(pid, &status, 0);
  }
}

void handle_command(char input[]) {
  // parse input
  char *token = strtok(input, " \n");
  char *argv[256];
  int argc = 0;
  while (token != NULL) {
    argv[argc] = token;
    argc++;
    token = strtok(NULL, " \n");
  }
  argv[argc] = NULL;
  // // debug
  // for (int i = 0; i < argc; i++) {
  //   printf("argv[%d] = [%s]\n", i, argv[i]);
  // }

  // execute command
  if (strcmp(argv[0], "exit") == 0) {
    exit(0);
  } else if (strcmp(argv[0], "echo") == 0) {
    for (int i = 1; i < argc; i++) {
      printf("%s ", argv[i]);
    }
    printf("\n");
  } else if (strcmp(argv[0], "cat") == 0) {
    for (int i = 1; i < argc; i++) {
      FILE *fp = fopen(argv[i], "r");
      if (fp == NULL) {
        printf("cat: %s: No such file or directory\n", argv[i]);
      } else {
        char line[256];
        while (fgets(line, 256, fp) != NULL) {
          printf("%s", line);
        }
        fclose(fp);
      }
    }
  } else if (strcmp(argv[0], "ls") == 0) {
    fork_and_exec("ls", argv);
  } else if (strcmp(argv[0], "pwd") == 0) {
    system("pwd");
  } else if (strcmp(argv[0], "vi") == 0) {
    fork_and_exec("vi", argv);
  } else if (strcmp(argv[0], "clear") == 0) {
    system("clear");
  } else if (strcmp(argv[0], "cd") == 0) {
    if (argc == 1) {
      chdir(getenv("HOME"));
    } else {
      chdir(argv[1]);
    }
  } else if (strcmp(argv[0], "help") == 0) {
    printf("Welcome to myshell\n\n");
    printf("Available commands:\n");
    printf("exit\n");
    printf("echo [arg...]\n");
    printf("cat [file...]\n");
    printf("ls\n");
    printf("pwd\n");
    printf("cd [dir]\n");
    printf("help\n");
    printf("clear\n");
  } else {
    printf("myshell: %s: command not found\n", argv[0]);
  }

  fflush(stdout);
}

int main() {
  // display shell prompt in a loop

  while (1) {
    prompt();

    // read input from stdin
    char input[256];
    fgets(input, 256, stdin);

    // split the input into commands with delimiter ';', '||', '&&'
    char command[256][256];
    int command_index = 0;
    char *token = strtok(input, ";||&&");
    while (token != NULL) {
      strcpy(command[command_index], token);
      command_index++;
      token = strtok(NULL, ";||&&");
    }

    // print all commands
    for (int i = 0; i < command_index; i++) {
      handle_command(command[i]);
    }
  }
  return 0;
}
