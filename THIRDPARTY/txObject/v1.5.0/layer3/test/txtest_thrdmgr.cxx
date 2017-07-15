///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h> 

#include "txlock.h"
#include "txthread.h"
#include "txthrdmgr.h"

static txEvent G_EXIT_EVENT;
static int G_EXIT_FLAG = 0;
static unsigned long G_TIMERS = 0;

void timerThread (void* time_out)
{
	int y_time = *(int*) time_out;

	while (!G_EXIT_FLAG)
	{
		txThread::yield(y_time);
		G_TIMERS++;

		printf("TIMEOUT : %d\n", y_time);
		printf("   ID   : %ld\n", txThread::id());
		printf("   NAME : %s\n", txThread::name());
	}
}

void timerTest (void*)
{
	txThread::yield(10000);

	G_EXIT_EVENT.trigger();
	G_EXIT_FLAG = 1;

	printf(" NUM OF WAITS PER 10 SECONDS = %ld\n", G_TIMERS);
}

int main (void)
{
	printf("BEGIN MAIN LOOP\n");

	txThread::start(timerTest, 0, "timerTest");

	int pri = txNativeThread::DEFAULT_PRIORITY;
	int stack = txNativeThread::DEFAULT_STACK_SIZE;;
	txThreadManager threadMgr("THRD_MGR", 5, 10, 5, 10, pri, stack);

	int* a = new int(100);
	int* b = new int(500);
	int* c = new int(1000);

	threadMgr.start(timerThread, a);
	threadMgr.start(timerThread, b);
	threadMgr.start(timerThread, c);

	txThread::yield(G_EXIT_EVENT);

	printf("END MAIN LOOP\n");

	delete a; delete b; delete c;

	return 1; 
}

