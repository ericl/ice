#include <stdio.h>
#include <stdlib.h>
#include <state.h>
#include <pqueue.h>
#include <omp.h>

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
  printf("s=%d, ", score(analyze_state(S), analyze_state(end)) - offset);
#endif
  printf("%s\n", S->history);
#if DEBUG_VERBOSE
  print_state(S);
  PrintPBM(S->bits, S->num_bits, xMax, yMax);
#endif
}

int work(hashmap_t *map, master_pq_t *master, state_t *start, state_t *end, analysis_t *A, analysis_t *B) {
  int offset = score(A, B);
#if DEBUG
  int perm = 0, added = 0, discard = 0, duplicate = 0;
#endif

  state_t *current, *to_be_added;
  int num_next;
  bool running = true, history_printed = false, waiting = false;
  int num_waiting = 0;
  #pragma omp parallel private(num_next, current, A, to_be_added) firstprivate(waiting)
  while (running) {
    current = indexed_pq_take(master, omp_get_thread_num());
    if (!current)
      current = global_pq_take(master, omp_get_thread_num());
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
          A = analyze_state(next + i);
#if DEBUG_VERY_VERBOSE
          PrintAnalysis(A);
#endif
          if (can_reach_state(A, B)) {
            to_be_added = malloc(sizeof(state_t));
            memcpy(to_be_added, A->state, sizeof(state_t));
            int s = score(A, B);
            if (s > to_be_added->prev->score)
              s += SCORE_REGRESSION_PENALTY;
            indexed_pq_add(master, omp_get_thread_num(), to_be_added, s - offset);
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
  int exit_code = 0;
  state_t *start, *end;

  start = ReadPBM(argv[1], &xMax, &yMax);
  end = ReadPBM(argv[2], &xMax, &yMax); // assume same size arrays

  hashmap_t *map = create_hashmap();
  master_pq_t *master = new_master_pq(omp_get_max_threads(), QUEUE_DELAY);

  analysis_t *A = analyze_state(start);
  analysis_t *B = analyze_state(end);
#if DEBUG_VERBOSE
  PrintAnalysis(A);
#endif
  if (can_reach_state(A, B))
    indexed_pq_add(master, 0, start, 0);
  if (state_equal(start, end))
    return 0;

  exit_code = work(map, master, start, end, A, B);

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
