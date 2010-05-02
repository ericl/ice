#ifndef STATE_H
#define STATE_H

#include <malloc.h>
#include <stdbool.h>

#define DEBUG printf

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

typedef struct state {
	char *history;
	int num_bits;
	coord_t *bits;
} state_t;

typedef struct analysis {
	struct state *state;
	struct range_list *ranges;
} analysis_t;

bool coord_in_non_1x1_ranges(coord_t c, range_list_t *ranges) {
	while (ranges) {
		if (ranges->value.o == HORIZ) {
			if (ranges->value.bound == c.y && ranges->value.min <= c.x && ranges->value.max >= c.x && ranges->value.min != ranges->value.max)
				return true;
		} else {
			if (ranges->value.bound == c.x && ranges->value.min <= c.y && ranges->value.max >= c.y && ranges->value.min != ranges->value.max)
				return true;
		}
		ranges = ranges->next;
	}
	return false;
}


bool coord_in_ranges(coord_t c, range_list_t *ranges) {
	while (ranges) {
		if (ranges->value.o == HORIZ) {
			if (ranges->value.bound == c.y && ranges->value.min <= c.x && ranges->value.max >= c.x)
				return true;
		} else {
			if (ranges->value.bound == c.x && ranges->value.min <= c.y && ranges->value.max >= c.y)
				return true;
		}
		ranges = ranges->next;
	}
	return false;
}

bool all_coords_in_ranges(range_list_t *ranges, state_t *T) {
	for (int i=0; i < T->num_bits; i++) {
		if (!coord_in_ranges(T->bits[i], ranges))
			return false;
	}
	return true;
}

bool can_reach_state(analysis_t *A, state_t *T) {
	return (A->state->num_bits == T->num_bits)
		&& all_coords_in_ranges(A->ranges, T);
}

analysis_t *new_analysis(state_t *S, range_list_t *ranges) {
	analysis_t *A = malloc(sizeof(analysis_t));
	A->state = S;
	A->ranges = ranges;
	return A;
}

void prepend(range_list_t **ranges, int min, int max, int bound, orientation o) {
	range_list_t *new_list = malloc(sizeof(range_list_t));
	new_list->next = *ranges;
	new_list->value.min = min;
	new_list->value.max = max;
	new_list->value.bound = bound;
	new_list->value.o = o;
	*ranges = new_list;
}

void put_range(range_list_t **list_handle, int bound, state_t *S, orientation o) {
	range_list_t *ranges = *list_handle;
	int first = true;
	int min, max;
	while (ranges) {
		if (ranges->value.o != o) {
			if (ranges->value.min <= bound && ranges->value.max >= bound) {
				if (first) {
					min = ranges->value.bound;
					max = ranges->value.bound;
					first = false;
				} else {
					if (ranges->value.bound < min)
						min = ranges->value.bound;
					if (ranges->value.bound > min)
						max = ranges->value.bound;
				}
			}
		}
		ranges = ranges->next;
	}
	for (int i=0; i < S->num_bits; i++) {
		coord_t c = S->bits[i];
		if (o == HORIZ && c.y == bound) {
			if (first) {
				min = c.x;
				max = c.x;
				first = false;
			} else {
				if (c.x < min)
					min = c.x;
				if (c.x > max)
					max = c.x;
			}
		} else if (o == VERT && c.x == bound) {
			if (first) {
				min = c.y;
				max = c.y;
				first = false;
			} else {
				if (c.y < min)
					min = c.y;
				if (c.y > max)
					max = c.y;
			}
		}
	}
	if (!first)
		prepend(list_handle, min, max, bound, o);
}

bool state_equal(state_t *A, state_t *B) {
  if (A->num_bits != B->num_bits)
    return false;
  bool ok;
  int max = A->num_bits;
	for (int i=0; i < max; i++) {
    ok = false;
    for (int j=0; j < max; j++) {
      if (B->bits[j].x == A->bits[i].x && B->bits[j].y == A->bits[i].y) {
        ok = true;
        break;
      }
    }
    if (!ok)
      return false;
  }
  return true;
}

