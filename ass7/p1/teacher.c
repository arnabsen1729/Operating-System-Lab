#include <errno.h>  /* for perror */
#include <signal.h> /* for signal(2) kill(2) */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> /* for shmget() shmctl() */
#include <sys/sem.h>
#include <sys/shm.h>   /* for shmget() shmctl() */
#include <sys/types.h> /* for wait() kill(2)*/
#include <sys/wait.h>  /* for wait() */
#include <time.h>
#include <unistd.h> /* for fork() */

#define SHARED_FILE "./file.txt"
#define MAX_ATTENDANCE 100
#define P(s) semop(s, &Pop, 1);  // wait operation
#define V(s) semop(s, &Vop, 1);  // signal operation
struct sembuf Pop;               // wait operation flags
struct sembuf Vop;               // signal operation flags
#define NO_SEM 1                 // we need only 1 semaphore

typedef void (*sighandler_t)(int);  // define the sighandler
int semid = -1;                     // global semaphore id

union semun {
  int val;               /* Value for SETVAL */
  struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
  unsigned short *array; /* Array for GETALL, SETALL */
  struct seminfo *__buf; /* Buffer for IPC_INFO (Linux-specific) */
};

typedef struct att {
  int roll; /* The roll number of the student giving the attendance */

  time_t
      tm; /* To keep the time when attendance was given by the student. Read the
             manual for time(), that is, "man 2 time" Also see the manual
             entries mentioned in the "see also" section of this manual page! */
} att;

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
    printf("initialising semaphore...\n");
    union semun setValArg;
    setValArg.val = 1;
    if (semctl(semid, 0, SETVAL, setValArg) == -1) {
      perror("semctl() in sem_init() :");
      exit(1);
    }
  } else {
    printf("semaphores already initialised\n");
  }
}

void sem_log(int semid) { printf("sem = %d\n", semctl(semid, 0, GETVAL)); }

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

void releaseSHMandSEM(int signum) {
  displayAttendance();

  int status = semctl(
      semid, 0,
      IPC_RMID); /* IPC_RMID is the command for destroying the semaphore*/
  if (status == 0) {
    fprintf(stderr, "\nRemoved semaphore with id = %d.\n", semid);
  } else if (status == -1) {
    fprintf(stderr, "Cannot remove semaphore with id = %d.\n", semid);
  } else {
    fprintf(stderr,
            "shmctl() returned wrong value while removing semaphore with id"
            "= %d.\n",
            semid);
  }
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
  shandler = signal(SIGINT, releaseSHMandSEM);
  key_t mykey = Ftok(SHARED_FILE, 1);

  init();
  semid = sem_create(mykey, NO_SEM);
  sem_init(semid);
  sem_log(semid);

  shmid = getSHM(mykey, sizeof(att));

  att *attendance = (att *)getPtrSHM();

  attendance->roll = -1;

  while (true) {
    P(semid);
    if (attendance->roll != -1) {
      if (checkIfRollPresent(roll_numbers, student_count, attendance->roll)) {
        printf("Attendance given by: \n");
        printf("Roll number: %d\n", attendance->roll);
        printf("Time: %s\n", ctime(&attendance->tm));

        student_attendance[student_attendance_count++] = *attendance;
      }
      attendance->roll = -1;
    }
    V(semid);
  }

  return 0;
}
