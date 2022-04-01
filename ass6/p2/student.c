/**
 * @file student.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @brief Friday, 18 February Question 2
 * @date 2022-02-25
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>   /* for semget(2) semop(2) semctl(2) and sembuf */
#include <sys/types.h> /* for semget(2) ftok(3) semop(2) semctl(2) */
#include <unistd.h>

#define P(s) semop(s, &Pop, 1);     // wait operation
#define V(s) semop(s, &Vop, 1);     // signal operation
struct sembuf Pop;                  // wait operation flags
struct sembuf Vop;                  // signal operation flags
#define MAX_PROCESSES_ALLOWED 3     // we will allow at max 3 student process
#define NO_SEM 1                    // we need only 1 semaphore
typedef void (*sighandler_t)(int);  // define the sighandler
int semid = -1;                     // global semaphore id

union semun {
  int val;               /* Value for SETVAL */
  struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
  unsigned short *array; /* Array for GETALL, SETALL */
  struct seminfo *__buf; /* Buffer for IPC_INFO (Linux-specific) */
};

/**
 * @brief Initialize the Pop and Vop
 *
 */
void init() {
  Pop.sem_num = 0;
  Pop.sem_op = -1;
  Pop.sem_flg = SEM_UNDO;

  Vop.sem_num = 0;
  Vop.sem_op = 1;
  Vop.sem_flg = SEM_UNDO;
}

/**
 * @brief Handler for releasing semaphore
 *
 * @param signum
 */
void release_sem(int signum) {
  V(semid);  // since we are closing a process that opens up a spot for another
             // process
  int status;
  int v = semctl(semid, 0, GETVAL);
  if (v == MAX_PROCESSES_ALLOWED) {
    status = semctl(
        semid, 0,
        IPC_RMID); /* IPC_RMID is the command for destroying the semaphore*/
    if (status == 0) {
      fprintf(stderr, "\nRemoved semaphore with id = %d.\n", semid);
    } else if (status == -1) {
      fprintf(stderr, "Cannot remove semaphore with id = %d.\n", semid);
    } else {
      fprintf(stderr,
              "shmctl() returned wrong value while removing semaphore with id "
              "= %d.\n",
              semid);
    }
  }
  // int kill(pid_t pid, int sig);
  status = kill(0, SIGKILL);
  if (status == 0) {
    fprintf(stderr,
            "kill successful.\n"); /* this line may not be executed :P WHY?*/
  } else if (status == -1) {
    perror("kill failed.\n");
    fprintf(stderr, "Cannot remove semaphore with id = %d.\n", semid);
  } else {
    fprintf(stderr, "kill(2) returned wrong value.\n");
  }
}

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

/**
 * @brief Check if the semaphore exists
 *
 * if it doesn't exit initialize it
 *
 * @param semid
 */
void sem_init(int semid) {
  pid_t p = semctl(semid, 0, GETPID);

  if (p <= 0) {
    printf("initialising semaphores...\n");
    union semun setValArg;
    setValArg.val = MAX_PROCESSES_ALLOWED;
    if (semctl(semid, 0, SETVAL, setValArg) == -1) {
      perror("semctl() in sem_init() :");
      exit(1);
    }
  } else {
    printf("semaphores already initialised\n");
  }
}

void sem_log(int semid) { printf("sem = %d\n", semctl(semid, 0, GETVAL)); }

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("argument missing!\n");
    printf("USAGE: %s <integer-value>\n", argv[0]);
    exit(1);
  }

  init();
  signal(SIGINT, release_sem);

  key_t key = Ftok("./test", 1);
  semid = sem_create(key, NO_SEM);

  sem_init(semid);

  P(semid);
  sem_log(semid);
  while (1) {
    printf("I am student %d!\n", atoi(argv[1]));
    sleep(1);
  }
  return 0;
}
