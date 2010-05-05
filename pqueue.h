#ifndef PQUEUE_H
#define PQUEUE_H

#include <queue.h>
#include <math.h>
#include <cbuf.h>
#include <omp.h>

typedef struct pqnode {
	int prio;
	struct pqnode *higher;
	queue_t *queue;
} pqnode_t;

typedef struct pqueue {
	pqnode_t *head;
	cbuf_t *buf;
} pqueue_t;

typedef struct balancer {
	int num;
	pqueue_t **queues;
	omp_lock_t *locks;
} balancer_t;

pqnode_t *construct_pqnode(int prio, pqnode_t *higher) {
	pqnode_t *head = malloc(sizeof(pqnode_t));
	head->prio = prio;
	head->higher = higher;
	head->queue = construct_queue();
	return head;
}

pqueue_t *construct_pqueue(int bufsize) {
	pqueue_t *pq = malloc(sizeof(pqueue_t));
	pq->head = construct_pqnode(20, NULL);
	pq->head = construct_pqnode(10, pq->head);
	pq->head = construct_pqnode(5, pq->head);
	pq->buf = new_buffer(bufsize);
	return pq;
}

balancer_t *new_balancer(int num, int bufsize) {
	balancer_t *balancer = malloc(sizeof(balancer_t));
	balancer->queues = malloc(num * sizeof(pqueue_t*));
	balancer->locks = malloc(num * sizeof(omp_lock_t));
	for (int i=0; i < num; i++) {
		balancer->queues[i] = construct_pqueue(bufsize);
		omp_init_lock(balancer->locks + i);
	}
	balancer->num = num;
	return balancer;
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
	return !lowest_node(pq->head) && cbuf_empty(pq->buf);
}

void pq_unbuffered_add(pqueue_t *pq, state_t *state, int priority) {
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

void pq_add(pqueue_t *pq, state_t *state, int priority) {
	if (cbuf_full(pq->buf)) {
		bufnode_t *node = cbuf_remove(pq->buf);
		pq_unbuffered_add(pq, node->ptr, node->prio);
	}
	cbuf_add(pq->buf, state, priority);
}

state_t *pq_take(pqueue_t *pq) {
	pqnode_t *lowest = lowest_node(pq->head);
	if (!lowest) {
		if (cbuf_empty(pq->buf))
			return NULL;
		return cbuf_remove(pq->buf)->ptr;
	}
	return take(lowest->queue);
}

void indexed_pq_add(balancer_t *balancer, int index, state_t *state, int priority) {
	omp_set_lock(balancer->locks + index);
	pq_add(balancer->queues[index], state, priority);
	omp_unset_lock(balancer->locks + index);
}

state_t *balancer_indexed_assign(balancer_t *balancer, int index) {
	omp_set_lock(balancer->locks + index);
	state_t *ret = pq_take(balancer->queues[index]);
	omp_unset_lock(balancer->locks + index);
	return ret;
}

state_t *balancer_fallback_assign(balancer_t *balancer, int index) {
	state_t *ret = NULL;
	for (int i=index; i < balancer->num; i++) {
		ret = balancer_indexed_assign(balancer, i);
		if (ret)
			return ret;
	}
	for (int i=0; i < index; i++) {
		ret = balancer_indexed_assign(balancer, i);
		if (ret)
			return ret;
	}
	return ret;
}

state_t *balancer_assign(balancer_t *balancer, int index) {
    state_t *job = balancer_indexed_assign(balancer, index);
    if (!job)
      job = balancer_fallback_assign(balancer, index);
	return job;
}

#endif
