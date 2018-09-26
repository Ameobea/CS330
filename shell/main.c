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

#include "linevec.h"

#define BUFFER_SIZE 2048
#define SHELL_NAME "cssh"
#define false 0
#define true 1

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
 * Checks if a given directory entry matches the provided command and can be
 * executed by the user.  Returns the full path of the program if it matches and
 * `NULL` otherwise.
 */
char *checkDirEntry(struct dirent *dirEntry, char *dirPath, char *cmd) {
  // check if the filename matches
  if (strcmp(cmd, dirEntry->d_name) != 0) {
    return NULL;
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
    fprintf(stderr, "Error retrieving permissions for file: %s\n", fullPath);
    free(fullPath);
    return NULL;
  }

  // check if it's readable + executable
  // TODO: check if current UID has perms as well
  if ((statusBuffer.st_mode & S_IXOTH) == 0) {
    // Not executable; have to skip it.
    free(fullPath);
    return NULL;
  }

  // We've found it!  Return the full path
  return fullPath;
}

/**
 * Checks all files (nonrecursively) in a given directory to see if any match
 * the provided command are able to be executed.  If one is found, its full file
 * path is returned; otherwise `NULL` is returned.
 */
char *traverseDir(char *dirPath, char *cmd) {
  DIR *pathDir = opendir(dirPath);
  if (pathDir == NULL) {
    return NULL;
  }

  struct dirent *dirEntry;
  dirEntry = readdir(pathDir);
  while (dirEntry != NULL) {
    char *programPath = checkDirEntry(dirEntry, dirPath, cmd);
    if (programPath != NULL) {
      return programPath;
    }
    dirEntry = readdir(pathDir);
  }

  // No match in the current directory
  return NULL;
}

/**
 * Searches the `PATH` for the given program, returning its path if it is found
 * and `NULL` otherwise.
 */
char *lookupPath(char *cmd) {
  // Search all of the directories in the path one at a time for filenames
  // matching `cmd`, returning the first one that is found.
  char *pathSrc = getenv("PATH");

  // We have to copy the `PATH` string to a new buffer because `strtok`
  // overwrites it, meaning that it can't be re-used later on in the program.
  char *path = (char *)malloc(strlen(pathSrc) * sizeof(char));
  strcpy(path, pathSrc);

  char *dirPath = strtok(path, ":");
  while (dirPath != NULL) {
    char *programPath = traverseDir(dirPath, cmd);
    if (programPath != NULL) {
      return programPath;
    }

    dirPath = strtok(NULL, ":");
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
