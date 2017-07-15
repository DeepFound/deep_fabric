///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "txsync.h"
#include "txtimer.h"
#include "txevent.h"
#include "txthread.h"

static int G_TIMEOUT = 60000;
static int G_EXIT_FLAG = 0;
static double G_START = 0.0;
static double G_STOP = 0.0;
static unsigned long G_COUNT = 0;
static unsigned long G_EXIT_COUNT = 100000000;

static txEvent H_TRIGGER_1;
static txEvent H_TRIGGER_2;

static txNativeEvent N_TRIGGER_1;
static txNativeEvent N_TRIGGER_2;

/*
unsigned long global_id = 0;

void fd_func (void*, int, unsigned long)
{
	printf("write to screen. "); fflush(stdout);

	txSync::unregisterIO(global_id);

	global_id = 0;
}

txTimerEnum::RETURN_STATUS timer1 (void* x)
{
	int* i = (int*) x;

	printf("Call Timer of 1000 (ms) : %d\n", *i);

	if (!global_id)
	{
		global_id = txSync::registerIO(fd_func, 0, 0, txSync::IOWrite);
	}

	return txTimerEnum::CONTINUE;
}
*/

/*
txTimerEnum::RETURN_STATUS timer1 (void* x)
{
	int* i = (int*) x;

	printf("Call Timer of 1000 (ms) : %d\n", *i);

	if (*i != 100)
	{
		return txTimerEnum::CONTINUE;
	}
	else
	{
		return txTimerEnum::STOP;
	}
}
*/

void homeSwitchThread1 (void*)
{
	G_START = txTimer::currentTime() * 0.001;

	while (!G_EXIT_FLAG)
	{
		H_TRIGGER_2.trigger();
		H_TRIGGER_1.wait();

		G_COUNT++;

		if (G_COUNT >= G_EXIT_COUNT)
		{
			G_EXIT_FLAG = 1;
		}

		//printf("-"); fflush(stdout);
	}

	G_STOP = txTimer::currentTime() * 0.001;
	H_TRIGGER_2.trigger();
}

void nativeSwitchThread1 (void*)
{
	G_START = txTimer::currentTime() * 0.001;

	while (!G_EXIT_FLAG)
	{
		N_TRIGGER_2.trigger();
		N_TRIGGER_1.wait();

		G_COUNT++;

		if (G_COUNT >= G_EXIT_COUNT)
		{
			G_EXIT_FLAG = 1;
		}

		//printf("-"); fflush(stdout);
	}

	G_STOP = txTimer::currentTime() * 0.001;
	N_TRIGGER_2.trigger();
}

void homeExitThread (void*)
{
	txThread::yield(G_TIMEOUT);

	G_STOP = txTimer::currentTime() * 0.001;
	G_EXIT_FLAG = 1;

	printf("HOME TIMED OUT\n");

	H_TRIGGER_2.trigger();
}

void nativeExitThread (void*)
{
	txNativeThread::yield(G_TIMEOUT);

	G_STOP = txTimer::currentTime() * 0.001;
	G_EXIT_FLAG = 1;

	printf("NATIVE TIMED OUT\n");

	N_TRIGGER_2.trigger();
}

static bool DO_HOME = true;

int main (int argc, char** argv)
{
	if (argc == 2)
	{
		DO_HOME = atoi(argv[1]);
	}

	if (DO_HOME == true)
	{
		printf("BEGIN MAIN LOOP ( HOME )\n");
	}
	else
	{
		printf("BEGIN MAIN LOOP ( NATIVE )\n");
	}

	/*
	int fd = 0; // file descriptor for screen output;
	txTimer timer(timer1, new int(0), 150);
	global_id = txSync::registerIO(fd_func, 0, fd, txSync::IOWrite);
	*/

	if (DO_HOME == true)
	{
		int priority = 1; /* set same as io/timer to schedule it in */
		txThread::start(homeExitThread, 0, "homeExitThread", priority);
		txThread::start(homeSwitchThread1, 0, "homeSwitchThread", priority);
		txThread::start(homeSwitchThread1, 0, "homeSwitchThread", priority);
		txThread::start(homeSwitchThread1, 0, "homeSwitchThread", priority);
		txThread::start(homeSwitchThread1, 0, "homeSwitchThread", priority);
		txThread::start(homeSwitchThread1, 0, "homeSwitchThread", priority);
	}
	else
	{
		txNativeThread::start(nativeExitThread, 0, "nativeExitThread");
		txNativeThread::start(nativeSwitchThread1, 0, "nativeSwitchThread");
		txNativeThread::start(nativeSwitchThread1, 0, "nativeSwitchThread");
		txNativeThread::start(nativeSwitchThread1, 0, "nativeSwitchThread");
		txNativeThread::start(nativeSwitchThread1, 0, "nativeSwitchThread");
		txNativeThread::start(nativeSwitchThread1, 0, "nativeSwitchThread");
	}

	while (!G_EXIT_FLAG)
	{
		if (DO_HOME == true)
		{
			H_TRIGGER_2.wait();
			H_TRIGGER_1.trigger();
		}
		else
		{
			N_TRIGGER_2.wait();
			N_TRIGGER_1.trigger();
		}

		//printf("+"); fflush(stdout);
	}

	printf("END MAIN LOOP\n");
	printf(" NUM OF WAITS PER %d SECONDS = %ld, %f\n", (G_TIMEOUT / 1000), G_COUNT, (G_STOP-G_START));

	return 1;
}

