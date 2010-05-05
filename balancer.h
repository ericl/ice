#ifndef BALANCER_H
#define BALANCER_H

#include <pqueue.h>
#include <common.h>

typedef struct balancer {
	int num;
	pqueue_t **queues;
	omp_lock_t *locks;
} balancer_t;

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

void balancer_add(balancer_t *balancer, int index, state_t *state, int priority) {
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
