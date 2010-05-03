#include <stdio.h>
#include <stdlib.h>
#include <state.h>
#include <pqueue.h>
#include <omp.h>

state_t *ReadPBM(char *, int *, int *);
void print_state(state_t *);
void print_analysis(analysis_t *);
int xMax, yMax;

void print_history(state_t *S, state_t *end, int offset) {
#ifdef DEBUG
  if (!S)
#else
  if (!S || !S->prev)
#endif
    return;
#ifdef DEBUG
  if (S->prev)
#endif
    print_history(S->prev, end, offset);
#ifdef DEBUG
  printf("s=%d, ", SCORE(S, end) - offset);
#endif
  printf("%s\n", S->history);
#ifdef DEBUG_VERBOSE
  print_state(S);
  PrintPBM(S->bits, S->num_bits, xMax, yMax);
#endif
}

int work(hashmap_t *map, pqueue_t *pq, state_t *start, state_t *end) {
  int offset = SCORE(start, end);
  int perm = 0, added = 0, discard = 0, duplicate = 0;

  analysis_t *A;

  bool test, running = true, history_printed = false, waiting = false;
  int num_next, num_waiting = 0;
  state_t *current, *to_be_added;
  #pragma omp parallel private(test, num_next, current, A, to_be_added) firstprivate(waiting)
  while (running) {
    #pragma omp critical (pq)
    current = pq_take(pq);
    if (current != NULL) {
      state_t *next = possible_next_states(current, &num_next);
      perm++;
      for (int i=0; i < num_next; i++) {
        if (state_equal(next + i, end)) {
          #pragma omp critical (all)
          {
            if (!history_printed) {
              print_history(next + i, end, offset);
              history_printed = true;
              running = false;
            }
          }
          goto stop;
        }
        #pragma omp critical (map)
        test = put(map, (next + i)->bits, (next + i)->num_bits);
        if (test) {
          duplicate++;
        } else {
          A = analyze_state(next + i);
          if (can_reach_state(A, end)) {
            to_be_added = malloc(sizeof(state_t));
            memcpy(to_be_added, A->state, sizeof(state_t));
            int s = SCORE(to_be_added, end);
            if (s > to_be_added->prev->score)
              s += SCORE_REGRESSION_PENALTY;
            #pragma omp critical (pq)
            pq_add(pq, to_be_added, s - offset);
            added++;
          } else {
            discard++;
          }
//          free_list(A->ranges);
//          free(A);
        }
      }
//      free(next);
    } else {
      #pragma omp critical (all)
      {
        if (!waiting) {
          waiting = true;
          num_waiting++;
        } else if (num_waiting >= omp_get_num_threads()) {
          running = false;
        }
      }
    }
    stop:;
  }
#ifdef DEBUG
  printf("%d states tried\n", perm);
  printf("%d invalid branches eliminated\n", discard);
  printf("%d duplicate branches eliminated\n", duplicate);
  printf("%d states still queued\n", added - perm + 1);
  printf("%d is max hash table load\n", map->maxlen);
  printf("%d is hash table size\n", map->size);
  printf("%d is depth of priority queue\n", pq_stat_list_depth(pq));
#endif
  if (history_printed)
    return 0;
  return 1;
}

int main(int argc, char *argv[])
{
  int exit_code = 0;
  state_t *start, *end;

  start = ReadPBM(argv[1], &xMax, &yMax);
  end = ReadPBM(argv[2], &xMax, &yMax); // assume same size arrays

  hashmap_t *map = create_hashmap();
  pqueue_t *pq = construct_pqueue();

  analysis_t *A = analyze_state(start);
  if (can_reach_state(A, end))
    pq_add(pq, start, 0);
  if (state_equal(start, end))
    return 0;

  exit_code = work(map, pq, start, end);

  if (exit_code == 1)
    printf("IMPOSSIBLE\n");

  return exit_code;
}


void print_analysis(analysis_t *A) {
  printf("Analysis of state %p: ", A->state);
  range_list_t *list = A->ranges;
  printf("\n");
  while (list) {
    printf("\t%s ([%d, %d], %d) ", prettyo(list->value.o), list->value.min, list->value.max, list->value.bound);
    list = list->next;
    printf("\n");
  }
}

void print_state(state_t *S) {
  printf("State %p: ", S);
  for (int i=0; i < S->num_bits; i++)
    printf("(%d, %d) ", S->bits[i].x, S->bits[i].y);
  printf("\n");
}

state_t *ReadPBM(char *filename, int *xMax_ptr, int *yMax_ptr) 
{
  FILE *fp;
  char c, pbm_type[3];  // P[1-6], two characters. We assume P1 (bitmap)
  state_t *init_state = malloc(sizeof(state_t));
  int size = 100;
  init_state->bits = malloc(size*sizeof(coord_t));
  init_state->num_bits = 0;
  init_state->prev = NULL;
  init_state->history = "read from file";
  init_state->score = SCORE_UNDEFINED_SCORE;
  coord_t *pos = init_state->bits;

  fp = fopen(filename,"r");
  if (!fp) {
    printf("could not open file '%s'\n", filename);
    exit(2);
  }
  int ret = fscanf(fp,"%s %d %d", pbm_type, xMax_ptr, yMax_ptr);
  if (!ret)
    printf("read failed");
  do {c=getc(fp);} while (c != '0' && c != '1'); // Read all whitespace
  ungetc(c,fp);

  for (int y = 0; y < *yMax_ptr; y++) {
    for (int x = 0; x < *xMax_ptr; x++) {
      if (((int)getc(fp)-'0')) {
        init_state->num_bits++;
        if (init_state->num_bits >= size) {
          printf("REALLOC\n");
          size *= 2;
          int diff = pos - init_state->bits;
          init_state->bits = realloc(init_state->bits, size*sizeof(coord_t));
          pos = init_state->bits + diff;
        }
        pos->x = x;
        pos->y = y;
        pos++;
      }
      getc(fp); // EOL CR or space between letters
    }
  }

  fclose(fp);
  return init_state;
}

// vim: et sw=2
