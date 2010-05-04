#ifndef CONFIG_H
#define CONFIG_H

//#define DEBUG_VERY_VERBOSE
//#define DEBUG_VERBOSE
#define DEBUG

#define PARALLEL

#define HASHMAP_DEFAULT_CAPACITY 100003 // prime
#define QUEUE_DEFAULT_CAPACITY 100
#define QUEUE_BUFFER_SIZE 80 // affects breadth of search

#define SCORE_UNDEFINED_SCORE 999999
#define SCORE_REGRESSION_PENALTY 5

#endif
