#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <time.h>

typedef struct att {
  int roll;
  time_t time;
} att;

typedef void (*sighandler_t)(int);  // define the sighandler

char* FILENAME;
const char* COMMON_FILE = "./file.txt";
const int PROJECT_ID = 1;
int shmid;
att* attendance_list;

void releaseSHM(int signum) {
  // printChronologicalOrder(); // prints the attendace array in chronological
  // order

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
    fprintf(stderr, "kill susccesful.\n");
  else if (status == -1) {
    perror("kill failed.\n");
    fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
  } else
    fprintf(stderr, "kill(2) returned wrong value.\n");
}

int count_students() {
  FILE* fp = fopen(FILENAME, "r");
  if (!fp) {
    printf("Error: File could not be opened\n");
    return -1;
  }
  int count = 0;
  char c;
  for (c = getc(fp); c != EOF; c = getc(fp)) {
    if (c == '\n') count++;
  }
  fclose(fp);
  return count;
}

int* read_rolls(int n) {
  FILE* fp = fopen(FILENAME, "r");
  if (!fp) {
    printf("Error: File could not be opened\n");
    return NULL;
  }
  int* rolls = (int*)malloc(sizeof(int) * n);
  for (int i = 0; i < n; i++) {
    fscanf(fp, "%d", &rolls[i]);
  }
  fclose(fp);
  return rolls;
}

void display_roll_list(int* roll_list, int n) {
  for (int i = 0; i < n; i++) {
    printf("%d\n", roll_list[i]);
  }
}

void initialize_att(att* attendance, int n) {
  for (int i = 0; i < n; i++) {
    attendance[i].roll = -1;
    attendance[i].time = 0;
  }
}

int find_roll(int* roll_list, int n, int roll) {
  for (int i = 0; i < n; i++) {
    if (roll_list[i] == roll) {
      return i;
    }
  }
  return -1;
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
  int prev = -1;
  if (argc != 2) {
    printf("Error: No filename provided\n");
    return 0;
  }
  FILENAME = argv[1];
  int student_count = count_students();
  att* attendance = (att*)malloc(sizeof(att) * student_count);
  initialize_att(attendance, student_count);
  int* roll_list = read_rolls(student_count);
  // display_roll_list(roll_list, student_count);

  sighandler_t shandler = signal(SIGINT, releaseSHM);

  int shmkey = ftok(COMMON_FILE, 1);
  shmid = shmget(shmkey, sizeof(att), IPC_CREAT | 0777);

  att* shared_att = (att*)getPtrSHM();
  shared_att->roll = -1;

  int index = 0, idx;

  while (1) {
    if (shared_att->roll != -1) {
      //   idx = find_roll(roll_list, student_count, shared_att->roll);
      //   if (idx != -1) {
      //   roll_list[idx] = -2;  // attendance has been marked for this student
      //   attendance[index].roll = shared_att->roll;
      //   attendance[index].time = shared_att->time;
      printf("Roll no: %d, Time: %s\n", shared_att->roll,
             ctime(&(shared_att->time)));
      index++;
      // }
      shared_att->roll = -1;
      shared_att->time = 0;
    }
  }

  return 0;
}
