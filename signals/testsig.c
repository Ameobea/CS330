/*  CS 330 Program to Illustrate set_sig().
 *  Press control-C three times.
 *
 *  Modified by Casey Primozic
 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* Forward declarations */
static void set_sig(int signo, void (*)(int));
static void ctrl_c_handler(int signo);
static void gotcha_handler(int signo);
static void sigterm_handler(int signo);
static void sigusr_handler(int signo);
static void sigquit_handler(int signo);

/* Main: Set up ctrl-c signal handler, wait forever */
int main() {
  printf("\nType Ctrl+\ to exit.\n");
  set_sig(SIGINT, ctrl_c_handler);
  set_sig(SIGILL, gotcha_handler);
  set_sig(5, gotcha_handler);
  set_sig(8, gotcha_handler);
  set_sig(SIGTERM, sigterm_handler);
  set_sig(SIGUSR1, sigusr_handler);
  set_sig(SIGUSR2, sigusr_handler);
  set_sig(SIGQUIT, sigquit_handler);
  while (1)
    ;
}

/* Field ctrl-c signals */
void ctrl_c_handler(int signo) {
  pid_t pid;
  printf("PID to kill: ");
  if (scanf("%d", &pid) <= 0) {
    printf("\nInvalid PID entered.");
    return ctrl_c_handler(signo);
  }

  kill(pid, SIGINT);
  printf("Process sent SIGINT.\n");
}

void sigquit_handler(int signo) { exit(0); }

void sigusr_handler(int signo) { printf("user signal received"); }

/* Refuse to be SIGTERMed.  Bwa ha ha! */
void sigterm_handler(int signo) { printf("I won't be terminated.\n"); }

/* Gotcha! */
void gotcha_handler(int signo) {
  printf("You can't fool me!  That was signal number %d\n", signo);
}

/* Install a signal handler using sigaction() */
void set_sig(int signo, void (*handler)(int)) {
  struct sigaction act;

  act.sa_handler = handler;
  act.sa_flags = SA_RESTART;
  sigemptyset(&act.sa_mask);

  if (sigaction(signo, &act, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
}
