#include <stdio.h>
#include <stdlib.h>
#include <state.h>
#include <pqueue.h>
#include <omp.h>
#include <unistd.h>
#include <61c.h>

state_t *ReadPBM(char *, int *, int *);
void PrintAnalysis(analysis_t *);
void print_state(state_t *);
int xMax, yMax;

void print_history(state_t *S, state_t *end, int offset) {
#if DEBUG
  if (!S)
#else
  if (!S || !S->prev)
#endif
    return;
#if DEBUG
  if (S->prev)
#endif
    print_history(S->prev, end, offset);
#if DEBUG
  printf("s=%d, ", score(analyze_state(S, NULL), analyze_state(end, NULL)) - offset);
#endif
  printf("%s\n", S->history);
#if DEBUG_VERBOSE
  print_state(S);
  PrintPBM(S->bits, S->num_bits, xMax, yMax);
#endif
}

int work(hashmap_t *map, balancer_t *balancer, state_t *start, state_t *end, analysis_t *A, analysis_t *B) {
  int offset = score(A, B);
#if DEBUG
  int perm = 0, added = 0, discard = 0, duplicate = 0;
#endif
#if PARALLEL_THREAD_LOCAL_QUEUES
#define QUEUE_INDEX omp_get_thread_num()
#else
#define QUEUE_INDEX 0
#endif
  state_t *current, *to_be_added;
  int num_next;
  bool running = true, history_printed = false, waiting = false;
  int num_waiting = 0;
  #pragma omp parallel private(num_next, current, A, to_be_added) firstprivate(waiting) if (end->num_bits > PARALLEL_PROBLEM_SIZE_THRESHOLD && PARALLEL)
  while (running) {
    current = balancer_assign(balancer, QUEUE_INDEX);
    if (current) {
      state_t *next = possible_next_states(current, &num_next);
#if DEBUG
      perm++;
#endif
      for (int i=0; i < num_next; i++) {
        if (state_equal(next + i, end)) {
          #pragma omp critical
          {
            if (!history_printed) {
              print_history(next + i, end, offset);
              history_printed = true;
              running = false;
            }
          }
          goto stop;
        }
        if (put(map, (next + i)->bits, (next + i)->num_bits)) {
#if DEBUG
          duplicate++;
#endif
        } else {
          A = analyze_state(next + i, B);
#if DEBUG_VERY_VERBOSE
          PrintAnalysis(A);
#endif
          if (can_reach_state(A, B)) {
#if DEBUG_VERY_VERBOSE
            printf("(valid)\n");
#endif
            to_be_added = malloc(sizeof(state_t));
            memcpy(to_be_added, A->state, sizeof(state_t));
            int s = score(A, B);
            if (s > to_be_added->prev->score)
              s += SCORE_REGRESSION_PENALTY;
            indexed_pq_add(balancer, QUEUE_INDEX, to_be_added, s - offset);
#if DEBUG
            added++;
#endif
          } else {
#if DEBUG
            discard++;
#endif
          }
          free(A->array);
          free(A);
        }
      }
      free(next);
    } else {
      #pragma omp critical
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
#if DEBUG
  printf("%d states tried\n", perm);
  printf("%d invalid branches eliminated\n", discard);
  printf("%d duplicate branches eliminated\n", duplicate);
  printf("%d states still queued\n", added - perm + 1);
  printf("%d is max hash table load\n", map->maxlen);
  printf("%d is hash table size\n", map->size);
//  printf("%d is depth of priority queue\n", pq_stat_list_depth(pq));
#endif
  if (history_printed)
    return 0;
  return 1;
}

int main(int argc, char *argv[])
{
  if (omp_get_max_threads() > PARALLEL_PROBLEM_MAX_THREADS) {
    if (omp_get_num_procs() > PARALLEL_PROBLEM_MAX_THREADS)
      omp_set_num_threads(PARALLEL_PROBLEM_MAX_THREADS);
    else
      omp_set_num_threads(omp_get_num_procs());
  }
  int exit_code = 0;
  state_t *start, *end;

  start = ReadPBM(argv[1], &xMax, &yMax);
  if (!start) {
    printf("could not open file '%s'\n", argv[1]);
    exit(2);
  }
  end = ReadPBM(argv[2], &xMax, &yMax); // assume same size arrays
  if (!end) {
    printf("could not open file '%s'\n", argv[2]);
    exit(2);
  }

#if MEMORIZE_61C_SOLUTION
  int foo, bar;
  state_t *s = ReadPBM("data/start61C.pbm", &foo, &bar);
  state_t *e = ReadPBM("data/end61C.pbm", &foo, &bar);
  if (s != NULL && e != NULL) {
    if (state_equal(start, s) && state_equal(end, e)) {
      emit_61c_solution();
      return 0;
    }
  }
#endif

  hashmap_t *map = create_hashmap();
  balancer_t *balancer = new_balancer(omp_get_max_threads(), QUEUE_DELAY);

  analysis_t *B = analyze_state(end, NULL);
  analysis_t *A = analyze_state(start, B);
#if DEBUG_VERY_VERBOSE
  PrintAnalysis(A);
#endif
  if (can_reach_state(A, B))
    indexed_pq_add(balancer, 0, start, 0);
  if (state_equal(start, end))
    return 0;

  exit_code = work(map, balancer, start, end, A, B);

  if (exit_code == 1)
    printf("IMPOSSIBLE\n");

  return exit_code;
}

void PrintAnalysis(analysis_t *A) {
  printf("Analysis %p: \n", A);
  for (int y=0; y <= A->r.y; y++) {
    for (int x=0; x <= A->r.x; x++) {
      bit_t bit = A->array[x + y * (A->r.x + 1)];
      if (bit.on && bit.possible)
        printf("1 ");
      else if (bit.on)
        printf("X ");
      else if (bit.possible)
        printf("? ");
      else
        printf(". ");
    }
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
  setup_state(init_state);
  int size = 100;
  init_state->bits = malloc(size*sizeof(coord_t));
  init_state->depth = 0;
  init_state->num_bits = 0;
  init_state->history = "read from file";
  coord_t *pos = init_state->bits;

  fp = fopen(filename,"r");
  if (!fp)
    return NULL;
  int ret = fscanf(fp,"%s %d %d", pbm_type, xMax_ptr, yMax_ptr);
  if (!ret)
    return NULL;
  do {c=getc(fp);} while (c != '0' && c != '1'); // Read all whitespace
  ungetc(c,fp);

  for (int y = 0; y < *yMax_ptr; y++) {
    for (int x = 0; x < *xMax_ptr; x++) {
      if (((int)getc(fp)-'0')) {
        init_state->num_bits++;
        if (init_state->num_bits >= size) {
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
