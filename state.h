#include <malloc.h>
#include <stdbool.h>

#define DEBUG printf

typedef enum {HORIZ, VERT} orientation;

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
	struct coord *bits;
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

analysis_t *analyze_state(state_t *S) {
	coord_t left_bound, right_bound;
	bool collide = false;
	range_list_t *ranges = NULL;
	for (int i=0; i < S->num_bits; i++) {
		if (S->bits[i].x < left_bound.x)
			left_bound.x = S->bits[i].x;
		if (S->bits[i].y < left_bound.y)
			left_bound.y = S->bits[i].y;
		if (S->bits[i].x > right_bound.x)
			right_bound.x = S->bits[i].x;
		if (S->bits[i].y > right_bound.y)
			right_bound.y = S->bits[i].y;
	}

	analysis_loop: // shrink border to analyze each time

	DEBUG("left bound is (%d, %d)\n", left_bound.x, left_bound.y);
	DEBUG("right bound is (%d, %d)\n", right_bound.x, right_bound.y);
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

// vim: ts=2 et
