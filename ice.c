#include <stdio.h>
#include <stdlib.h>
#include <state.h>
#include <queue.h>

state_t *ReadPBM(char *, int *, int *);
void print_state(state_t *);
void print_analysis(analysis_t *);

void print_history(state_t *S) {
  if (!S || !S->prev)
    return;
  print_history(S->prev);
  printf("%s\n", S->history);
}

int main(int argc, char *argv[])
{
  int xMax, yMax;
  state_t *start, *end;

  start = ReadPBM(argv[1], &xMax, &yMax);
  end = ReadPBM(argv[2], &xMax, &yMax); // assume same size arrays

  int num_next;
  queue_t *queue = construct_queue();
  add(queue, start);

  int perm = 0, added = 0, discard = 0;
  while (!isempty(queue)) {
    state_t *next = possible_next_states(take(queue), &num_next);
    perm++;
    for (int i=0; i < num_next; i++) {
      if (state_equal(next + i, end)) {
        print_history(next + i);
        goto out;
      }
      analysis_t *A = analyze_state(next + i);
      if (can_reach_state(A, end)) {
        state_t *to_be_added = malloc(sizeof(state_t));
        memcpy(to_be_added, A->state, sizeof(state_t));
        add(queue, to_be_added);
        added++;
      } else {
        discard++;
      }
      free_list(A->ranges);
      free(A);
    }
    free(next);
  }
  printf("IMPOSSIBLE\n");

  out:
  printf("%d states tried\n", perm);
  printf("%d branches eliminated\n", discard);
  printf("%d states still queued\n", added - perm + 1);
  return 0;
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
  coord_t *pos = init_state->bits;

  fp = fopen(filename,"r");
  fscanf(fp,"%s %d %d", pbm_type, xMax_ptr, yMax_ptr);
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
