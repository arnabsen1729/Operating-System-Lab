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

int n = 2;
int m = 3;
int p = 4;
const int A[2][3] = {{1, 2, 3}, {4, 5, 6}};                         // n*m
const int B[3][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};  // m*p

int getSHM(size_t memory_size) {
  int shmid = shmget(IPC_PRIVATE, memory_size, IPC_CREAT | 0777);

  if (shmid == -1) { /* shmget() failed() */
    perror("shmget() failed: ");
    exit(1);
  }

  printf("shmget() returns shmid = %d.\n", shmid);
  return shmid;
}

void destroySHM(int shmid) {
  int status;
  status = shmctl(shmid, IPC_RMID, NULL);
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

void calculateResultRow(int **result, int row) {
  int i, j;
  result[row] = (int *)malloc(sizeof(int) * p);
  for (i = 0; i < p; i++) {
    int sum = 0;
    for (j = 0; j < m; j++) {
      sum += A[row][j] * B[j][i];
    }
    result[row][i] = sum;
  }
}

void displayResult(int **result) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < p; j++) {
      printf("%d ", result[i][j]);
    }
    printf("\n");
  }
}

int main() {
  // int shared_matrix = getSHM(n * p * sizeof(int));
  // 2-d matrix with n rows and p columns
  int **result = (int **)malloc(n * sizeof(int *));

  for (int i = 0; i < m; i++) {
    calculateResultRow(result, i);
  }

  displayResult(result);
  return 0;
}
