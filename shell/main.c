#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 2048

void _exit() { exit(0); }

void environment() {}

void path() {}

int CMP_TABLE_LEN = 3;
char **cmpTable = {"exit", "environment", "path"};
void (*jmpTable[])() = {_exit, environment, path};

void handleInput(char **input) {
  // Check to see if the entered string is in the special input table
  for (int i = 0; i < CMP_TABLE_LEN; i++) {
    if (strncmp(cmpTable[i], input, strlen(cmpTable[i])) == 0) {
      return jmpTable[i]();
    }
  }

  // No match in built-ins, so we have to process the command as a program
  // TODO
}

int main(int argc, char *argv[]) {
  char buffer[BUFFER_SIZE];
  while (1) {
    puts("» ");
  }
}