analysis_t *analyze_state(state_t *S) {
	coord_t left_bound, right_bound;
	bool collide = false;
	range_list_t *ranges = NULL;
	for (int i=0; i < S->num_bits; i++) {
		if (!i || S->bits[i].x < left_bound.x)
			left_bound.x = S->bits[i].x;
		if (!i || S->bits[i].y < left_bound.y)
			left_bound.y = S->bits[i].y;
		if (!i || S->bits[i].x > right_bound.x)
			right_bound.x = S->bits[i].x;
		if (!i || S->bits[i].y > right_bound.y)
			right_bound.y = S->bits[i].y;
	}

	analysis_loop: // shrink border to analyze each time

	if (left_bound.x > right_bound.x || left_bound.y > right_bound.y)
		return new_analysis(S, ranges);

	if (left_bound.x < right_bound.x) {
		// analyze two vertical lines
		put_range(&ranges, left_bound.x, S, VERT);
		put_range(&ranges, right_bound.x, S, VERT);
	} else {
		// analyze single vertical line
		put_range(&ranges, left_bound.x, S, VERT);
		collide = true;
	}

	if (left_bound.y < right_bound.y) {
		// analyze the two horizontal lines
		put_range(&ranges, left_bound.y, S, HORIZ);
		put_range(&ranges, right_bound.y, S, HORIZ);
	} else if (!collide) {
		// analyze single horizontal line
		put_range(&ranges, left_bound.y, S, HORIZ);
	}
	
	left_bound.x++;
	left_bound.y++;
	right_bound.x--;
	right_bound.y--;

	goto analysis_loop;
}

void replace_bit(state_t *S, coord_t old, coord_t dest, state_t *next) {
  next->bits = malloc(S->num_bits*sizeof(coord_t));
  for (int i=0; i < S->num_bits; i++) {
    coord_t bit = S->bits[i];
    if (bit.x == old.x && bit.y == old.y) {
      next->bits[i].x = dest.x;
      next->bits[i].y = dest.y;
    } else {
      next->bits[i].x = bit.x;
      next->bits[i].y = bit.y;
    }
  }
  next->num_bits = S->num_bits;
}

void bit_dir(state_t *S, coord_t bit, coord_t **next, direction dir) {
  *next = NULL;
  int modx = 0, mody = 0;
  for (int i=0; i < S->num_bits; i++) {
    coord_t test = S->bits[i];
    if ((((dir == WEST || dir == EAST) && test.y == bit.y) ||
         ((dir == NORTH || dir == SOUTH) && test.x == bit.x)) &&
        ((dir == WEST && (test.x + 1 < bit.x)) ||
         (dir == EAST && (test.x - 1 > bit.x)) ||
         (dir == NORTH && (test.y + 1 < bit.y)) ||
         (dir == SOUTH && (test.y - 1 > bit.y)))) {
      if (!*next || (dir == WEST && ((*next)->x < test.x))
                 || (dir == EAST && ((*next)->x > test.x))
                 || (dir == NORTH && ((*next)->y < test.x))
                 || (dir == SOUTH && ((*next)->y > test.x))) {
        *next = malloc(sizeof(coord_t));
        switch (dir) {
          case WEST: modx = 1; break;
          case EAST: modx = -1; break;
          case NORTH: mody = 1; break;
          case SOUTH: mody = -1; break;
        }
        (*next)->x = test.x + modx;
        (*next)->y = test.y + mody;
      }
    }
  }
}

state_t *possible_next_states(state_t *S, int *num_states) {
  state_t *states = malloc(4*sizeof(state_t)*S->num_bits);
  state_t *pos = states;
  *num_states = 0;
  for (int i=0; i < S->num_bits; i++) {
    coord_t bit = S->bits[i];
    coord_t *next;
    bit_dir(S, bit, &next, WEST);
    if (next) {
      replace_bit(S, bit, *next, pos++);
      *num_states = *num_states + 1;
    }
    bit_dir(S, bit, &next, EAST);
    if (next) {
      replace_bit(S, bit, *next, pos++);
      *num_states = *num_states + 1;
    }
    bit_dir(S, bit, &next, NORTH);
    if (next) {
      replace_bit(S, bit, *next, pos++);
      *num_states = *num_states + 1;
    }
    bit_dir(S, bit, &next, SOUTH);
    if (next) {
      replace_bit(S, bit, *next, pos++);
      *num_states = *num_states + 1;
    }
  }
  return states;
}

#endif

// vim: sw=2 et
