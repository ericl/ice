#ifndef QUEUE_H
#define QUEUE_H

#include <state.h>
#include <string.h>

#define QUEUE_DEFAULT_CAPACITY 1000

typedef struct queue {
	state_t **storage;
	state_t **head;
	state_t **tail;
	int capacity;
	int used;
} queue_t;

queue_t *construct_queue() {
	state_t **storage = malloc(QUEUE_DEFAULT_CAPACITY*sizeof(state_t*));
	queue_t *queue = malloc(sizeof(queue_t));
	queue->storage = storage;
	queue->head = storage;
	queue->tail = storage;
	queue->capacity = QUEUE_DEFAULT_CAPACITY;
	return queue;
}

void free_queue(queue_t *queue) {
	free(queue->storage);
	free(queue);
}

bool isempty(queue_t *queue) {
	return queue->tail == queue->head;
}

void add(queue_t *queue, state_t *state) {
	if (queue->head - queue->storage >= queue->capacity) {
		int steps_back = queue->tail - queue->storage;
		memmove(queue->storage, queue->tail, queue->head - queue->tail);
		queue->head -= steps_back;
		queue->tail -= steps_back;
		int headpos = queue->head - queue->storage;
		queue->capacity *= 2;
		queue->storage = realloc(queue->storage, queue->capacity*sizeof(state_t*));
		queue->head = queue->storage + headpos;
		queue->tail = queue->storage;
	}
	*queue->head++ = state;
}

state_t *take(queue_t *queue) {
	return *queue->tail++;
}

#endif
