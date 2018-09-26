#include <stdlib.h>
#include <string.h>

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
