/* q.c - lock-free, non-blocking message queue.  See https://github.com/fordsfords/q/tree/gh-pages */
/*
# This code and its documentation is Copyright 2014, 2015 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/q/tree/gh-pages
*/
/* Although the code contained herein was written from scratch by Steve Ford in 2014,
 * the algorithm was influenced by John D. Valois' 1994 paper:
 *     "Implementing Lock-Free Queues"
 * http://people.cs.pitt.edu/~jacklange/teaching/cs2510-f12/papers/implementing_lock_free.pdf
 *     Valois, J.: Implementing lock-free queues. In: Proceedings of the Seventh
 *     International Conference on Parallel and Distributed Computing Systems. (1994) 64–69
 *
 * See q.h file for details on the API functions.
 */

#include <stdlib.h>
#include <pthread.h>

/* If built with "-DSELFTEST" then include extras for unit testing. */
#ifdef SELFTEST
#  include "q_selftest.h"
#endif

#include "q.h"

#define Q_FENCE 0x7d831f20   /* used for testing (random number generated at random.org) */

/* This list of strings must be kept in sync with the
 * corresponding "QERR_*" constant definitions in "q.h".
 * It is used by the q_qerr_str() function. */
static char *qerrs[] = {
	"QERR_OK",
	"QERR_BUG1",
	"QERR_BUG2",
	"QERR_BADSIZE",
	"QERR_MALLOCERR",
	"QERR_FULL",
	"QERR_EMPTY",
	"BAD_QERR", NULL};
#define BAD_QERR (sizeof(qerrs)/sizeof(qerrs[0]) - 2)


/* message element */
struct q_msg_s {
	void *d;
	int in_use;
};
typedef struct q_msg_s q_msg_t;

/* q.h contains an empty forward definition of "q_s", and defines "q_t" */
struct q_s {
	unsigned int enq_cnt;     /* count of successful messages enqueued (tail pointer) */
	char enq_pad[CACHE_LINE_SIZE - (sizeof(unsigned int))];  /* align next var on cache line */

	unsigned int deq_cnt;     /* count of successful messages dequeued (head pointer) */
	char deq_pad[CACHE_LINE_SIZE - (sizeof(unsigned int))];  /* align next var on cache line */

	q_msg_t * volatile msgs;  /* Array of "q_size" elements */
	unsigned int size_mask;  /* Number of msgs elements minus 1 */
	/* make total size a multiple of cache line size, to prevent interference with whatever comes after */
	char final_pad[CACHE_LINE_SIZE - ( sizeof(unsigned int) + sizeof(void **) )];
};  /* struct q_s */


/* Internal function: return 1 if power of 2 */
static int is_power_2(unsigned int n)
{
	/* Thanks to Alex Allain at http://www.cprogramming.com/tutorial/powtwosol.html for this cute algo. */
	return ((n-1) & n) == 0;
}  /* is_power_2 */


/* See q.h for doc */
char *q_qerr_str(qerr_t qerr)
{
	if (qerr >= BAD_QERR) { return qerrs[BAD_QERR]; }  /* bad qerr */

	return qerrs[qerr];
}  /* q_qerr_str */


/* See q.h for doc */
qerr_t q_create(q_t **rtn_q, unsigned int q_size)
{
	/* Sanity check the error code definitions and strings */
	if (LAST_QERR != BAD_QERR - 1) { return QERR_BUG1; }  /* the QERR_* are out of sync with qerrs[] */
	if (sizeof(q_t) % CACHE_LINE_SIZE != 0) { return QERR_BUG2; }  /* q_t not multiple of cache line size */

	/* Sanity check input size */
	if (q_size <= 1 || ! is_power_2(q_size)) { return QERR_BADSIZE; }

	/* Create queue object instance */
	q_t *q = NULL;
	int perr = posix_memalign((void **)&q, CACHE_LINE_SIZE, sizeof(*q));
	if (perr != 0 || q == NULL) { return QERR_MALLOCERR; }

	/* Allocate message storage array (one extra unused element for fence) */
	q->msgs = NULL;
	perr = posix_memalign((void **)&q->msgs, CACHE_LINE_SIZE, (q_size + 1) * sizeof(q->msgs[0]) );
	if (perr != 0 || q->msgs == NULL) { free(q);  return QERR_MALLOCERR; }

	q->msgs[q_size].d = (void *)Q_FENCE;  /* used by unit tests to insure no overflow */

	/* empty the queue */
	unsigned int i;
	for (i = 0; i < q_size; i++) {
		q->msgs[i].in_use = 0;
	}

	q->enq_cnt = 0;  /* Init the queue counters */
	q->deq_cnt = 0;

	q->size_mask = q_size - 1;  /* bit mask to "and" enq_cnt and deq_cnt to get tail and head */

	/* Success */
	*rtn_q = q;
	return QERR_OK;
}  /* q_create */


/* See q.h for doc */
qerr_t q_delete(q_t *q)
{
	/* Quick sanity check to make sure the queue didn't overflow */
	if (q->msgs[q->size_mask + 1].d != (void *)Q_FENCE) { return QERR_BUG1; }
	q->msgs[q->size_mask + 1].d = NULL;  /* remove fence to maybe detect double-delete */

	free((void *)q->msgs);
	free(q);

	return QERR_OK;
}  /* q_delete */


/* See q.h for doc */
qerr_t q_enq(q_t *q, void *m)
{
	unsigned int tail = (unsigned)(q->enq_cnt & q->size_mask);

	/* Queue must always have at least one empty slot.  Make sure that
	 * after the current tail is filled, the next slot will be empty. */
	unsigned int next_tail = (tail + 1) & q->size_mask;
	if (q->msgs[next_tail].in_use) { return QERR_FULL; }  /* Queue is full, item not added */

	q->msgs[tail].d = (void * volatile)m;
	q->msgs[tail].in_use = 1;
	q->enq_cnt++;
	return QERR_OK;
}  /* q_enq */


/* See q.h for doc */
qerr_t q_deq(q_t *q, void **rtn_m)
{
	unsigned int head = (unsigned)(q->deq_cnt & q->size_mask);
	if (! q->msgs[head].in_use) { return QERR_EMPTY; }

	*rtn_m = (void *)q->msgs[head].d;
	q->msgs[head].in_use = 0;  /* mark it as empty */
	q->deq_cnt++;
	return QERR_OK;
}  /* q_deq */


/* See q.h for doc */
int q_is_empty(q_t *q)
{
	unsigned int head = (unsigned)(q->deq_cnt & q->size_mask);
	return (! q->msgs[head].in_use);
}  /* q_is_empty */


/* See q.h for doc */
int q_is_full(q_t *q)
{
	unsigned int tail = (unsigned)(q->enq_cnt & q->size_mask);
	unsigned int next_tail = (tail + 1) & q->size_mask;
	return (q->msgs[next_tail].in_use);
}  /* q_is_full */


/* If built with "-DSELFTEST" then include main() for unit testing. */
#ifdef SELFTEST
#include "q_selftest.c"
#endif
