#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>   /* for semget(2) semop(2) semctl(2) and sembuf */
#include <sys/types.h> /* for semget(2) ftok(3) semop(2) semctl(2) */
#include <time.h>
#include <unistd.h>

#define NO_SEM 2                    // we need only 2 semaphore
#define P(s) semop(s, &Pop, 1);     // wait operation
#define V(s) semop(s, &Vop, 1);     // signal operation
struct sembuf Pop;                  // wait operation flags
struct sembuf Vop;                  // signal operation flags
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

// /**
//  * @brief Get the semaphore of given idx from the semaphore set coressponding
//  to
//  * the semid (global variable)
//  *
//  * @param idx
//  * @return int
//  */
// int getSem(int idx) {
//   unsigned short int arr[SEM_SET_SIZE];
//   union semun getAllArg;
//   getAllArg.array = arr;
//   int status = semctl(semid, 0, GETALL, getAllArg);  // 0 doesn't matter

//   // check if semctl was successful or not
//   if (status == -1) {
//     perror("semctl() failed");
//     exit(1);
//   }

//   return arr[idx];
// }

int check_sem_exists(int semid) {
  pid_t p = semctl(semid, 0, GETPID);

  return (p <= 0 ? 0 : 1);
}

void semOperation(int semid, int idx, int op_val) {
  if (check_sem_exists(semid) == 0) {
    printf("semaphore doesn't exist\n");
    exit(1);
  }
  struct sembuf op;
  op.sem_num = idx;
  op.sem_op = op_val;
  op.sem_flg = SEM_UNDO;
  int status = semop(semid, &op, 1);

  if (status != 0) {
    perror("semop fail");
    exit(1);
  }
}

/**
 * @brief Handler for releasing semaphore
 *
 * @param signum
 */
void release_sem(int signum) {
  int status = semctl(
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
  // int kill(pid_t pid, int sig);
  status = kill(0, SIGKILL);
  if (status == 0) {
    fprintf(stderr,
            "kill susccesful.\n"); /* this line may not be executed :P WHY?*/
  } else if (status == -1) {
    perror("kill failed.\n");
    fprintf(stderr, "Cannot remove semaphore with id = %d.\n", semid);
  } else {
    fprintf(stderr, "kill(2) returned wrong value.\n");
  }
}

void log_time() {
  time_t rawtime;
  time(&rawtime);
  printf("Time: %s\n", ctime(&rawtime));
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
    setValArg.val = 1;
    if (semctl(semid, 0, SETVAL, setValArg) == -1) {
      perror("semctl() in sem_init() :");
      exit(1);
    }

    setValArg.val = 0;
    if (semctl(semid, 1, SETVAL, setValArg) == -1) {
      perror("semctl() in sem_init() :");
      exit(1);
    }
  } else {
    printf("semaphores already initialised\n");
  }
}

void sem_log(int semid) {
  printf("sem = %d, %d\n", semctl(semid, 0, GETVAL), semctl(semid, 1, GETVAL));
}

int main() {
  init();
  sighandler_t shandler = signal(SIGINT, release_sem);
  key_t mykey;
  pid_t pid;

  int status;

  mykey = Ftok("./test", 1);

  semid = sem_create(mykey, NO_SEM);
  printf("semaphore set created with id = %d\n", semid);
  sem_init(semid);
  // sem_log(semid);
  int count = 0;
  while (++count) {
    // 1, 0
    semOperation(semid, 0, -1);
    printf("%d I am producer\n", count);
    semOperation(semid, 1, 1);
    // sem_log(semid);
    sleep(2);
  }
}
