#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>

/**
 * Primitive vector that contains a set of strings and can be grown dynamically.
 *
 * All strings pushed into this vector must be dynamically allocated; they will
 * be `free`'d once the vector is released.
 **/
struct linevector {
  int size;
  int len;
  char **buf;
};

const int LINEVECTOR_DEFAULT_SIZE = 1;
const int LINEVECTOR_GROWBY = 8;

void linevectorInit(struct linevector *vec) {
  vec->size = LINEVECTOR_DEFAULT_SIZE;
  vec->len = 0;
  vec->buf = (char **)malloc(LINEVECTOR_DEFAULT_SIZE * sizeof(char *));
}

/**
 * Allocate a new buffer, memcpy the old buffer to it, and free the old buffer.
 */
void linevectorGrow(struct linevector *vec) {
  char **newBuf =
      (char **)malloc((vec->size + LINEVECTOR_GROWBY) * sizeof(char *));
  memcpy(newBuf, vec->buf, vec->len * sizeof(char *));
  free(vec->buf);
  vec->buf = newBuf;
}

void linevectorPush(struct linevector *vec, char *line) {
  // Grow the vector if it's currently full
  if (vec->size == vec->len) {
    linevectorGrow(vec);
  }

  vec->buf[vec->len] = line;
  vec->len += 1;
}

/**
 * Takes a statically allocated string and returns a dynamically-allocated
 * copy of it.
 */
char *dynAlloc(char *staticString) {
  size_t len = strlen(staticString);
  char *dynBuf = (char *)malloc(len * sizeof(char));
  strcpy(dynBuf, staticString);
  return dynBuf;
}

char *USAGE = "usage: ./abs [-s] [-r] [-m] [-i] [-u] [-l] [-t] [-h]";

void getOSName(struct linevector *lines) {
  struct utsname unameData;
  uname(&unameData);
  char *buf =
      (char *)malloc((strlen(unameData.sysname) + 3 + 1) * sizeof(char));
  sprintf(buf, "OS %s", unameData.sysname);
  linevectorPush(lines, buf);
}

void getSystemRelease(struct linevector *lines) {
  struct utsname unameData;
  uname(&unameData);
  puts(unameData.release);
  char *buf =
      (char *)malloc((strlen(unameData.release) + 8 + 1) * sizeof(char));
  sprintf(buf, "Release %s", unameData.release);
  linevectorPush(lines, buf);
}

void getMachineType(struct linevector *lines) {
  struct utsname unameData;
  uname(&unameData);
  char *buf =
      (char *)malloc((strlen(unameData.machine) + 8 + 1) * sizeof(char));
  sprintf(buf, "Machine %s", unameData.machine);
  linevectorPush(lines, buf);
}

void getPID(struct linevector *lines) {
  pid_t curPID = getpid();
  pid_t parentPID = getppid();
  const int maxPIDLength = 6;
  char *buf = (char *)malloc((2 * maxPIDLength + 4 + 7 + 1) * sizeof(char));
  sprintf(buf, "PID=%i, PPID=%i", curPID, parentPID);
  linevectorPush(lines, buf);
}

void getUserInfo(struct linevector *lines) {
  // TODO
}

void getCurrentShell(struct linevector *lines) {
  char *shell = getenv("SHELL");
  size_t shellNameLen = strlen(shell);
  if (shell == NULL) {
    shellNameLen = strlen("(null)");
  }

  char *buf = (char *)malloc((shellNameLen + 8 + 1) * sizeof(char));
  sprintf(buf, "Shell = %s", shell);
  linevectorPush(lines, buf);
}

void getCurDateTime(struct linevector *lines) {
  time_t tod_or_error;
  time_t todmem;

  tod_or_error = time(&todmem);
  if (tod_or_error == -1) {
    perror("There was an error fetching the current time of day.");
    return;
  }

  char *timestring = ctime(&tod_or_error);
  char *buf = (char *)malloc((strlen(timestring) + 7 + 1) * sizeof(char));
  sprintf(buf, "Time = %s", timestring);
  linevectorPush(lines, buf);
}

void getHelp(struct linevector *lines) {
  linevectorPush(lines, dynAlloc(USAGE));
}

void invalidArgument(struct linevector *lines, char *invalidArg) {
  char *buf = (char *)malloc((34 + (int)strlen(invalidArg) + 1) * sizeof(char));
  sprintf(buf, "Error: invalid argument provided: %s", invalidArg);
  linevectorPush(lines, buf);
  getHelp(lines);
}

const int FLAG_COUNT = 8;

char **driver(int argc, char *argv[]) {
  printf("argc %i\n", argc);
  for (int i = 0; i < argc; ++i) {
    printf("argv[ %d ] = %s (%p)\n", i, argv[i], argv[i]);
  }

  // This is C, even though it's running in the web browser.  We do low-level
  // hackery here.
  void (*jumptable[])(struct linevector *) = {
      getOSName,   getSystemRelease, getMachineType, getPID,
      getUserInfo, getCurrentShell,  getCurDateTime, getHelp};

  struct linevector lines;
  linevectorInit(&lines);
  if (argc == 1) {
    // If no arguments supplied, print everything
    for (int i = 0; i < 7; i++) {
      jumptable[i](&lines);
    }
  }

  char *flagIndices = "srmiulth";
  for (int argIndex = 1; argIndex < argc; argIndex++) {
    if (argv[argIndex][0] != '-' || argv[argIndex][1] == 0 ||
        argv[argIndex][2] != 0) {
      invalidArgument(&lines, argv[argIndex]);
      break;
    }

    char flag = argv[argIndex][1];
    printf("Flag: %c\nflagIndices: %s\n", flag, flagIndices);

    printf("Matching flag %c\n", flag);
    for (int i = 0; i < FLAG_COUNT; i++) {
      if (flag == flagIndices[i]) {
        jumptable[i](&lines);
      }
    }
  }

  // Add a final `NULL` into the linevec to indicate the end of the lines
  linevectorPush(&lines, NULL);
  return lines.buf;
}
