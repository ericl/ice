#ifndef STATE_H
#define STATE_H

#include <hashmap.h>
#include <common.h>
#include <math.h>
#include <alloca.h>
#include <string.h>

typedef struct state {
  char *history;
  int score;
  coord_t *bits;
  int num_bits : 16;
  int depth : 16;
} state_t;

void setup_state(state_t *state) {
  state->score = SCORE_UNDEFINED_SCORE;
}

void fisher_yates_shuffle(state_t *array, int n) {
  state_t *tmp = alloca(sizeof(state_t));
  for (int i=n; i > 1; i--) {
    int j = rand() % i; // [0, i-1];
    memcpy(tmp, array + j, sizeof(state_t));
    memcpy(array + j, array + i - 1, sizeof(state_t));
    memcpy(array + i - 1, tmp, sizeof(state_t));
  }
}

typedef struct bit {
  bool on : 1;
  bool possible : 1;
} bit_t;

// NOTE: ice free()s the array later, do not access
// from state->analysis!
typedef struct analysis {
	struct state *state;
    coord_t l;
    coord_t r;
    bit_t *array;
    bool valid;
} analysis_t;

bool all_coords_in_array(coord_t bound, bit_t *array, state_t *T) {
  for (int i=0; i < T->num_bits; i++) {
    coord_t e = T->bits[i];
    if (e.x > bound.x || e.y > bound.y || !array[e.x + e.y * (bound.x+1)].possible)
      return false;
  }
  return true;
}

bool can_reach_state(analysis_t *A, analysis_t *B) {
	return A->valid && (A->state->num_bits == B->state->num_bits)
        && A->r.x >= B->r.x && A->r.y >= B->r.y
		&& all_coords_in_array(A->r, A->array, B->state);
}

analysis_t *new_analysis(state_t *S, coord_t l, coord_t r, bit_t *array) {
	analysis_t *A = malloc(sizeof(analysis_t));
	A->state = S;
    A->l.x = l.x;
    A->l.y = l.y;
    A->r.x = r.x;
    A->r.y = r.y;
    A->array = array;
    A->valid = true;
	return A;
}

bool put_edge(bit_t *array, int bound, state_t *S, orientation o, int xmax, int ymax, analysis_t *end, direction d) {
    // whether the min/max bits of S are on in the final state
    bool lon = false;
    bool ron = false;
    int endc = 0;
    if (o == VERT) {
      int buf[ymax+1];
      int I = 0;
      for (int y=0; y <= ymax; y++) {
        if (array[bound + y * (xmax+1)].on)
          buf[I++] = y;
      }
      if (end) {
        // we're sure this matches the edge exactly
        if ((d == WEST && bound == end->l.x) || (d == EAST && bound == end->r.x)) {
          // check memory bounds (in y multiples), if out of bounds false
          lon = buf[0] <= end->r.y && end->array[bound + buf[0]*(end->r.x+1)].on;
          ron = buf[I-1] <= end->r.y && end->array[bound + buf[I-1]*(end->r.x+1)].on;
          for (int y=0; y <= end->r.y; y++)
            if (end->array[bound + y*(end->r.x+1)].on)
              endc++;
          if (endc > I)
            return false;
        }
      }
      for (int i=0; i < I; i++) {
        for (int disp=-i; disp < I-i; disp++) {
          if (end) {
            if (disp != 0 && I == 2 && ((lon && ron) || (!lon && !ron && I == endc)))
              continue;
            if (i == 1 && disp < 0 && lon)
              continue;
            if (i == I-2 && disp > 0 && ron)
              continue;
          }
          array[bound + (buf[i]+disp) * (xmax+1)].possible = true;
        }
      }
    } else {
      int buf[ymax+1];
      int I = 0;
      for (int x=0; x <= xmax; x++) {
        if (array[x + bound * (xmax+1)].on)
          buf[I++] = x;
      }
      if (end) {
        // we're sure this matches the edge exactly
        if ((d == NORTH && bound == end->l.y) || (d == SOUTH && bound == end->r.y)) {
          // check memory bounds (e.g. if at end of array)
          lon = buf[0] <= end->r.x && end->array[buf[0] + bound*(end->r.x+1)].on;
          ron = buf[I-1] <= end->r.x && end->array[buf[I-1] + bound*(end->r.x+1)].on;
          for (int x=0; x <= end->r.x; x++)
            if (end->array[x + bound*(end->r.x+1)].on)
              endc++;
          if (endc > I)
            return false;
        }
      }
      for (int i=0; i < I; i++) {
        for (int disp=-i; disp < I-i; disp++) {
          if (end) {
            if (disp != 0 && I == 2 && ((lon && ron) || (!lon && !ron && I == endc)))
              continue;
            if (i == 1 && disp < 0 && lon)
              continue;
            if (i == I-2 && disp > 0 && ron)
              continue;
          }
          array[buf[i] + disp + bound * (xmax+1)].possible = true;
        }
      }
    }
    return true;
}

