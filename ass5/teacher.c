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
#define MAX_ATTENDANCE 100

typedef void (*sighandler_t)(int);  // define the sighandler_t type

typedef struct att {
  int roll; /* The roll number of the student giving the attendance */

  time_t
      tm; /* To keep the time when attendance was given by the student. Read the
             manual for time(), that is, "man 2 time" Also see the manual
             entries mentioned in the "see also" section of this manual page! */
} att;

// --------- SHARED MEMORY RELATED FUNCTIONS --------------

int shmid;
att student_attendance[MAX_ATTENDANCE];
int student_attendance_count = 0;

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
void *getPtrSHM() {
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

void displayAttendance() {
  printf("\n\nAttendance of students:\n");
  for (int i = 0; i < student_attendance_count; i++) {
    printf("Roll: %d Time: %s\n", student_attendance[i].roll,
           ctime(&student_attendance[i].tm));
  }
}

void releaseSHM(int signum) {
  displayAttendance();
  destroySHM(shmid);
}

int countLines(FILE *fp) {
  char ch;
  int lines = 0;
  while (!feof(fp)) {
    ch = fgetc(fp);
    if (ch == '\n') {
      lines++;
    }
  }
  return lines;
}

void readFile(FILE *fp, int *arr) {
  fseek(fp, 0, SEEK_SET);
  int i = 0;
  while (!feof(fp)) {
    fscanf(fp, "%d\n", &arr[i]);
    i++;
  }
}

bool checkIfRollPresent(int *arr, int n, int roll) {
  for (int i = 0; i < n; i++) {
    if (arr[i] == roll) {
      return true;
    }
  }
  return false;
}

int main(int argc, const char *argv[]) {
  if (argc == 1 || argc > 2) {
    printf("Usage: ./teacher.out <roll_number_file>\n");
    exit(1);
  }

  // ---------- FILE HANDLING ------------
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("fopen() failed");
    exit(1);
  }
  int student_count = countLines(fp);
  int *roll_numbers = (int *)malloc(sizeof(int) * student_count);
  readFile(fp, roll_numbers);

  sighandler_t shandler;
  shandler = signal(SIGINT, releaseSHM);

  shmid = getSHM(sizeof(att));
  att *attendance = (att *)getPtrSHM();

  attendance->roll = -1;

  while (true) {
    if (attendance->roll != -1) {
      if (checkIfRollPresent(roll_numbers, student_count, attendance->roll)) {
        printf("Attendance given by: \n");
        printf("Roll number: %d\n", attendance->roll);
        printf("Time: %s\n", ctime(&attendance->tm));

        student_attendance[student_attendance_count++] = *attendance;
      }
      attendance->roll = -1;
    }
  }

  return 0;
}
