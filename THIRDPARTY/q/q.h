/* q.h - lock-free, non-blocking message queue.  See https://github.com/fordsfords/q/tree/gh-pages */
/*
# This code and its documentation is Copyright 2014, 2015 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/q/tree/gh-pages
*/

#ifndef Q_H
#define Q_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int qerr_t;     /* see QERR_* definitions above */

struct q_s;                       /* forward (opaque) definition */
typedef struct q_s q_t;           /* object type of queue instance */

/* Most q APIs return "qerr_t".  These definitions must
 * be kept in sync with the "qerrs" string array in "q.c". */
#define QERR_OK 0         /* Success */
#define QERR_BUG1 1       /* Internal software bug - should never happen */
#define QERR_BUG2 2       /* Internal software bug - should never happen */
#define QERR_BADSIZE 3    /* q_size parameter invalid */
#define QERR_MALLOCERR 4  /* No memory available */
#define QERR_FULL 5       /* No room in queue */
#define QERR_EMPTY 6      /* No messages in queue */
#define LAST_QERR 6   /* Set to value of last "QERR_*" definition */

qerr_t q_create(q_t **rtn_q, unsigned int q_size);
/* Create an instance of a queue.
 * rtn_q  : Pointer to caller's queue instance handle.
 * q_size : Number of queue elements to allocate.  Must be > 1 and a power
 *          of 2.  Due to the nature of the algorithm used, a maximum of
 *          q_size - 1 elements can actually be stored in the queue.
 * Returns QERR_OK on success, or other QERR_* value on error. */

qerr_t q_delete(q_t *q);
/* Delete an instance of a queue.
 * q : Queue instance handle.
 * Returns QERR_OK on success, or other QERR_* value on error. */

qerr_t q_enq( q_t *q, void *m);
/* Add a message to the queue.
 * q : Queue instance handle.
 * m : Message to enqueue.
 * Returns QERR_OK on success, QERR_FULL if queue full, or other QERR_* value on error. */

qerr_t q_deq(q_t *q, void **rtn_m);
/* Remove a message from the queue.
 * q     : Queue instance handle.
 * rtn_m : Pointer to caller's message handle.
 * Returns QERR_OK on success, QERR_EMPTY if queue empty, or other QERR_* value on error. */

int q_is_empty(q_t *q);
/* Returns 1 if queue is empty (contains no messages), 0 otherwise.
 * q : Queue instance handle. */

int q_is_full(q_t *q);
/* Returns 1 if queue is full (contains q_size-1 message), 0 otherwise.
 * q : Queue instance handle. */

char *q_qerr_str(qerr_t qerr);
/* Returns a string representation of a queue API return error code.
 * qerr : value returned by most q APIs indicating success or faiure.
 * (See q.h for list of QERR_* definitions.) */

#ifdef __cplusplus
}
#endif

#endif  /* Q_H */
