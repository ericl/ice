#ifndef CBUF_H
#define CBUF_H

#include <state.h>

typedef struct bufnode {
	state_t *ptr;
	int prio;
} bufnode_t;

typedef struct cbuf {
	bufnode_t *storage;
	bufnode_t *head;
	bufnode_t *tail;
	int size;
	int capacity;
} cbuf_t;

cbuf_t *new_buffer(int capacity) {
	cbuf_t *buf = malloc(sizeof(cbuf_t));
	buf->storage = calloc(capacity, sizeof(bufnode_t));
	buf->head = buf->storage;
	buf->tail = buf->head;
	buf->capacity = capacity;
	buf->size = 0;
	return buf;
}

bufnode_t *increment(cbuf_t *buf, bufnode_t *ptr) {
	if (ptr == buf->storage + buf->capacity - 1)
		return buf->storage;
	return ptr + 1;
}

bool cbuf_full(cbuf_t *buf) {
	return buf->size >= buf->capacity;
}

bool cbuf_empty(cbuf_t *buf) {
	return buf->size <= 0;
}

void cbuf_add(cbuf_t *buf, state_t *ptr, int prio) {
	if (cbuf_full(buf)) {
		perror("circular buffer overflow\n");
		return;
	}
	buf->head->ptr = ptr;
	buf->head->prio = prio;
	buf->head = increment(buf, buf->head);
	buf->size++;
}

bufnode_t *cbuf_remove(cbuf_t *buf) {
	if (cbuf_empty(buf)) {
		perror("circular buffer underflow\n");
		return NULL;
	}
	bufnode_t *node = buf->tail;
	buf->tail = increment(buf, buf->tail);
	buf->size--;
	return node;
}

#endif
