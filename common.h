#ifndef COMMON_H
#define COMMON_H

#include <malloc.h>
#include <stdbool.h>
#include <config.h>

typedef enum {HORIZ, VERT} orientation;
typedef enum {EAST, WEST, NORTH, SOUTH} direction;

typedef struct coord {
	int x;
	int y;
} coord_t;

typedef struct range {
	orientation o;
	int min;
	int max;
	int bound;
} range_t;

typedef struct range_list {
	struct range value;
	struct range_list *next;
} range_list_t;

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

#endif
