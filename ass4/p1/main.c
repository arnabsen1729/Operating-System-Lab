/**
 * @file main.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @brief
Let there be two  variables n (an integer variable) and c (a character
variable) which are shared between two processes P1 (parent) and P2 (child).P1
initializes c  to 'y' (the character 'y' for 'yes') at the beginning.

Subsequently, in an infinite loop, whenever  P1 finds 'y' in the shared variable
c, it puts 'n' in c and a random (integer) number in the shared variable n. On
the other hand, P2, in an infinite loop, whenever finds 'n' in the shared
variable c, it computes and prints the factorial of the number it finds in the
shared variable n, and puts 'y' in c.

Write a complete C program to implement the above. The program should make
arrangement for releasing shared memory that has been created during execution.

The processes must print meaningful output so that the user understands what is
happening.
 * @date 2022-02-04
 *
 */

#include <errno.h>  /* for perror */
#include <signal.h> /* for signal(2) kill(2) */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>   /* for shmget() shmctl() */
#include <sys/shm.h>   /* for shmget() shmctl() */
#include <sys/types.h> /* for wait() kill(2)*/
#include <sys/wait.h>  /* for wait() */
#include <unistd.h>    /* for fork() */

/* defined globally so that signal handler can access it */
int shmid_c;  // shared memory id for character c
int shmid_n;  // shared memory id for number n

/* following is a signal handler for the keypress ^C, that is, ctrl-c */
typedef void (*sighandler_t)(int);
// function pointer that takes int value and return type is void

#define MAX_FACTORIAL 10

/**
 * @brief Finding the nth factorial
 *
 * @param value
 * @return int
 */
int factorial(int value) {
  int result = 1;

  for (int i = 2; i <= value; i++) result *= i;

  return result;
}

/**
 * @brief Destroy the shared memory
 *
 * @param shmid
 */
void destroySHM(int shmid) {
  int status;
  status = shmctl(
      shmid, IPC_RMID,
      NULL); /* IPC_RMID is the command for destroying the shared memory*/
  if (status == 0) {
    fprintf(stderr, "\nRemove shared memory with id = %d.\n", shmid);
  } else if (status == -1) {
    fprintf(stderr, "\nCannot remove shared memory with id = %d.\n", shmid);
  } else {
    fprintf(stderr,
            "\nshmctl() returned wrong value while removing shared memory with "
            "id = %d.\n",
            shmid);
  }
  status = kill(0, SIGKILL);

  if (status == 0) {
    fprintf(stderr,
            "kill susccesful.\n"); /* this line may not be executed :P WHY?*/
  } else if (status == -1) {
    perror("kill failed.\n");
    fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
  } else {
    fprintf(stderr, "kill(2) returned wrong value.\n");
  }
}

/**
 * @brief Get the shared memory id
 *
 * @param shmid
 */
void getSHM(int *shmid) {
  *shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0777);

  if ((*shmid) == -1) { /* shmget() failed() */
    perror("shmget() failed: ");
    exit(1);
  }

  printf("shmget() returns shmid = %d.\n", *shmid);
}

/**
 * @brief Handler for the signal SIGINT
 *
 */
void releaseSHM() {
  destroySHM(shmid_c);
  destroySHM(shmid_n);
}

/**
 * @brief Get the pointer to the SHM object
 *
 * @param shmid
 * @param isParent
 * @return void*
 */
void *getPtrSHM(int shmid, bool isParent) {
  void *value;
  char msg[30];
  sprintf(msg, "shmat() failed at %s", isParent ? "parent" : "child");
  value = shmat(shmid, NULL, 0);
  if (value == (void *)-1) { /* shmat fails */
    perror(msg);
    exit(1);
  }
  return value;
}

int main() {
  int status;
  pid_t p1 = 0;

  sighandler_t shandler;

  shandler = signal(SIGINT, releaseSHM);

  getSHM(&shmid_c);
  getSHM(&shmid_n);
  printf("Parent process: shmid_c = %d, shmid_n = %d.\n", shmid_c, shmid_n);

  p1 = fork();
  if (p1 == 0) { /* child process */
    char *c = getPtrSHM(shmid_c, false);
    int *n = getPtrSHM(shmid_n, false);

    while (1) {
      if ((*c) == 'n') {
        printf("Child process calculating factorial of %d is %d\n", *n,
               factorial(*n));
        *c = 'y';
        sleep(1);
      }
    }

    exit(0);

  } else { /* parent process */
    int i;
    char *c = getPtrSHM(shmid_c, true);
    int *n = getPtrSHM(shmid_n, true);
    *c = 'y';

    while (1) {
      if ((*c) == 'y') {
        *c = 'n';
        *n = rand() % MAX_FACTORIAL;
        printf("Parent updated SHM to %d\n", *n);
      }
    }
    exit(0);
  }
}
