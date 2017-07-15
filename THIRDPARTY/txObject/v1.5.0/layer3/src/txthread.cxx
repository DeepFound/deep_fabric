///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "thread.h"

#include "txlist.h"
#include "txevent.h"
#include "txtimer.h"
#include "txthread.h"
#include "sys/txthrdss.h"

static void startupThreading (void)
{
	txThreadSS::startup();	
}

static void shutdownThreading (void)
{
	txThreadSS::shutdown();
}

TX_STATIC_ALLOC_NOTIFY(startupThreading)
TX_STATIC_DEALLOC_NOTIFY(shutdownThreading)

const int txThread::DEFAULT_PRIORITY = 2;
const int txThread::DEFAULT_STACK_SIZE = THREAD_STACK_SIZE;

void txThread::start (TX_THREAD_FUNC func, void* context,
	const char* name, int priority, int stack_size)
{
	if (txThreadSS::isShutdown())
	{
		return;
	}

	#if defined TX_NON_PREEMPT_SUPPORT
		if ((priority < 1) || (priority >= THREAD_PRIORITY_RANGE))
		{
			fprintf(stderr, "TXOBJECT[error] : invalid priority range[%d]\n", priority);
			fflush(stderr); TX_CRASH;
		}

		if (stack_size < (THREAD_STACK_SIZE / 2))
		{
			fprintf(stderr, "TXOBJECT[error] : invalid stack size[%d]\n", stack_size);
			fflush(stderr); TX_CRASH;
		}

		new Thread (func, context, name, priority, stack_size);
	#endif
}

int txThread::priority (void)
{
	int retval = 0;

	#if defined TX_NON_PREEMPT_SUPPORT
		retval = Thread::_current_thread_->priority();
	#endif

	return retval;
}

int txThread::priority (int priority)
{
	int retval = 0;

	#if defined TX_NON_PREEMPT_SUPPORT
		if ((priority <= 1) || (priority >= THREAD_PRIORITY_RANGE))
		{
			fprintf(stderr, "TXOBJECT[error] : invalid priority range[%d]\n", priority);
			fflush(stderr); TX_CRASH;
		}

		retval = Thread::_current_thread_->priority();

		Thread::_current_thread_->priority(priority);
	#endif

	return retval;
}

unsigned long txThread::id (void)
{
	unsigned long retval = 0;

	#if defined TX_NON_PREEMPT_SUPPORT
		retval = (unsigned long) Thread::_current_thread_;
	#endif

	return retval;
}

const char* txThread::name (void)
{
	const char* retval = 0; 

	#if defined TX_NON_PREEMPT_SUPPORT
		retval = Thread::_current_thread_->name();
	#endif

	return retval;
}

const char* txThread::name (const char* name)
{
	const char* retval = 0; 

	#if defined TX_NON_PREEMPT_SUPPORT
		retval = Thread::_current_thread_->name();

		Thread::_current_thread_->name(name);	
	#endif

	return retval;
}

static txList THE_EVENT_REUSE_LIST(TX_AUTODEL_ON);

inline txTimerEnum::RETURN_STATUS timerCallBackFunction (void* self)
{
	txEvent* event = (txEvent*) self;
 
	event->trigger();
 
	return txTimerEnum::STOP;
}

inline int wait (txEvent* event, unsigned long time_out)
{
	int reuse_flag = 0;

	if (!event)
	{
		if (!(event = (txEvent*) THE_EVENT_REUSE_LIST.get()))
		{
			event = new txEvent();
		}

		reuse_flag = 1;
	}

	txTimer timer(timerCallBackFunction, event, time_out);

	event->wait();

	if (reuse_flag)
	{
		THE_EVENT_REUSE_LIST.append(event);
	}

	return timer.status();
}

void txThread::yield (void)
{
#if defined TX_NON_PREEMPT_SUPPORT
	if (Thread::_io_time_thread_ == Thread::_current_thread_)
	{
		fprintf(stderr, "TXOBJECT[error]: yielding in an IO or TIMER callback is not allowed\n");
		fflush(stderr); TX_CRASH;
	}
#endif

	::wait(0, 0);
}

