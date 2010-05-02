#ifndef QUEUE_H
#define QUEUE_H

#include <state.h>

typedef struct queue {
	state_t **head;
	state_t **tail;
	int capacity;
	int used;
} queue_t;

queue_t *construct_queue() {
	state_t **storage = malloc(100000*sizeof(state_t*));
	queue_t *queue = malloc(sizeof(queue_t));
	queue->head = storage;
	queue->tail = storage;
	queue->capacity = 100000;
	queue->used = 0;
	return queue;
}

bool isempty(queue_t *queue) {
	return queue->tail == queue->head;
}

void add(queue_t *queue, state_t *state) {
	if (queue->used++ >= queue->capacity) {
		printf("uh oh, probably should write a proper queue now");
		exit(1);
	}
	*queue->head++ = state;
}

state_t *take(queue_t *queue) {
	return *queue->tail++;
}

#endif
