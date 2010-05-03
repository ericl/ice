default:
	gcc -Wall -ffast-math -funroll-loops -O3 -fopenmp -lm -g -I. -Wall -std=c99 *.c -o ice
clean:
	rm ice
