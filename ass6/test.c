#include <signal.h>
#include <stdio.h>
#include <stdlib.h>    /* for exit(3) */
#include <sys/ipc.h>   /* for semget(2) ftok(3) semop(2) semctl(2) */
#include <sys/sem.h>   /* for semget(2) semop(2) semctl(2) */
#include <sys/types.h> /* for semget(2) ftok(3) semop(2) semctl(2) */
#include <unistd.h>    /* for fork(2) */

#define NO_SEM 1

#define P(s) semop(s, &Pop, 1);
#define V(s) semop(s, &Vop, 1);

#define MAX_PROCESSES 3

struct sembuf Pop;
struct sembuf Vop;
int semid;

void release_sem(int signum) {
  V(semid);
  int status;
  int v = semctl(semid, 0, GETVAL);
  if (v == MAX_PROCESSES) {
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
            "kill susccesful.\n"); /* this line may not be executed :P WHY?*/
  } else if (status == -1) {
    perror("kill failed.\n");
    fprintf(stderr, "Cannot remove semaphore with id = %d.\n", semid);
  } else {
    fprintf(stderr, "kill(2) returned wrong value.\n");
  }
}
int main(int argc, char *argv[]) {
  signal(SIGINT, release_sem);
  key_t mykey;
  pid_t pid;

  int status;

  union semun {
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO (Linux-specific) */
  } setvalArg;

  /* struct sembuf has the following fields */
  // unsigned short sem_num;  /* semaphore number */
  // short          sem_op;   /* semaphore operation */
  // short          sem_flg;  /* operation flags */

  Pop.sem_num = 0;
  Pop.sem_op = -1;
  Pop.sem_flg = SEM_UNDO;

  Vop.sem_num = 0;
  Vop.sem_op = 1; /* semaphore value is incremented by only
                                  1 thus can't perform P(s) operation  and have
                     to wait for consumer */
  Vop.sem_flg = SEM_UNDO;

  // key_t ftok(const char *pathname, int proj_id);
  mykey = ftok(
      "/mnt/d/souvik mahato/college study/3rdyear/os/lab/a6_final/student.c",
      1);

  if (mykey == -1) {
    perror("ftok() failed");
    exit(1);
  }

  //       int semget(key_t key, int nsems, int semflg);
  semid = semget(mykey, NO_SEM, IPC_CREAT | 0777);
  if (semid == -1) {
    perror("semget() failed");
    exit(1);
  }

  // int semctl(int semid, int semnum, int cmd, ...);
  pid_t p = semctl(semid, 0, GETPID);
  if (p <= 0) {
    setvalArg.val = MAX_PROCESSES;
    status = semctl(semid, 0, SETVAL, setvalArg);
    if (status == -1) {
      perror("semctl() failed");
      exit(1);
    }
  }

  P(semid);
  int v = semctl(semid, 0, GETVAL);
  printf("semval = %d\n", v);
  int i = 0;
  while (1) {
    fprintf(stdout, "%d : I am student %d!\n\n", i++, atoi(argv[1]));
    sleep(1);
  }
  return 0;
}
