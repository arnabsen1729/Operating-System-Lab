/**
 * @file main.c
 * @author Arnab Sen (arnabsen1729@gmail.com)
 * @brief
 *
 * Write a complete C program that reads a +ve integer (say, n) number as a
 * command line argument. That is, uses "int argc" and "char *argv[]" to read n
 * when the program is executed as "./a.out n". The program then creates n child
 * processes P1, P2, ..., Pn such that Pi, 1 ≤  i ≤  n, computes and prints the
 * ith prime number. For example, "./a.out 5" would create 5 processes P1, P2,
 * ..., P5, such that, P1 prints 1, P2 prints 2, P3 prints 3, P4 prints 5 and P5
 * prints 7.
 *
 * @date 2022-01-21
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief function to check if a number passed in the param is a prime or not
 *
 * @param n the number to check
 * @return bool true if prime, false otherwise
 */
bool isPrime(int n) {
  for (int i = 2; i * i <= n; i++) {
    // if n is divisible by i, then it is not a prime number
    // cause prime numbers are divisible by 1 and itself only
    if (n % i == 0) return false;
  }
  return true;
}

/**
 * @brief returns the n-th prime number
 *
 * @param n
 * @return int
 */
int nthPrime(int n) {
  int number = 0;
  // iterate till n-th prime number is found
  while (n) {
    number++;
    // if number is prime, decrement n and continue
    if (isPrime(number)) n--;
  }
  // return the number
  return number;
}

/**
 * @brief The main function
 *
 * @param argc
 * @param argv
 * @return int 0 if success, 1 otherwise
 */
int main(int argc, char const *argv[]) {
  if (argc < 2) {
    // no argument passed
    printf("Argument needed !\n\nUSAGE: %s n\nn : any positive integer\n",
           argv[0]);
    return 1;
  }

  int n = atoi(argv[1]);  // convert string to int (header: stdlib.h)

  if (n <= 0) {
    // incase n is not positive
    printf(
        "Argument must be a positive integer !\n\nUSAGE: %s n\nn : any "
        "positive "
        "integer\n",
        argv[0]);
    return 1;
  }

  // fork n times to create n child processes
  for (int i = 0; i < n; i++) {
    pid_t pid = fork();  // create a child process (header: unistd.h)

    if (pid == 0) {
      // child process
      printf("%d-th prime number is: %d\n", i + 1, nthPrime(i + 1));
      return 0;
    }
  }
  return 0;
}
