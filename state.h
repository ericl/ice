#ifndef STATE_H
#define STATE_H

#include <hashmap.h>
#include <common.h>
#include <math.h>

typedef struct state {
  struct state *prev;
  char *history;
  int num_bits;
  int score;
  coord_t *bits;
} state_t;

typedef struct analysis {
	struct state *state;
	struct range_list *ranges;
} analysis_t;

void free_list(range_list_t *ranges) {
  if (ranges) {
    free_list(ranges->next);
    free(ranges);
  }
}

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
  return coord_set_equal(A->bits, B->bits, A->num_bits);
}

int score(state_t *S, state_t *end) {
  if (S->score != SCORE_UNDEFINED_SCORE)
    return S->score;
  int num_bits = S->num_bits;
  int bp = 0, hp = 0;
  bool ok;
  coord_t extras[num_bits];
  coord_t holes[num_bits];
  for (int i=0; i < num_bits; i++) {
    coord_t e = end->bits[i];
    ok = false;
    for (int j=0; j < num_bits; j++) {
      coord_t s = S->bits[i];
      if (s.x == e.x && s.y == e.y) {
        ok = true;
        break;
      }
    }
    if (!ok)
      holes[bp++] = e;
  }
  for (int i=0; i < num_bits; i++) {
    coord_t s = S->bits[i];
    ok = false;
    for (int j=0; j < num_bits; j++) {
      coord_t e = end->bits[i];
      if (s.x == e.x && s.y == e.y) {
        ok = true;
        break;
      }
    }
    if (!ok)
      extras[hp++] = s;
  }

  double edgedist = 0;

  for (int i=0; i < hp; i++) {
    for (int j=0; j < hp; j++) {
      double a = (holes[i].x - extras[j].x) * (holes[i].x - extras[j].x);
      double b = (holes[i].y - extras[j].y) * (holes[i].y - extras[j].y);
      if (a < b)
        edgedist += a;
      else
        edgedist += b;
    }
  }

  S->score = (int)sqrt(edgedist);
  return S->score;
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

char *prettyo(orientation o) {
  if (o == HORIZ)
    return "HORIZ";
  else
    return "VERT";
}

char *prettydir(direction dir) {
  switch (dir) {
    case EAST: return "E";
    case WEST: return "W";
    case SOUTH: return "S";
    case NORTH: return "N";
  }
  return "ERROR";
}

void replace_bit(state_t *S, coord_t old, coord_t dest, state_t *next, direction dir) {
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
  next->score = SCORE_UNDEFINED_SCORE;
  next->num_bits = S->num_bits;
  next->history = malloc(15*sizeof(char)); // that's room for 6-digit #'s
  next->prev = S;
  sprintf(next->history, "%d %d %s", old.x, old.y, prettydir(dir));
}

void bit_dir(state_t *S, coord_t bit, coord_t **west, coord_t **east, coord_t **north, coord_t **south) {
  int westpos, eastpos, northpos, southpos;
  bool nowestpos = true, noeastpos = true, nonorthpos = true, nosouthpos = true;
  for (int i=0; i < S->num_bits; i++) {
    coord_t test = S->bits[i];
    if (test.x == bit.x) {
      if (test.y > bit.y) {
        if (nosouthpos || test.y < southpos)
          southpos = test.y;
        nosouthpos = false;
      } else if (test.y < bit.y) {
        if (nonorthpos || test.y > northpos)
          northpos = test.y;
        nonorthpos = false;
      }
    } else if (test.y == bit.y) {
      if (test.x > bit.x) {
        if (noeastpos || test.x < eastpos)
          eastpos = test.x;
        noeastpos = false;
      } else if (test.x < bit.x) {
        if (nowestpos || test.x > westpos)
          westpos = test.x;
        nowestpos = false;
      }
    }
  }
  if (!nowestpos && westpos + 1 != bit.x) {
    *west = malloc(sizeof(coord_t));
    (*west)->x = westpos + 1;
    (*west)->y = bit.y;
  }
  if (!noeastpos && eastpos - 1 != bit.x) {
    *east = malloc(sizeof(coord_t));
    (*east)->x = eastpos - 1;
    (*east)->y = bit.y;
  }
  if (!nonorthpos && northpos + 1 != bit.y) {
    *north = malloc(sizeof(coord_t));
    (*north)->x = bit.x;
    (*north)->y = northpos + 1;
  }
  if (!nosouthpos && southpos - 1 != bit.y) {
    *south = malloc(sizeof(coord_t));
    (*south)->x = bit.x;
    (*south)->y = southpos - 1;
  }
}

state_t *possible_next_states(state_t *S, int *num_states) {
  state_t *states = malloc(4*sizeof(state_t)*S->num_bits);
  state_t *pos = states;
  *num_states = 0;
  for (int i=0; i < S->num_bits; i++) {
    coord_t bit = S->bits[i];
    coord_t *west = NULL, *east = NULL, *north = NULL, *south = NULL;
    bit_dir(S, bit, &west, &east, &north, &south);
    if (west) {
      replace_bit(S, bit, *west, pos++, WEST);
      *num_states = *num_states + 1;
      free(west);
    }
    if (east) {
      replace_bit(S, bit, *east, pos++, EAST);
      *num_states = *num_states + 1;
      free(east);
    }
    if (north) {
      replace_bit(S, bit, *north, pos++, NORTH);
      *num_states = *num_states + 1;
      free(north);
    }
    if (south) {
      replace_bit(S, bit, *south, pos++, SOUTH);
      *num_states = *num_states + 1;
      free(south);
    }
  }
  return states;
}

#endif

// vim: sw=2 et
