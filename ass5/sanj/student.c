#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <time.h>

int shmid;

typedef struct att {
  int roll;
  time_t time;
} att;

const int PROJECT_ID = 1;
const char* COMMON_FILE = "./file.txt";

void releaseSHM(int shmid) {
  int status = shmctl(shmid, IPC_RMID, NULL);
  // int shmctl(int shmid, int cmd, struct shmid_ds *buf);
  // control the SHM held by shmid
  // what to do is defined by the cmd
  // here IPC_RMID is a signal to free the SHM
  // in ICP_RMID mode -> return 0 if success, -1 on error

  if (status == 0)
    fprintf(stderr, "Remove shared memory with id = %d.\n", shmid);
  else if (status == -1)
    fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
  else
    fprintf(stderr,
            "shmctl() returned wrong value while removing shared memory with "
            "id = %d.\n",
            shmid);

  // now we have freed the SHM, now we kill the program itself
  status = kill(0, SIGKILL);
  // int kill(pid_t pid, int sig);
  // pid = 0 -> send sig all the processes in the group (also includes the
  // child) returns 0 if success, -1 on error

  if (status == 0)
    fprintf(stderr,
            "kill susccesful.\n"); /* this line may not be executed :P WHY?*/
  else if (status == -1) {
    perror("kill failed.\n");
    fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
  } else
    fprintf(stderr, "kill(2) returned wrong value.\n");
}

void* getPtrSHM() {
  void* value;
  value = shmat(shmid, NULL, 0);
  if (value == (void*)-1) { /* shmat fails */
    perror("shmat() failed");
    exit(1);
  }
  return value;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Error: Please keep just 1 argument\n");
    return 0;
  }

  int roll = atoi(argv[1]);
  time_t curr_time = time(NULL);
  int shmkey = ftok(COMMON_FILE, 1);
  shmid = shmget(shmkey, sizeof(att), IPC_CREAT | 0777);

  att* shared_att = (att*)getPtrSHM();

  while (1) {
    if (shared_att->roll != -1) {
      continue;
    }
    shared_att->roll = roll;
    shared_att->time = curr_time;
    break;
  }
  return 0;
}
