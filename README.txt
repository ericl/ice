Solver for the CS61C "Blocks on Ice" contest (which was never actually held...)
See http://inst.eecs.berkeley.edu/~cs61c/sp10/assignments/ice/ for details.

Example Usage:

	time ./ice data/start_blocks.pbm data/end_blocks.pbm

Some thoughts:

The algorithm I used is a variation on greedy best-first search. Each state is analyzed to determine if it should be pruned off the search tree. It has a tendency to either solve puzzles within the first second or not at all within memory bounds. Dense puzzles greater than 15x15 probably won't be solvable without a lot of luck.

Parallelism can be enabled by editing config.h, but speedups are not great and it makes runtimes nondeterministic.
