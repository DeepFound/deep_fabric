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

static txNativeLock G_LOCK;
static txNativeEvent G_EXIT_EVENT;
static int G_EXIT_FLAG = 0;
static unsigned long G_TIMERS = 0;

void timerThread (void* time_out)
{
	int y_time = *(int*) time_out;

	while (!G_EXIT_FLAG)
	{
		txNativeThread::yield(y_time);
		G_TIMERS++;

		G_LOCK.acquire();
		printf("TIMEOUT : %d\n", y_time);
		printf("   ID   : %ld\n", txNativeThread::id());
		printf("   NAME : %s\n", txNativeThread::name());
		G_LOCK.release();
	}
}

void timerTest (void*)
{
	txNativeThread::yield(10000);

	G_EXIT_EVENT.trigger();
	G_EXIT_FLAG = 1;

	G_LOCK.acquire();
	printf(" NUM OF WAITS PER 10 SECONDS = %ld\n", G_TIMERS);
	G_LOCK.release();
}

int main (void)
{
	G_LOCK.acquire();
	printf("BEGIN MAIN LOOP\n");
	G_LOCK.release();

	txNativeThread::start(timerTest, 0, "timerTest");

	int pri = txNativeThread::DEFAULT_PRIORITY;
	int stack = txNativeThread::DEFAULT_STACK_SIZE;;
	txNativeThreadManager threadMgr("THRD_MGR", 5, 10, 5, 10, pri, stack);

	int* a = new int(100);
	int* b = new int(500);
	int* c = new int(1000);

	threadMgr.start(timerThread, a);
	threadMgr.start(timerThread, b);
	threadMgr.start(timerThread, c);

	txNativeThread::yield(G_EXIT_EVENT);

	G_LOCK.acquire();
	printf("END MAIN LOOP\n");
	G_LOCK.release();

	delete a; delete b; delete c;

	return 1; 
}

