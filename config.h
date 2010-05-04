#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG_VERY_VERBOSE 0
#define DEBUG_VERBOSE 0
#define DEBUG 0

#define HASHMAP_DEFAULT_CAPACITY 100003
#define QUEUE_DEFAULT_CAPACITY 100

#define PARALLEL 1
#define PARALLEL_PROBLEM_SIZE_THRESHOLD 10
#define PARALLEL_PROBLEM_MAX_THREADS 2

// set higher to increase search breadth
#define QUEUE_DELAY 80

// these two tend to conflict with each other
// resulting in highly unstable times for certain puzzles
//	* shuffling also tends to yield unstable times by itself
//	* parallel local thread queues are rather stable
#define PARALLEL_THREAD_LOCAL_QUEUES 1
#define SHUFFLE_NEW_STATES 0

#define SCORE_UNDEFINED_SCORE 999999
#define SCORE_REGRESSION_PENALTY 5
#define MEMORIZE_61C_SOLUTION 1

#endif