void txThread::yield (long time_out)
{
#if defined TX_NON_PREEMPT_SUPPORT
	if (Thread::_io_time_thread_ == Thread::_current_thread_)
	{
		fprintf(stderr, "TXOBJECT[error] : yielding in an IO or TIMER callback is not allowed\n");
		fflush(stderr); TX_CRASH;
	}
#endif

	::wait(0, time_out);
}

void txThread::yield (txEvent& event)
{
#if defined TX_NON_PREEMPT_SUPPORT
	if (Thread::_io_time_thread_ == Thread::_current_thread_)
	{
		fprintf(stderr, "TXOBJECT[error] : yielding in an IO or TIMER callback is not allowed\n");
		fflush(stderr); TX_CRASH;
	}
#endif

	event.wait();
}

int txThread::yield (txEvent& event, long time_out)
{
#if defined TX_NON_PREEMPT_SUPPORT
	if (Thread::_io_time_thread_ == Thread::_current_thread_)
	{
		fprintf(stderr, "TXOBJECT[error] : yielding in an IO or TIMER callback is not allowed\n");
		fflush(stderr); TX_CRASH;
	}
#endif

	return ::wait(&event, time_out);
}

void txThread::schedule (void)
{
#if defined TX_NON_PREEMPT_SUPPORT
	if (Thread::_current_thread_)
	{
		Thread::_current_thread_->schedule();
	}
#endif
}

#if defined TX_PREEMPT_SUPPORT

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	#include <sched.h>
	#include <stdlib.h>
	#include <signal.h>
	#include <unistd.h>
	#include <pthread.h>
#else
	#include <windows.h>
#endif


#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
class txThreadArgs // Used with pthreads to sim HOME threads
{
	public:
		TX_THREAD_FUNC func;
		void* context;

	public:
		txThreadArgs (TX_THREAD_FUNC f, void* c) : func(f), context(c)
		{
		}
};
#endif

const int txNativeThread::DEFAULT_PRIORITY = 2;
const int txNativeThread::DEFAULT_STACK_SIZE = THREAD_STACK_SIZE;

int txNativeThread::priority (void)
{
	// TODO
	return 0;
}

int txNativeThread::priority (int priority)
{
	// TODO
	return 0;
}

unsigned long txNativeThread::id (void)
{
#if defined TX_WIN
	return GetCurrentThreadId();
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	return (unsigned long) pthread_self();
#endif
}

const char* txNativeThread::name (void)
{
	static char buf[100] = "null";

#if defined TX_WIN
	sprintf(buf, "Windows Native thread : %ld", txNativeThread::id());
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	sprintf(buf, "Posix Native thread : %ld", txNativeThread::id());
#endif

	return buf;
}

const char* txNativeThread::name (const char* name)
{
	// not applicable
	return name;
}

void txNativeThread::yield (void)
{
#if defined TX_WIN
	Sleep(1);
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC

	#if TX_SOL
		static int check = 0;

		if (!check)
		{
			sigignore(SIGALRM); check = 1;
		}
	#endif

	::usleep(1000);
#endif
}

void txNativeThread::yield (long time)
{
#if defined TX_WIN
	Sleep(time);
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC

	#if TX_SOL
		static int check = 0;

		if (!check)
		{
			sigignore(SIGALRM); check = 1;
		}
	#endif

	::usleep(time * 1000);
#endif
}

void txNativeThread::yield (txNativeEvent& event)
{
	event.wait();
}

int txNativeThread::yield (txNativeEvent& event, long time)
{
	return event.wait(time);
}

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
static void* txCb (txThreadArgs* args) // Used with pthreads to sim HOME threads
{
	args->func(args->context);

	delete args; args = 0;

	pthread_t t = (pthread_t) txNativeThread::id();

	pthread_exit(&t);

	return 0;
}
#endif

void txNativeThread::start (TX_THREAD_FUNC func, void* context,
	const char*, int, int)
{
	// TODO: name, priority, size

	unsigned long id = 0;

#if defined TX_WIN
	id = (unsigned long)
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE) func, context, 0, 0);
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	txThreadArgs* args = new txThreadArgs(func, context); // delete in txCb
	pthread_create((pthread_t*) &id, 0, (void* (*)(void*)) txCb, args);
#endif
}

#endif // TX_PREEMPT_SUPPORT

