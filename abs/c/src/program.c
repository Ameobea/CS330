#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

char *USAGE = "usage: ./abs [-s] [-r] [-m] [-i] [-u] [-l] [-t] [-h]";

char **getOSName() {
  // puts("1");
  char **output = (char **)malloc(2 * sizeof(char *));
  output[1] = NULL;
  return output;
}

char **getSystemRelease() {
  // puts("2");
  char **output = (char **)malloc(2 * sizeof(char *));
  output[1] = NULL;
  return output;
}

char **getMachineType() {
  // puts("3");
  char **output = (char **)malloc(2 * sizeof(char *));
  output[1] = NULL;
  return output;
}

char **getPID() {
  // puts("4");
  pid_t curPID = getpid();
  pid_t parentPID = getppid();
  const int maxPIDLength = 6;
  char *buf = (char *)malloc((2 * maxPIDLength + 13 + 14 + 1) * sizeof(char));
  sprintf(buf, "Current PID: %i, Parent PID: %i", curPID, parentPID);
  char **output = (char **)malloc(2 * sizeof(char *));
  output[0] = buf;
  output[1] = NULL;
  return output;
}

char **getUserInfo() {
  // puts("5");
  char **output = (char **)malloc(2 * sizeof(char *));
  output[1] = NULL;
  return output;
}

char **getCurrentShell() {
  // puts("6");
  char **output = (char **)malloc(2 * sizeof(char *));
  output[1] = NULL;
  return output;
}

char **getCurDateTime() {
  // puts("7");
  char **output = (char **)malloc(2 * sizeof(char *));
  output[1] = NULL;
  return output;
}

char **getHelp() {
  // puts("8");
  char **output = (char **)malloc(2 * sizeof(char *));
  output[0] = USAGE;
  output[1] = NULL;
  return output;
}

char **getInvalidFlag(char invalidFlag) {
  char *msgBuf1 = (char *)malloc(33 * sizeof(char));
  sprintf(msgBuf1, "Error: Invalid flag `%c` supplied.", invalidFlag);

  char **output = (char **)malloc(3 * sizeof(char *));
  output[0] = msgBuf1;
  output[1] = getHelp()[0];
  output[2] = NULL;
  return output;
}

char **invalidArguments(char *invalidArg) {
  char *buf = (char *)malloc((34 + (int)strlen(invalidArg) + 1) * sizeof(char));
  sprintf(buf, "Error: invalid argument provided: %s", invalidArg);
  char **output = (char **)malloc(2 * sizeof(char *));
  output[0] = buf;
  output[1] = NULL;
  return output;
}

const int FLAG_COUNT = 8;

char **driver(int argc, char *argv[]) {
  printf("argc %i\n", argc);
  for (int i = 0; i < argc; ++i) {
    printf("argv[ %d ] = %s (%p)\n", i, argv[i], argv[i]);
  }

  if (argc == 1) {
    printf("No arguments supplied; not yet handled.\n");
    return NULL;
  }

  char *flagIndices = "srmiulth";
  for (int argIndex = 1; argIndex < argc + 1; argIndex++) {
    if (argv[argIndex][0] != '-' || argv[argIndex][1] == 0 ||
        argv[argIndex][2] != 0) {
      return invalidArguments(argv[argIndex]);
    }

    char flag = argv[argIndex][1];
    printf("Flag: %c\nflagIndices: %s\n", flag, flagIndices);

    // This is C, even though it's running in the web browser.  We do low-level
    // hackery here.
    char **(*jumptable[])() = {
        getOSName,   getSystemRelease, getMachineType, getPID,
        getUserInfo, getCurrentShell,  getCurDateTime, getHelp};

    printf("Matching flag %c\n", flag);
    for (int i = 0; i < FLAG_COUNT; i++) {
      if (flag == flagIndices[i]) {
        return jumptable[i]();
      }
    }
  }

  return NULL;
}