void put_range(bit_t *array, int bound, state_t *S, orientation o, int xmax, int ymax) {
    bool first = true;
    int min = 0, max = 0;
    if (o == VERT) {
      for (int y=0; y <= ymax; y++) {
        if (array[bound + y * (xmax+1)].on || array[bound + y * (xmax+1)].possible) {
          if (first) {
            min = y;
            max = y;
            first = false;
          } else {
            max = y;
          }
        } 
      }
      if (!first)
        for (int y=min; y <= max; y++)
          array[bound + y * (xmax+1)].possible = true;
    } else {
      for (int x=0; x <= xmax; x++) {
        if (array[x + bound * (xmax+1)].on || array[x + bound * (xmax+1)].possible) {
          if (first) {
            min = x;
            max = x;
            first = false;
          } else {
            max = x;
          }
        }
      }
      if (!first)
        for (int x=min; x <= max; x++)
          array[x + bound * (xmax+1)].possible = true;
    }
}

bool state_equal(state_t *A, state_t *B) {
  if (A->num_bits != B->num_bits)
    return false;
  return coord_set_equal(A->bits, B->bits, A->num_bits);
}

int score_node_dist(state_t *S, state_t *end, analysis_t *S_a, analysis_t *end_a) {
  int num_bits = S->num_bits;
  int bp = 0, hp = 0;
  coord_t extras[num_bits];
  coord_t holes[num_bits];

  for (int i=0; i < num_bits; i++) {
    register coord_t e = end->bits[i];
    if (e.x > S_a->r.x || e.y > S_a->r.y || !S_a->array[e.x + e.y * (S_a->r.x+1)].on) {
      holes[bp++] = e;
    }
  }
  for (int i=0; i < num_bits; i++) {
    register coord_t s = S->bits[i];
    if (s.x > end_a->r.x || s.y > end_a->r.y || !end_a->array[s.x + s.y * (end_a->r.x+1)].on) {
      extras[hp++] = s;
    }
  }

  int edgedist = 0;

  for (int i=0; i < hp; i++) {
    for (int j=0; j < hp; j++) {
      int a = abs(holes[i].x - extras[j].x);
      int b = abs(holes[i].y - extras[j].y);
      if (a < b)
        edgedist += a;
      else
        edgedist += b;
    }
  }
  return edgedist + hp;
}

