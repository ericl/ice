default:
	gcc -pthread -lm -g -I. -Wall -std=c99 *.c -o ice
clean:
	rm ice
