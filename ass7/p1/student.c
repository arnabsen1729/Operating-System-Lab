#include <errno.h>  /* for perror */
#include <signal.h> /* for signal(2) kill(2) */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>   /* for shmget() shmctl() */
#include <sys/sem.h>   /* for wait() */
#include <sys/shm.h>   /* for shmget() shmctl() */
#include <sys/types.h> /* for wait() kill(2)*/
#include <sys/wait.h>  /* for wait() */
#include <time.h>
#include <unistd.h> /* for fork() */

#define SHARED_FILE "./file.txt"
#define P(s) semop(s, &Pop, 1);  // wait operation
#define V(s) semop(s, &Vop, 1);  // signal operation
struct sembuf Pop;               // wait operation flags
struct sembuf Vop;               // signal operation flags
#define NO_SEM 1                 // we need only 1 semaphore

int semid = -1;  // global semaphore id

union semun {
  int val;               /* Value for SETVAL */
  struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
  unsigned short *array; /* Array for GETALL, SETALL */
  struct seminfo *__buf; /* Buffer for IPC_INFO (Linux-specific) */
};

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
 * @brief Wrapper on ftok that handles the exception
 *
 * @param pathname
 * @param proj_id
 * @return key_t
 */
key_t Ftok(const char *pathname, int proj_id) {
  key_t key = ftok(pathname, proj_id);
  if (key == 0) {
    perror("ftok() in Ftok() :");
    exit(1);
  }

  return key;
}

// --------- SEMAPHORE RELATED FUNCTIONS --------------

/**
 * @brief Initialize the Pop and Vop
 *
 */
void init() {
  Pop.sem_num = 0;
  Pop.sem_op = -1;
  Pop.sem_flg = 0;

  Vop.sem_num = 0;
  Vop.sem_op = 1;
  Vop.sem_flg = 0;
}

/**
 * @brief Create semaphores
 *
 * @param key
 * @param count
 * @return int Semid of the semaphore
 */
int sem_create(key_t key, int count) {
  int semid = semget(key, count, IPC_CREAT | 0777);

  if (semid == -1) {
    perror("semget() in sem_create() :");
    exit(1);
  }
  return semid;
}

void sem_log(int semid) { printf("sem = %d\n", semctl(semid, 0, GETVAL)); }

/**
 * @brief Allocates the memory. Also handles the error if the memory is not
 * allocated.
 *
 * @param memory_size Size needed for shared memory
 * @return int Shared memory id
 */
int getSHM(int shmkey, size_t memory_size) {
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

  key_t mykey = Ftok(SHARED_FILE, 1);
  shmid = getSHM(mykey, sizeof(att));
  init();
  semid = sem_create(mykey, NO_SEM);
  sem_log(semid);

  att *attendance = (att *)getPtrSHM(shmid);
  while (true) {
    P(semid);
    if (attendance->roll == -1) {
      printf("Attendance given by: \n");
      printf("Roll number: %d\n", roll_number);
      attendance->roll = roll_number;
      attendance->tm = getCurrentTime();
      printf("Time: %s\n", ctime(&attendance->tm));
      V(semid);
      exit(EXIT_SUCCESS);
    }
    V(semid);
  }
  exit(EXIT_SUCCESS);
}