analysis_t *analyze_state(state_t *S, analysis_t *end) {
	coord_t left_bound, right_bound, l, r;
    left_bound.x = 0;
    left_bound.y = 0;
    right_bound.x = 0;
    right_bound.y = 0;
	bool collide = false;

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

    bit_t *array = calloc((1+right_bound.x) * (1+right_bound.y), sizeof(bit_t));
	for (int i=0; i < S->num_bits; i++)
      array[S->bits[i].x + S->bits[i].y * (right_bound.x+1)].on = true;

    l.x = left_bound.x;
    l.y = left_bound.y;
    r.x = right_bound.x;
    r.y = right_bound.y;
    bool should_use_edge_analysis = true;

	analysis_loop: // shrink border to analyze each time

	if (left_bound.x > right_bound.x || left_bound.y > right_bound.y)
		return new_analysis(S, l, r, array);

	if (left_bound.x < right_bound.x) {
		// analyze two vertical lines
        if (should_use_edge_analysis) {
          if (!put_edge(array, left_bound.x, S, VERT, r.x, r.y, end, WEST))
            goto bad_analysis;
          if (!put_edge(array, right_bound.x, S, VERT, r.x, r.y, end, EAST))
            goto bad_analysis;
        } else {
          put_range(array, left_bound.x, S, VERT, r.x, r.y);
          put_range(array, right_bound.x, S, VERT, r.x, r.y);
        }
	} else {
		// analyze single vertical line
        if (should_use_edge_analysis) {
          if (!put_edge(array, left_bound.x, S, VERT, r.x, r.y, end, WEST))
            goto bad_analysis;
        } else
          put_range(array, left_bound.x, S, VERT, r.x, r.y);
		collide = true;
	}

	if (left_bound.y < right_bound.y) {
		// analyze the two horizontal lines
        if (should_use_edge_analysis) {
          if (!put_edge(array, left_bound.y, S, HORIZ, r.x, r.y, end, NORTH))
            goto bad_analysis;
          if (!put_edge(array, right_bound.y, S, HORIZ, r.x, r.y, end, SOUTH))
            goto bad_analysis;
        } else {
          put_range(array, left_bound.y, S, HORIZ, r.x, r.y);
          put_range(array, right_bound.y, S, HORIZ, r.x, r.y);
        }
	} else if (!collide) {
		// analyze single horizontal line
        if (should_use_edge_analysis) {
          if (!put_edge(array, left_bound.y, S, HORIZ, r.x, r.y, end, NORTH))
            goto bad_analysis;
        } else
          put_range(array, left_bound.y, S, HORIZ, r.x, r.y);
	}
	
	left_bound.x++;
	left_bound.y++;
	right_bound.x--;
	right_bound.y--;

    should_use_edge_analysis = false;
	goto analysis_loop;

    bad_analysis:
    ;

    analysis_t *a = new_analysis(S, l, r, array);
    a->valid = false;
    return a;
}

int score(analysis_t *A, analysis_t *B) {
  if (A->state->score != SCORE_UNDEFINED_SCORE)
    return A->state->score;
  int ret = score_node_dist(A->state, B->state, A, B);
  return A->state->score = ret;
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
  setup_state(next);
  next->num_bits = S->num_bits;
  // that's room for 6-digit #'s
  int len = strlen(S->history);
  next->history = calloc(len + 15, sizeof(char));
  strncpy(next->history, S->history, len);
  int more = sprintf(next->history + len, "%d %d %s", old.x, old.y, prettydir(dir));
  next->history[len + more] = '\n';
  next->depth = S->depth + 1;
}

void bit_dir(state_t *S, coord_t bit, coord_t **west, coord_t **east, coord_t **north, coord_t **south) {
  int westpos = 0, eastpos = 0, northpos = 0, southpos = 0;
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
  int n = 0;
  for (int i=0; i < S->num_bits; i++) {
    coord_t bit = S->bits[i];
    coord_t *west = NULL, *east = NULL, *north = NULL, *south = NULL;
    bit_dir(S, bit, &west, &east, &north, &south);
    if (east) {
      replace_bit(S, bit, *east, pos++, EAST);
      n++;
      free(east);
    }
    if (south) {
      replace_bit(S, bit, *south, pos++, SOUTH);
      n++;
      free(south);
    }
    if (west) {
      replace_bit(S, bit, *west, pos++, WEST);
      n++;
      free(west);
    }
    if (north) {
      replace_bit(S, bit, *north, pos++, NORTH);
      n++;
      free(north);
    }
  }
  *num_states = n;
#if SHUFFLE_NEW_STATES
  fisher_yates_shuffle(states, n);
#endif
  return states;
}

#endif

// vim: sw=2 et
