#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *func(void *ptr) {
  char *msg;
  msg = (char *)ptr;
  printf("%s\n", msg);
}

int main(){
  pthread_t thread1, thread2;

  char *message1 = "Thread 1";
  char *message2 = "Thread 2";

  int iret1, iret2;

  iret1 = pthread_create(&thread1, NULL, func, (void *) message1);
  iret2 = pthread_create(&thread2, NULL, func, (void *) message2);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  exit(0);
}
