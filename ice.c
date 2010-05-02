#include <stdio.h>
#include <stdlib.h>
#include <state.h>

state_t *ReadPBM(char *, int *, int *);
void print_state(state_t *);
void print_analysis(analysis_t *);

char *prettyo(orientation o) {
  if (o == HORIZ)
    return "HORIZ";
  else
    return "VERT";
}

char *prettyb(bool b) {
  if (b)
    return "true";
  else
    return "false";
}

int main(int argc, char *argv[])
{
  int xMax, yMax;
  state_t *start, *end;
  analysis_t *A;

  start = ReadPBM(argv[1], &xMax, &yMax);
  end   = ReadPBM(argv[2], &xMax, &yMax); // assume same size arrays

  print_state(start);
  print_state(end);
  A = analyze_state(start);
  print_analysis(A);
  printf("%s\n", prettyb(can_reach_state(A, end)));

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
  init_state->bits = malloc(100*sizeof(coord_t));
  init_state->num_bits = 0;
  coord_t *pos = init_state->bits;

  fp = fopen(filename,"r");
  fscanf(fp,"%s %d %d", pbm_type, xMax_ptr, yMax_ptr);
  do {c=getc(fp);} while (c != '0' && c != '1'); // Read all whitespace
  ungetc(c,fp);

  for (int y = 0; y < *yMax_ptr; y++) {
    for (int x = 0; x < *xMax_ptr; x++) {
      if (((int)getc(fp)-'0')) {
        init_state->num_bits++;
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
