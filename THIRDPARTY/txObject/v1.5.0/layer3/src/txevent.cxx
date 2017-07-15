///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	#include <sched.h>
	#include <stdlib.h>
	#include <signal.h>
	#include <unistd.h>
	#include <sys/time.h>
#endif

#include <stdio.h>

#include "txevent.h"
#include "txthread.h"

#if defined TX_NON_PREEMPT_SUPPORT
	#include "event.h"
#endif

txEvent::txEvent (const char* name) :
	txThrdBase(name)
{
	#if defined TX_NON_PREEMPT_SUPPORT
		_native_obj = new Event(_name);
	#endif
}

txEvent::~txEvent (void)
{
	if (!_native_obj) return;

	trigger();

	#if defined TX_NON_PREEMPT_SUPPORT
		delete ((Event*) _native_obj); _native_obj = 0;
	#endif
}

void txEvent::trigger (void)
{
	if (!_native_obj) return;

	#if defined TX_NON_PREEMPT_SUPPORT
		((Event*) _native_obj)->trigger();
	#endif
}

int txEvent::wait (long timeout)
{
	if (!_native_obj) return -1;

	return txThread::yield(*this, timeout);
}

void txEvent::wait (void)
{
	if (!_native_obj) return;

	#if defined TX_NON_PREEMPT_SUPPORT
		((Event*) _native_obj)->wait();
	#endif
}

#if defined TX_PREEMPT_SUPPORT

txNativeEvent::txNativeEvent (const char* name) :
	txThrdBase(name)
{
#if defined TX_WIN
	_cond_ = CreateEvent(0, FALSE, FALSE, 0);
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	_signalled_ = 0;
	pthread_cond_init(&_cond_, 0);
	pthread_mutex_init(&_mutex_, 0);
#endif
}

txNativeEvent::~txNativeEvent (void)
{
#if defined TX_WIN
	CloseHandle(_cond_);
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	_signalled_ = -1;
	pthread_cond_destroy(&_cond_);
	pthread_mutex_destroy(&_mutex_);
#endif
}

int txNativeEvent::wait (long time)
{
	int flag = 0;

#if defined TX_WIN
	flag = (int) WaitForSingleObject(_cond_, time);
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	pthread_mutex_lock(&_mutex_);
	if (_signalled_ == 0)
	{
		timeval nw;
		timespec ts;

		gettimeofday(&nw, (struct timezone*) 0);

		ts.tv_sec = nw.tv_sec + (time_t) (time * 0.001);
		ts.tv_nsec = (nw.tv_usec * 1000) + (long) (time % 1000) * 1000000;

		flag = (int) pthread_cond_timedwait(&_cond_, &_mutex_, &ts);
	}
	_signalled_ = 0;
	pthread_mutex_unlock(&_mutex_);
#endif

	if (flag) flag = 0; else flag = 1; // mimic TX threads

	return flag;
}

void txNativeEvent::wait (void)
{
#if defined TX_WIN
	WaitForSingleObject(_cond_, INFINITE);
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	pthread_mutex_lock(&_mutex_);
	if (_signalled_ == 0)
	{
		pthread_cond_wait(&_cond_, &_mutex_);
	}
	_signalled_ = 0;
	pthread_mutex_unlock(&_mutex_);
#endif
}

void txNativeEvent::trigger (void)
{
#if defined TX_WIN
	SetEvent(_cond_);
#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	pthread_mutex_lock(&_mutex_);
	_signalled_ = 1;
	pthread_cond_signal(&_cond_);
	pthread_mutex_unlock(&_mutex_);
#endif
}

#endif // TX_PREEMPT_SUPPORT

