#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdbool.h>
#include <config.h>

typedef enum {HORIZ, VERT} orientation;
typedef enum {EAST, WEST, NORTH, SOUTH} direction;

typedef struct coord {
	unsigned int x : 8;
	unsigned int y : 8;
} coord_t;

bool coord_set_equal(coord_t *A, coord_t *B, int num_bits) {
  bool ok;
  for (int i=0; i < num_bits; i++) {
    ok = false;
    for (int j=0; j < num_bits; j++) {
      if (B[j].x == A[i].x && B[j].y == A[i].y) {
        ok = true;
        break;
      }
    }
    if (!ok)
      return false;
  }
  return true;
}

void PrintPBM(coord_t *bits, int num_bits, int xMax, int yMax) 
{
  for (int y = 0; y < yMax; y++) {
    for (int x = 0; x < xMax; x++) {
      int in = 0;
      for (int i=0; i < num_bits; i++) {
        if (x == bits[i].x && y == bits[i].y) {
          in = 1;
          break;
        }
      }
      printf("%d ", in);
    }
    printf("\n");
  }
}

#endif
