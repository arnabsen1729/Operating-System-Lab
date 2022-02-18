/**
 * @file main.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @date 2022-02-11
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

const int MAX_PROCESSES = 10;

/**
 * @brief Allocates the memory. Also handles the error if the memory is not
 * allocated.
 *
 * @param memory_size Size needed for shared memory
 * @return int Shared memory id
 */
int getSHM(size_t memory_size) {
  int shmid = shmget(IPC_PRIVATE, memory_size, IPC_CREAT | 0777);

  if (shmid == -1) { /* shmget() failed() */
    perror("shmget() failed: ");
    exit(1);
  }

  printf("shmget() returns shmid = %d.\n", shmid);
  return shmid;
}

/**
 * @brief Get the ptr to shared memory
 *
 * @param shmid Shared memory id
 * @return void* Pointer to shared memory
 */
void *getPtrSHM(int shmid) {
  void *value;
  value = shmat(shmid, NULL, 0);
  if (value == (void *)-1) { /* shmat fails */
    perror("shmat() failed");
    exit(1);
  }
  return value;
}

/**
 * @brief Release the shared memory
 *
 * @param shmid
 */
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

/**
 * @brief Access the shared memory at a specific location and return a pointer
 * to that location.
 *
 * @param ptr initial pointer to shared memory
 * @param row
 * @param col
 * @param m the matrix size
 * @return int* pointer to the specific location
 */
int *getPointerAt(int *ptr, int row, int col, int m) {
  return ptr + row * m + col;
}

/**
 * @brief Task for every child process.
 *
 * Calculate the matrix multiplication for one row of the final matrix.
 *
 * @param shmid
 * @param row
 * @param A
 * @param B
 * @param m
 * @param p
 */
void calculateResultRow(int shmid, int row, int **A, int **B, int m, int p) {
  int i, j;

  int *shared_memory = getPtrSHM(shmid);

  for (i = 0; i < p; i++) {
    int sum = 0;
    for (j = 0; j < m; j++) {
      sum += A[row][j] * B[j][i];
    }
    int *result = getPointerAt(shared_memory, row, i, m);
    *result = sum;
  }
}

/**
 * @brief Display the matrix in the shared memory
 *
 * @param shmid
 * @param n
 * @param m
 * @param p
 */
void displayResult(int shmid, int n, int m, int p) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < p; j++) {
      printf("%d ", *getPointerAt(getPtrSHM(shmid), i, j, m));
    }
    printf("\n");
  }
}

/**
 * @brief Initialize and allocate memory to a 2-d matrix of size r x c
 *
 * @param r
 * @param c
 * @return int**
 */
int **initializeMatrix(int r, int c) {
  int **M = (int **)malloc(r * sizeof(int *));
  for (int i = 0; i < r; i++) {
    M[i] = (int *)malloc(c * sizeof(int));
  }
  return M;
}

int main() {
  // taking input from user regarding the matrix size
  int n;
  int m;
  int p;
  printf("Details of Matrix A\n");
  printf("ROWS: ");
  scanf("%d", &n);
  printf("COLS: ");
  scanf("%d", &m);

  int **A = initializeMatrix(n, m);

  // taking the matrix value as input from user
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      printf("A[%d][%d]: ", i, j);
      scanf("%d", &A[i][j]);
    }
  }

  // taking input from user regarding the matrix size
  printf("Details of Matrix B\n");
  printf("ROWS: %d\n", m);
  printf("COLS: ");
  scanf("%d", &p);

  // taking the matrix value as input from user
  int **B = initializeMatrix(m, p);

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < p; j++) {
      printf("B[%d][%d]: ", i, j);
      scanf("%d", &B[i][j]);
    }
  }

  // creating shared memory
  int shmid = getSHM(n * p * sizeof(int));

  pid_t childPIDs[MAX_PROCESSES];  // array of child PIDs
  int childPIDIndex = 0;

  // create n child process
  for (int i = 0; i < n; i++) {
    pid_t pid = fork();
    childPIDs[childPIDIndex++] = pid;

    if (pid == 0) {
      // child process
      printf("Child process %d created.\n", i);
      // perform calculation to find the i-th row.
      // i.e Child 0 will calculate the first row, Child 1 will calculate the
      // second row and so on.
      calculateResultRow(shmid, i, A, B, m, p);
      exit(0);
    }
  }

  // wait for all child process to finish
  for (int i = 0; i < childPIDIndex; i++) {
    int wstatus;
    pid_t cpid = childPIDs[i], w;
    do {
      // wait for child process to finish
      w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
      if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
      printf("[%d] ", cpid);
      if (WIFEXITED(wstatus)) {
        printf("exited, status=%d\n", WEXITSTATUS(wstatus));
      } else if (WIFSIGNALED(wstatus)) {
        printf("killed by signal %d\n", WTERMSIG(wstatus));
      } else if (WIFSTOPPED(wstatus)) {
        printf("stopped by signal %d\n", WSTOPSIG(wstatus));
      } else if (WIFCONTINUED(wstatus)) {
        printf("continued\n");
      }
    } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  }

  // once all the child process is finished we can display the result
  displayResult(shmid, n, m, p);

  // release the shared memory
  destroySHM(shmid);
  return 0;
}
