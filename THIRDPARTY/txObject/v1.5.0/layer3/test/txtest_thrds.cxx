///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "thread.h"

#include "txlock.h"
#include "txevent.h"
#include "txthread.h"

static txEvent G_EXIT_EVENT;
static int G_EXIT_FLAG = 0;
static unsigned long G_TIMERS = 0;

void testThread (void* time_out)
{
	int y_time = *(int*) time_out; delete (int*) time_out;

	while (!G_EXIT_FLAG)
	{
		txThread::yield(y_time);

		G_TIMERS++;

		printf("TIMEOUT : %d\n", y_time);
		printf("   ID   : %ld\n", txThread::id());
		printf("   NAME : %s\n", txThread::name());

		#if defined TX_NON_PREEMPT_STATS
		Thread* t = (Thread*) txThread::id();
		printf("\nTHREAD STATS\n");
		printf("   numYields  : %lld\n", t->stats.getNumYields());
		printf("   runCPU     : %lld\n", t->stats.getRunCPU());
		printf("   runTime    : %lld\n", t->stats.getRunTime());
		printf("   waitTime   : %lld\n", t->stats.getWaitTime());
		printf("   aliveTime  : %lld\n", t->stats.getAliveTime());
		printf("   activeTime : %lld\n", t->stats.getActiveTime());
		printf("\n");
		#endif
	}
}

void testExit (void*)
{
	txThread::yield(10000);

	G_EXIT_FLAG = 1;
	G_EXIT_EVENT.trigger();

	printf(" NUM OF WAITS PER 10 SECONDS = %ld\n", G_TIMERS);
}

int main (void)
{
	printf("BEGIN MAIN LOOP\n");

	txThread::start(testExit, 0, "testExit");

	//txThread::start(testThread, new int(0), "testThread");
	//txThread::start(testThread, new int(10), "testThread");
	txThread::start(testThread, new int(100), "testThread");
	txThread::start(testThread, new int(1000), "testThread");
	txThread::start(testThread, new int(5000), "testThread");
	txThread::start(testThread, new int(10000), "testThread");
	txThread::start(testThread, new int(20000), "testThread");

	while (!G_EXIT_FLAG)
	{
		txThread::yield(G_EXIT_EVENT, 1000);
	}

	printf("END MAIN LOOP\n");

	return 1;
}

