#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>    /* for exit(3) */
#include <sys/ipc.h>   /* for semget(2) ftok(3) semop(2) semctl(2) */
#include <sys/sem.h>   /* for semget(2) semop(2) semctl(2) */
#include <sys/types.h> /* for semget(2) ftok(3) semop(2) semctl(2) */
#include <time.h>      /* for fork(2) */
#include <unistd.h>    /* for fork(2) */

int semid = -1;

typedef void (*sighandler_t)(int);  // define the sighandler
const int SEM_SET_SIZE = 2;         // one extra to keep count of the number of
                                    // processes accessing the semaphore set

/* struct sembuf has the following fields */
// unsigned short sem_num;  /* semaphore number */
// short          sem_op;   /* semaphore operation */
// short          sem_flg;  /* operation flags */

// key_t ftok(const char *pathname, int proj_id);
union semun {
  int val;               /* Value for SETVAL */
  struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
  unsigned short *array; /* Array for GETALL, SETALL */
  struct seminfo *__buf; /* Buffer for IPC_INFO (Linux-specific) */
} setValArg;

/**
 * @brief prints the semaphore set coressponding to the semid (global variable)
 *
 */
void print_sem_set() {
  printf("sem set: ");
  unsigned short int arr[SEM_SET_SIZE];
  union semun getAllArg;
  getAllArg.array = arr;
  int status = semctl(semid, 0, GETALL, getAllArg);

  for (int i = 0; i < SEM_SET_SIZE; i++) printf("%d\t", arr[i]);
  printf("\n");
}

/**
 * @brief wrapper for semop with error checking and reporting, semid required is
 * made global
 *
 * @param idx
 * @param op_val the chage to be done to the semaphore of idx
 * @param sem_flg the sem_flg for the semop
 * @param verbose to add verbosity to the console
 */
void semOperation(int idx, int op_val, short sem_flg, bool verbose) {
  struct sembuf op;

  /* struct sembuf has the following fields */
  // unsigned short sem_num;  -> semaphore number -> index of the semaphone in
  // set short          sem_op;   -> semaphore operation -> +ve number to add,
  // -ve value try to decrement or wait,
  //                             0 will make process wait till semaphore is not
  //                             0
  // short          sem_flg;  -> operation flags
  op.sem_num = idx;
  op.sem_op = op_val;
  op.sem_flg = sem_flg;  // SEM_UNDO -> opreation done to semaphore will be
                         // undone when the process completes
  // other option of flag -> IPC_NOWAIT -> throw error if process have to wait
  int status = semop(semid, &op, 1);

  if (status == 0) {
    if (verbose) printf("Succesfully did %d on index %d\n", op_val, idx);
  } else {
    perror("semop fail");
    exit(1);
  }
}

/**
 * @brief Get the semaphore of given idx from the semaphore set coressponding to
 * the semid (global variable)
 *
 * @param idx
 * @return int
 */
int getSem(int idx) {
  unsigned short int arr[SEM_SET_SIZE];
  union semun getAllArg;
  getAllArg.array = arr;
  int status = semctl(semid, 0, GETALL, getAllArg);  // 0 doesn't matter

  // check if semctl was successful or not
  if (status == -1) {
    perror("semctl() failed");
    exit(1);
  }

  return arr[idx];
}

/**
 * @brief terminate the program by releasing the semaphore set coressponding to
 * the semid
 *
 * @param signum
 */
void terminateProgram(int signum) {
  printf("\n");

  semOperation(SEM_SET_SIZE - 1, -1, IPC_NOWAIT,
               true);  // decrease the last semaphore by one to signify that
                       // this process will no longer use the semaphore
  print_sem_set();

  int status;
  if (getSem(SEM_SET_SIZE - 1) ==
      0)  // the last semaphore will be zero iff no other processes are using
          // it, if that's the case, we delete the semaphore
  {
    status = semctl(semid, 0, IPC_RMID);

    if (status == 0)
      fprintf(stderr, "Removed semaphore set with id = %d.\n", semid);
    else if (status == -1)
      fprintf(stderr, "Cannot remove semaphore set with id = %d.\n", semid);
    else
      fprintf(stderr,
              "semctl() returned wrong value while removing semaphore set with "
              "id = %d.\n",
              semid);
  }

  // now we have freed the semaphore set, now we kill the program itself
  status = kill(0, SIGKILL);

  if (status == 0)
    fprintf(stderr,
            "kill susccesful.\n"); /* this line may not be executed :P WHY?*/
  else if (status == -1)
    perror("kill failed.\n");
  else
    fprintf(stderr, "kill(2) returned wrong value.\n");
}

void log_time() {
  time_t rawtime;
  time(&rawtime);
  printf("Time: %s\n", ctime(&rawtime));
}

int main() {
  sighandler_t shandler = signal(SIGINT, terminateProgram);
  key_t mykey;
  pid_t pid;

  int status;

  setValArg.val = 1;

  mykey = ftok("./", 1);

  if (mykey == -1) {
    perror("ftok() failed");
    exit(1);
  }

  //       int semget(key_t key, int nsems, int semflg);
  semid = semget(mykey, SEM_SET_SIZE, IPC_CREAT | 0777);
  if (semid == -1) {
    perror("semget() failed");
    exit(1);
  }

  setValArg.val = 1;
  semctl(semid, 0, SETVAL, setValArg);  // set one in 0th index to set the value
                                        // of semaphore for actual task

  setValArg.val = 0;
  semctl(semid, 1, SETVAL,
         setValArg);  // set zero in 1st index to set the value
                      // of semaphore for the other tasks
  print_sem_set();
  while (true) {
    // 1, 0
    semOperation(1, -1, SEM_UNDO, false);
    printf("I am consumer\n");
    log_time();
    semOperation(0, 1, SEM_UNDO, false);
    print_sem_set();
    sleep(2);
  }
}
