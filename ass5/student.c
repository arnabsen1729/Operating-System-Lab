#include <errno.h>  /* for perror */
#include <signal.h> /* for signal(2) kill(2) */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>   /* for shmget() shmctl() */
#include <sys/shm.h>   /* for shmget() shmctl() */
#include <sys/types.h> /* for wait() kill(2)*/
#include <sys/wait.h>  /* for wait() */
#include <time.h>
#include <unistd.h> /* for fork() */

#define SHARED_FILE "./"

typedef void (*sighandler_t)(int);  // define the sighandler_t type

typedef struct att {
  int roll; /* The roll number of the student giving the attendance */

  time_t
      tm; /* To keep the time when attendance was given by the student. Read the
             manual for time(), that is, "man 2 time" Also see the manual
             entries mentioned in the "see also" section of this manual page! */
} att;

int shmid;

/**
 * @brief Allocates the memory. Also handles the error if the memory is not
 * allocated.
 *
 * @param memory_size Size needed for shared memory
 * @return int Shared memory id
 */
int getSHM(size_t memory_size) {
  int shmkey = ftok(SHARED_FILE, 1);
  int shmid = shmget(shmkey, memory_size, IPC_CREAT | 0777);

  if (shmid == -1) { /* shmget() failed() */
    perror("shmget() failed: ");
    exit(1);
  }
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

time_t getCurrentTime() {
  time_t rawtime;
  time(&rawtime);
  return rawtime;
}

int main(int argc, const char *argv[]) {
  if (argc == 1 || argc > 2) {
    printf("Usage: ./student.out <roll_number>\n");
    exit(1);
  }

  int roll_number = atoi(argv[1]);

  shmid = getSHM(sizeof(att));

  att *attendance = (att *)getPtrSHM(shmid);
  while (true) {
    if (attendance->roll == -1) {
      printf("Attendance given by: \n");
      printf("Roll number: %d\n", roll_number);
      attendance->roll = roll_number;
      attendance->tm = getCurrentTime();
      printf("Time: %s\n", ctime(&attendance->tm));
      exit(EXIT_SUCCESS);
    }
  }
  exit(EXIT_SUCCESS);
}
