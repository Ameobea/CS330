#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

#define BUFFER_SIZE 2048
#define SHELL_NAME "cssh"

/**
 * Dynamic vector implementation since this is C and we're living in the stone
 * age
 */
struct linevector {
  int size;
  int len;
  char **buf;
};

int LINEVECTOR_DEFAULT_SIZE = 1;
int LINEVECTOR_GROWBY = 8;

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

void linevectorFree(struct linevector *vec) {
  int i = 0;
  while (vec->buf[i] != 0) {
    free(vec->buf[i]);
    i += 1;
  }
  free(vec->buf);
}

void quit() { exit(0); }

void environment() {
  int i = 0;
  while (environ[i] != NULL) {
    printf("%s\n", environ[i]);
    i += 1;
  }
}

void path() { printf("%s\n", getenv("PATH")); }

int CMP_TABLE_LEN = 3;
const char *cmpTable[] = {"exit", "environment", "path"};
void (*jmpTable[])() = {quit, environment, path};

/**
 * Searches the `PATH` for the given program, returning its path if it is found
 * and `NULL` otherwise.
 */
char *lookupPath(char *cmd) {
  // Search all of the directories in the path one at a time for filenames
  // matching `cmd`, returning the first one that is found.
  char *path = getenv("PATH");
  char *dirPath = strtok(path, ":");
  DIR *pathDir;
  while (dirPath != NULL) {
    pathDir = opendir(dirPath);
    if (!pathDir) {
      dirPath = strtok(NULL, ":");
      continue;
    }

    struct dirent *dirEntry;
    dirEntry = readdir(pathDir);
    while (dirEntry != NULL) {
      // check if the filename matches
      if (strcmp(cmd, dirEntry->d_name) != 0) {
        dirEntry = readdir(pathDir);
        continue;
      }
      // check if the file is a regular file
      // TODO: support symlinks
      // if (dirEntry->d_type != DT_REG) {
      //   printf("ASDF - nonmatching file");
      //   dirEntry = readdir(pathDir);
      //   continue;
      // }

      // Construct the full path to the file and retrieve permissions
      char *fullPath = (char *)malloc(
          (strlen(dirPath) + strlen(dirEntry->d_name) + 1) * sizeof(char));
      sprintf(fullPath, "%s/%s", dirPath, dirEntry->d_name);
      struct stat statusBuffer;
      int fileStatus = stat(fullPath, &statusBuffer);
      if (fileStatus == -1) {
        fprintf(stderr, "Error retrieving permissions for file: %s\n",
                fullPath);
        free(fullPath);
        dirEntry = readdir(pathDir);
        continue;
      }

      // check if it's readable + executable
      // TODO: check if current UID has perms as well
      if (statusBuffer.st_mode & S_IXOTH == 0) {
        // Not executable; have to skip it.
        free(fullPath);
        dirEntry = readdir(pathDir); // TODO: optimize this
        continue;
      }

      // We've found it!  Return the full path
      return fullPath;
    }

    dirPath = strtok(NULL, ":");
    continue;
  }

  // No matches
  return NULL;
}

void call(char *cmd, struct linevector *args) {
  char *programPath = lookupPath(cmd);
  if (programPath == NULL) {
    fprintf(stderr, "%s: command not found: %s\n", SHELL_NAME, cmd);
    return;
  }

  pid_t pid;
  int childStatus;

  if ((pid = fork()) == 0) {
    // Destroy current process, replacing it with the target program
    // printf("ARGS: ");
    // for (int i = 0; i < args->len; i++) {
    //   printf("%s,", args->buf[i]);
    // }
    // printf("\n");
    int status = execve(programPath, args->buf, NULL);

    // We should only reach here if we failed to swap over to the target program
    if (status == -1) {
      fprintf(stderr, "Error executing child process!");
      return;
    }
  } else {
    // Wait for child to complete
    pid = wait(&childStatus);
    if (pid == -1) {
      fprintf(stderr,
              "Error while retrieving the exit status of child process!");
      return;
    }

    printf("Child process with PID %i exited with status %i.\n", pid,
           WEXITSTATUS(childStatus));
  }
}

void handleInput(char *input) {
  char whitespace[] = " \t\n\r";
  char *cmd = strtok(input, whitespace);

  // Check to see if the entered string is in the special input table
  for (int i = 0; i < CMP_TABLE_LEN; i++) {
    if (strncmp(cmpTable[i], cmd, strlen(cmpTable[i])) == 0) {
      return jmpTable[i]();
    }
  }

  // No match in built-ins, so we have to process the command as a program
  char *substr = strtok(NULL, whitespace);
  struct linevector args;
  linevectorInit(&args);
  // Push the command name
  linevectorPush(&args, cmd);
  while (substr != NULL) {
    linevectorPush(&args, substr);
    substr = strtok(NULL, whitespace);
  }
  linevectorPush(&args, NULL);

  call(cmd, &args);

  // Don't need `linevectorFree` since the inner strings aren't dynamically
  // allocated
  free(args.buf);
}

int main(int argc, char *argv[]) {
  char buffer[BUFFER_SIZE];

  while (1) {
    memset(buffer, 0, BUFFER_SIZE);
    printf("» ");

    if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
      handleInput(buffer);
    } else {
      fprintf(stderr, "Error reading input!");
    }
  }
}
