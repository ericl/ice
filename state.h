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
} analysis_t;

bool all_edges_possible(analysis_t *A, state_t *T) {
  int wc = 0, nc = 0, sc = 0, ec = 0;
  // these hold the CURRENT state's edges
  coord_t w[T->num_bits];
  coord_t e[T->num_bits];
  coord_t n[T->num_bits];
  coord_t s[T->num_bits];
  coord_t bit;
  // first determine the edges for the CURRENT state
  for (int i=0; i < T->num_bits; i++) {
    bit = A->state->bits[i];
    // west edge
    if (bit.x == A->l.x) {
      w[wc++] = bit;
    }
    // east edge
    if (bit.x == A->r.x) {
      e[ec++] = bit;
    }
    // north edge
    if (bit.y == A->l.y) {
      n[nc++] = bit;
    }
    // south edge
    if (bit.y == A->r.y) {
      s[sc++] = bit;
    }
  }
  // now scan the END state to see if the edges work
  for (int j=0; j < T->num_bits; j++) {
    bit = T->bits[j];
    int lo_d = 0, hi_d = 0, hi_c = 0, lo_c = 0;
    // west edge
    if (bit.x == A->l.x) {
      for (int i=0; i < wc; i++) {
        if (w[i].y == bit.y) {
          goto bit_ok;
        } else if (w[i].y < bit.y) {
          if (!lo_c++ || bit.y - w[i].y < lo_d)
            lo_d = bit.y - w[i].y;
        } else if (w[i].y > bit.y) {
          if (!hi_c++ || w[i].y - bit.y < hi_d)
            hi_d = w[i].y - bit.y;
        }
      }
      if (!lo_c || !hi_c || (hi_c < lo_d && lo_c < hi_d))
        return false;
    }
    // east edge
    if (bit.x == A->r.x) {
      for (int i=0; i < ec; i++) {
        if (e[i].y == bit.y) {
          goto bit_ok;
        } else if (e[i].y < bit.y) {
          if (!lo_c++ || bit.y - e[i].y < lo_d)
            lo_d = bit.y - e[i].y;
        } else if (e[i].y > bit.y) {
          if (!hi_c++ || e[i].y - bit.y < hi_d)
            hi_d = e[i].y - bit.y;
        }
      }
      if (!lo_c || !hi_c || (hi_c < lo_d && lo_c < hi_d))
        return false;
    }
    // north edge
    if (bit.y == A->l.y) {
      for (int i=0; i < nc; i++) {
        if (n[i].x == bit.x) {
          goto bit_ok;
        } else if (n[i].x < bit.x) {
          if (!lo_c++ || bit.x - n[i].x < lo_d)
            lo_d = bit.x - n[i].x;
        } else if (n[i].x > bit.x) {
          if (!hi_c++ || n[i].x - bit.x < hi_d)
            hi_d = n[i].x - bit.x;
        }
      }
      if (!lo_c || !hi_c || (hi_c < lo_d && lo_c < hi_d))
        return false;
    }
    // south edge
    if (bit.y == A->r.y) {
      for (int i=0; i < sc; i++) {
        if (s[i].x == bit.x) {
          goto bit_ok;
        } else if (s[i].x < bit.x) {
          if (!lo_c++ || bit.x - s[i].x < lo_d)
            lo_d = bit.x - s[i].x;
        } else if (s[i].x > bit.x) {
          if (!hi_c++ || s[i].x - bit.x < hi_d)
            hi_d = s[i].x - bit.x;
        }
      }
      if (!lo_c || !hi_c || (hi_c < lo_d && lo_c < hi_d))
        return false;
    }
    bit_ok:
    ;
  }
  return true;
}

bool all_coords_in_array(coord_t bound, bit_t *array, state_t *T) {
  for (int i=0; i < T->num_bits; i++) {
    coord_t e = T->bits[i];
    if (e.x > bound.x || e.y > bound.y || !array[e.x + e.y * (bound.x+1)].possible)
      return false;
  }
  return true;
}

bool can_reach_state(analysis_t *A, analysis_t *B) {
	return (A->state->num_bits == B->state->num_bits)
        && A->r.x >= B->r.x && A->r.y >= B->r.y
        && all_edges_possible(A, B->state)
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
	return A;
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
        if (!first)
          for (int y=min; y <= max; y++) {
            array[bound + y * (xmax+1)].possible = true;
          }
      }
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
    coord_t e = end->bits[i];
    if (e.x > S_a->r.x || e.y > S_a->r.y || !S_a->array[e.x + e.y * (S_a->r.x+1)].on) {
      holes[bp++] = e;
    }
  }
  for (int i=0; i < num_bits; i++) {
    coord_t s = S->bits[i];
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
  return edgedist;
}

analysis_t *analyze_state(state_t *S) {
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

	analysis_loop: // shrink border to analyze each time

	if (left_bound.x > right_bound.x || left_bound.y > right_bound.y)
		return new_analysis(S, l, r, array);

	if (left_bound.x < right_bound.x) {
		// analyze two vertical lines
		put_range(array, left_bound.x, S, VERT, r.x, r.y);
		put_range(array, right_bound.x, S, VERT, r.x, r.y);
	} else {
		// analyze single vertical line
		put_range(array, left_bound.x, S, VERT, r.x, r.y);
		collide = true;
	}

	if (left_bound.y < right_bound.y) {
		// analyze the two horizontal lines
		put_range(array, left_bound.y, S, HORIZ, r.x, r.y);
		put_range(array, right_bound.y, S, HORIZ, r.x, r.y);
	} else if (!collide) {
		// analyze single horizontal line
		put_range(array, left_bound.y, S, HORIZ, r.x, r.y);
	}
	
	left_bound.x++;
	left_bound.y++;
	right_bound.x--;
	right_bound.y--;

	goto analysis_loop;
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
  next->score = SCORE_UNDEFINED_SCORE;
  next->num_bits = S->num_bits;
  next->history = malloc(15*sizeof(char)); // that's room for 6-digit #'s
  next->prev = S;
  sprintf(next->history, "%d %d %s", old.x, old.y, prettydir(dir));
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
    if (west) {
      replace_bit(S, bit, *west, pos++, WEST);
      n++;
      free(west);
    }
    if (east) {
      replace_bit(S, bit, *east, pos++, EAST);
      n++;
      free(east);
    }
    if (north) {
      replace_bit(S, bit, *north, pos++, NORTH);
      n++;
      free(north);
    }
    if (south) {
      replace_bit(S, bit, *south, pos++, SOUTH);
      n++;
      free(south);
    }
  }
  *num_states = n;
  return states;
}

#endif

// vim: sw=2 et
