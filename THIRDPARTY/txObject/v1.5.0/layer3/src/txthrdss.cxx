///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txutil.h"
#include "txsync.h"
#include "txtimer.h"
#include "txthread.h"
#include "sys/txthrdss.h"

#if defined TX_NON_PREEMPT_SUPPORT
	#include "thread.h"
#endif

int txThreadSS::_shutdown_ = 0;

#if defined TX_NON_PREEMPT_SUPPORT
	void txThreadSS::_process_ (void*)
	{
		while (!txThreadSS::isShutdown())
		{
			bool didtime = txTimer::processExpiredTimers();

			Thread::_current_thread_->schedule();

			long time = 0;

			if (Thread::getActiveThreadCount() == 0)
			{
				time = txTimer::getMinWaitTime();
			}

			bool didio = txSync::IoAndTime(time);

			if ((didio == true) || (didtime == false))
			{
				Thread::_current_thread_->schedule();
			}
		}
	}

	void txThreadSS::_initHomeThreading_ (void)
	{
		Thread::_current_thread_ = new Thread(0, 0, "TX_MAIN", 2, 0, RUNNING);
		Thread::_io_time_thread_ = new Thread(txThreadSS::_process_, 0, "TX_IO", 1);
	}
#endif // TX_NON_PREEMPT_SUPPORT

/*
** TO JUST USE NATIVE AND NOT HOME THREADS, TURN ON NATIVE AND TURN OFF HOME THREADS
*/
#if !defined TX_NON_PREEMPT_SUPPORT
	#if defined TX_PREEMPT_SUPPORT
		void txThreadSS::_process_ (void*)
		{
			while (!txThreadSS::isShutdown())
			{
				txTimer::processExpiredTimers();

				long time = txTimer::getMinWaitTime();

				if (time == -1)
				{
					time = 100;
				}

				txSync::IoAndTime(time);
			}
		}

		void txThreadSS::_initNativeThreading_ (void)
		{
			txNativeThread::start(txThreadSS::_process_, 0, "TX_IO");
		}
	#endif
#endif

void txThreadSS::startup (void)
{
	#if defined TX_NON_PREEMPT_SUPPORT
		_initHomeThreading_();
	#endif

	#if !defined TX_NON_PREEMPT_SUPPORT
		#if defined TX_PREEMPT_SUPPORT
			_initNativeThreading_();
		#endif
	#endif
}

void txThreadSS::shutdown (void)
{
	#if defined TX_NON_PREEMPT_SUPPORT
		Thread::_current_thread_ = 0;
	#endif

	_shutdown_ = 1;
}
