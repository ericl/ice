/************************************************************************
**
** NAME:        ice.c
**
** DESCRIPTION: The 2010Sp CS61C "Bits on Ice" performace contest.
**              Compile with "gcc -std=c99 -o ice ice.c"
**              Run with two arguments "./ice start.pbm end.pbm"
**
** AUTHORS:     Dan Garcia <ddgarcia@cs.berkeley.edu> (version 1.0)
**              Yourfirstname Yourlastname
**              Yourfirstname Yourlastname
**
** DATE:        2010-05-01
**
** UPDATE HIST:  
**
** 2010-05-01 : v1.0 Release
**
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>

int *ReadPBM(char *, int *, int *);
void PrintPBM(char *, int *, int, int);

int main(int argc, char *argv[])
{
  int xMax, yMax;
  int *start, *end;

  start = ReadPBM(argv[1], &xMax, &yMax);
  end   = ReadPBM(argv[2], &xMax, &yMax); // assume same size arrays

  PrintPBM(argv[1], start, xMax, yMax);
  PrintPBM(argv[2], end,   xMax, yMax);
  printf("EXAMPLE OUTPUT FOR ./ice start.pbm end.pbm\n");
  printf("3 0 W\n");
  printf("1 2 N\n");
  printf("EXAMPLE OUTPUT FOR ./ice end.pbm start.pbm\n");
  printf("IMPOSSIBLE\n");

  return(0); // success!
}

/************************************************************************
**
** NAME:        ReadPBM
**
** DESCRIPTION: Open the PBM file, allocate space for a buffer big 
**              enough to hold it, read and fill said buffer, and close it.
** 
** INPUTS:      char *filename : The name of the PBM file to open
**              int *xMax_ptr  : Pointer to the number of columns
**              int *yMax_ptr  : Pointer to the number of rows
**
** OUTPUTS:     (int *), a pointer to the buffer of bits
**
************************************************************************/

int *ReadPBM(char *filename, int *xMax_ptr, int *yMax_ptr) 
{
  FILE *fp;
  char c, pbm_type[3];  // P[1-6], two characters. We assume P1 (bitmap)
  int *bits;

  fp = fopen(filename,"r");
  fscanf(fp,"%s %d %d", pbm_type, xMax_ptr, yMax_ptr);
  do {c=getc(fp);} while (c != '0' && c != '1'); // Read all whitespace
  ungetc(c,fp);

  bits = (int *) malloc (sizeof(int) * *xMax_ptr * *yMax_ptr);
  // Assume the malloc succeeds

  for (int y = 0; y < *yMax_ptr; y++) {
    for (int x = 0; x < *xMax_ptr; x++) {
      // Want bits[0][0] to be the upper-left bit
      // The subtraction from '0' converts the '0' char to the number 0.
      *(bits + *xMax_ptr*y + x) = (int) getc(fp)-'0'; 
      getc(fp); // EOL CR or space between letters
    }
  }

  fclose(fp);
  return(bits);
}

/************************************************************************
**
** NAME:        PrintPBM
**
** DESCRIPTION: Pretty-prints a PBM array. bits[0][0] is lower-left bit.
** 
** INPUTS:      char *filename : The name of the PBM file to print
**              int *bits      : The bit array
**              int xMax       : The number of columns
**              int yMax       : The number of rows
**
************************************************************************/

void PrintPBM(char *filename, int *bits, int xMax, int yMax) 
{
  printf("%s:\n", filename);
  for (int y = 0; y < yMax; y++) {
    for (int x = 0; x < xMax; x++) {
      printf("%d ", *(bits + xMax*y + x));
    }
    printf("\n");
  }
}
