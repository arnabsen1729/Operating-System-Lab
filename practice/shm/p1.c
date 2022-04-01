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

int shmid;

typedef void (*sighandler_t)(int);
void releaseSHM(int signum) {
  int status;
  // int shmctl(int shmid, int cmd, struct shmid_ds *buf);
  status = shmctl(
      shmid, IPC_RMID,
      NULL); /* IPC_RMID is the command for destroying the shared memory*/
  if (status == 0) {
    fprintf(stderr, "Remove shared memory with id = %d.\n", shmid);
  } else if (status == -1) {
    fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
  } else {
    fprintf(stderr,
            "shmctl() returned wrong value while removing shared memory with "
            "id = %d.\n",
            shmid);
  }

  // int kill(pid_t pid, int sig);
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

int main() {
  sighandler_t shandler;

  /* install signal handler */
  // sighandler_t signal(int signum, sighandler_t handler);
  shandler = signal(SIGINT, releaseSHM); /* should we call this seperately in
                                            parent and child process */

  shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);

  if (shmid == -1) {
    perror("shmget() failed");
    exit(1);
  }

  printf("shared memory created with id: %d\n", shmid);

  pid_t pid = fork();

  if (pid == 0) {
    int *value;
    value = shmat(shmid, NULL, 0);

    if (value == (void *)-1) {
      perror("shmat() failed");
      exit(1);
    }

    for (int i = 0; i < 5; i++) {
      printf("Child Reads %d.\n", *value);
      getchar();
    }

    shmdt(value);
    exit(0);
  } else {
    int *value;
    value = shmat(shmid, NULL, 0);

    if (value == (void *)-1) {
      perror("shmat() failed");
      exit(1);
    }

    for (int i = 0; i < 5; i++) {
      *value = i;
      printf("Parent Writes %d.\n", *value);
      getchar();
    }

    int status;
    pid_t pchild = wait(&status);
    printf("pid = %d status = %d!\n", pchild, status);
    releaseSHM(0);
    exit(0);
  }
  return 0;
}
