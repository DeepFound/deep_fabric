# q
Fast, lock-free, non-blocking queue

The q package is a C module which implements a fast, lock-free, non-blocking queue.

## License

Copyright 2014, 2015 Steven Ford http://geeky-boy.com and licensed
"public domain" style under
[CC0](http://creativecommons.org/publicdomain/zero/1.0/): 
![CC0](https://licensebuttons.net/p/zero/1.0/88x31.png "CC0")

To the extent possible under law, the contributors to this project have
waived all copyright and related or neighboring rights to this work.
In other words, you can use this code for any purpose without any
restrictions.  This work is published from: United States.  The project home
is https://github.com/fordsfords/q/tree/gh-pages

To contact me, Steve Ford, project owner, you can find my email address
at http://geeky-boy.com.  Can't see it?  Keep looking.

## Introduction

The q module implements a fast, lock-free, non-blocking queue.  Its characteristics are:

* Fixed size (size specified at queue creation time).
* Non-blocking (enqueuing to a full queue returns immediately with error; dequeuing from an empty queue returns immediately with error).
* Single Producer, Single Consumer (SPSC).
* No dynamic memory allocates or frees during enqueue and dequeue operations.  Messages are stored as void pointers; _null pointers are not allowed_.
* High performance.  On my Macbook Air, streaming data through a queue averages 11 ns per message (queue size=32), while ping-pong latency is 69 ns (one-way).
* Tested on Mac OSX 10.9 (Mavericks) and Linux 2.6 and 3.5 kernels.  At present, I only recommend the 64-bit x86 processor family, due to the fact that I take advantage of its programmer-friendly memory model.  In the future I hope to generalize it to be efficient on other processor types.

You can find q at:

* User documentation (this README): https://github.com/fordsfords/q/tree/gh-pages
* Semi-literate internal documentation: http://fordsfords.github.io/q/html/

## Quick Start

These instructions assume that you are running on Unix.  I have tried it on Linux, Solaris, FreeBSD, HP-UX, AIX, and Cygwin (on Windows 7).
Download the "q_0.2.tar" file.

1. Get the [github project](https://github.com/fordsfords/q/tree/gh-pages).

2. Build and test the package:

        ./bld.sh
        ./tst.sh

3. Run the performance tool:

        ./q_perf

## C API

Taken from q.h:

```
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
```

## Release Notes

* 0.1 (19-May-2014)

    Initial pre-release.

* 0.2 (1-Jun-2014)

    API CHANGE: changed terminology "status' to "error".  Specifically, changed:

    * "qstat_t" to "qerr_t"
    * "QSTAT_..." to "QERR_..."
    * "q_qstat_str" to "q_qerr_str"

    Added "in_use" flag as part of each message slot.  This allows null messages to be enqueued and dequeued.

    Added directory "alternates" containing variations on the queue algorithm.
Minor corrections in q_selftest.c.

* 0.3 (11-May-2015)

    Moved to Github.
