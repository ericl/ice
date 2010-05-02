#ifndef PQUEUE_H
#define PQUEUE_H

#include <queue.h>
#include <math.h>

// goddamnit, a fake priority queue

typedef struct pqnode {
	int prio;
	struct pqnode *higher;
	queue_t *queue;
} pqnode_t;

typedef struct pqueue {
	pqnode_t *head;
} pqueue_t;

pqnode_t *construct_pqnode(int prio, pqnode_t *higher) {
	pqnode_t *head = malloc(sizeof(pqnode_t));
	head->prio = prio;
	head->higher = higher;
	head->queue = construct_queue();
	return head;
}

pqueue_t *construct_pqueue() {
	pqueue_t *pq = malloc(sizeof(pqueue_t));
	pq->head = construct_pqnode(10, NULL);
	return pq;
}

// anything over 5 is stepped down
int step_function(int p) {
	if (p < 5)
		return p;
	return 5 + (int)log(1+p-5);
}

int num_tail(pqnode_t *head) {
	if (!head)
		return 0;
	return 1 + num_tail(head->higher);
}

int pq_stat_list_depth(pqueue_t *pq) {
	return num_tail(pq->head);
}

pqnode_t *lowest_node(pqnode_t *head) {
	if (!head)
		return NULL;
	if (!isempty(head->queue))
		return head;
	return lowest_node(head->higher);
}

bool pq_isempty(pqueue_t *pq) {
	return !lowest_node(pq->head);
}

void pq_add(pqueue_t *pq, state_t *state, int priority) {
	priority = step_function(priority);
	pqnode_t *head = pq->head;
	if (priority < head->prio) {
		pq->head = construct_pqnode(priority, pq->head);
	}
	while (priority > head->prio) {
		if (!head->higher) {
			add(head->queue, state);
			return;
		}
		head = head->higher;
	}
	add(head->queue, state);
	return;
}

state_t *pq_take(pqueue_t *pq) {
	pqnode_t *lowest = lowest_node(pq->head);
	return take(lowest->queue);
}

#endif
