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

void balancer_return_result(balancer_t *balancer, int job_id, state_t *state, int priority) {
	omp_set_lock(balancer->locks + job_id);
	pq_add(balancer->queues[job_id], state, priority);
	omp_unset_lock(balancer->locks + job_id);
}

void balancer_seed(balancer_t *balancer, state_t *state, int priority) {
	return balancer_return_result(balancer, 0, state, priority);
}

state_t *balancer_indexed_assign(balancer_t *balancer, int thread_id) {
	omp_set_lock(balancer->locks + thread_id);
	state_t *ret = pq_take(balancer->queues[thread_id]);
	omp_unset_lock(balancer->locks + thread_id);
	return ret;
}

state_t *balancer_fallback_assign(balancer_t *balancer, int thread_id) {
	state_t *ret = NULL;
	for (int i=thread_id; i < balancer->num; i++) {
		ret = balancer_indexed_assign(balancer, i);
		if (ret)
			return ret;
	}
	for (int i=0; i < thread_id; i++) {
		ret = balancer_indexed_assign(balancer, i);
		if (ret)
			return ret;
	}
	return ret;
}

state_t *balancer_assign(balancer_t *balancer, int thread_id, int *job_id) {
    state_t *job = balancer_indexed_assign(balancer, thread_id);
	*job_id = thread_id;
    if (!job)
      job = balancer_fallback_assign(balancer, thread_id);
	return job;
}

#endif
