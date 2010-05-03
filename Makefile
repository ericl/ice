CC = gcc-4.2
default:
	$(CC) -Wall -ffast-math -funroll-loops -O3 -fopenmp -lm -I. -Wall -std=c99 *.c -o ice-binary
clean:
	rm ice-binary
