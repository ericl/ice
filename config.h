#ifndef CONFIG_H
#define CONFIG_H

#define DEBUG_VERY_VERBOSE 0
#define DEBUG_VERBOSE 0
#define DEBUG 0

#define HASHMAP_DEFAULT_CAPACITY 100003
#define QUEUE_DEFAULT_CAPACITY 100

#define PARALLEL 0
#define PARALLEL_PROBLEM_SIZE_THRESHOLD 10
#define PARALLEL_MIN_THREADS 2
#define PARALLEL_MAX_THREADS 2
#define PARALLEL_CONTROL_NUM_THREADS 1

// set higher to increase search breadth
#define QUEUE_DELAY 80

// enable recursive path tracing when scoring states
#define TRACE 0
#define MAX_TRACE_CALLS_PER_BIT 1000
#define TRACE_MAX_BITS 30
#define TRACE_DEPTH 3

// these two tend to conflict with each other
// resulting in highly unstable times for certain puzzles
//	* shuffling also tends to yield unstable times by itself
//	* parallel local thread queues are rather stable
#define PARALLEL_THREAD_LOCAL_QUEUES 0
#define SHUFFLE_NEW_STATES 0

#define SCORE_UNDEFINED_SCORE 999999
#define SCORE_REGRESSION_PENALTY 5
#define FREE_MORE_MEMORY 1

#endif
