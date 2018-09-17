#include "program.c"

int main(int argc, char **argv) {
  char **output = driver(argc, argv);
  if (output == NULL) {
    return 1;
  }

  int i = 0;
  while (output[i] != NULL) {
    puts(output[i]);
    i += 1;
  }

  return 0;
}
