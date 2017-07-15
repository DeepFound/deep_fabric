///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if defined TX_NON_PREEMPT_DEBUG
#include <fstream>

#include "event.h"
#include "thread.h"
#include "thrdmgr.h"
#include "txhashset.h"

txHashSet* ThrdMgr::_threads_ = 0;

void ThrdMgr::_startup_ (void)
{
	_threads_ = new txHashSet();
}

void ThrdMgr::_shutdown_ (void)
{
	delete _threads_; _threads_ = 0;
}

void ThrdMgr::add (Thread* thread)
{
	if (!_threads_)
	{
		_startup_();
	}

	_threads_->insert(thread);
}

void ThrdMgr::remove (Thread* thread)
{
	if (_threads_)
	{
		_threads_->remove(thread);

		if (!_threads_->entries())
		{
			_shutdown_();
		}
	}
}

void ThrdMgr::log (void)
{
	Thread* thread;
	static std::ofstream f("thread.dump", std::ios::app);

	if (!_threads_) return;

	f << "ThreadId State Priority EventId ThreadName EventName" << std::endl;
	f << "-------- ----- -------- ------- ---------- ---------" << std::endl;

	txHashSetIterator iter(*_threads_);

	while (thread = (Thread*) iter.next())
	{
		f << (void*) thread << " ";

		switch (thread->state())
		{
			case ZOMBIE:
			{
				f << "ZOMBIE   ";
				break;
			}
			case RUNNING:
			{
				f << "RUNNING  ";
				break;
			}
			case ACTIVE:
			{
				f << "ACTIVE   ";
				break;
			}
			case DEACTIVE:
			{
				f << "DEACTIVE ";
				break;
			}
			case DEAD:
			{
				f << "DEAD     ";
				break;
			}
		}

		f << thread->priority() << " ";

		if (thread->_wait_event_)
		{
			f << (void*) thread->_wait_event_ << "  ";
		}
		else
		{
			f << (void*) 0 << " ";
		}

		f << thread->name() << " ";

		if (thread->_wait_event_)
		{
			f << thread->_wait_event_->name() << " ";
		}
		else
		{
			f << (void*) 0 << " ";
		}

		f << std::endl;
	}

	f << std::endl;
}
#endif /* TX_NON_PREEMPT_DEBUG */

