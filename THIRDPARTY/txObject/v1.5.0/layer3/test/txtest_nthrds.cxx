///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "txlock.h"
#include "txevent.h"
#include "txthread.h"

static int G_EXIT_FLAG = 0;
static unsigned long G_TIMERS = 0;

static txNativeLock G_LOCK;
static txNativeEvent G_EXIT_EVENT;

void timer_thread (void* time_out)
{
	int y_time = *(int*) time_out; delete (int*) time_out;

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

	G_EXIT_FLAG = 1;
	G_EXIT_EVENT.trigger();

	G_LOCK.acquire();
	printf(" NUM OF WAITS PER 10 SECONDS = %ld\n", G_TIMERS);
	G_LOCK.release();
}

int main (void)
{
	G_LOCK.acquire();
	printf("BEGIN MAIN LOOP\n");
	G_LOCK.release();

	txNativeThread::start(timerTest);

	//txNativeThread::start(timer_thread, new int(0), "timer_thread");
	//txNativeThread::start(timer_thread, new int(10), "timer_thread");
	txNativeThread::start(timer_thread, new int(100), "timer_thread");
	txNativeThread::start(timer_thread, new int(1000), "timer_thread");
	txNativeThread::start(timer_thread, new int(5000), "timer_thread");
	txNativeThread::start(timer_thread, new int(10000), "timer_thread");
	txNativeThread::start(timer_thread, new int(20000), "timer_thread");

	while (!G_EXIT_FLAG)
	{
		txNativeThread::yield(G_EXIT_EVENT, 1000);
	}

	G_LOCK.acquire();
	printf("END MAIN LOOP\n");
	G_LOCK.release();

	return 1;
}

