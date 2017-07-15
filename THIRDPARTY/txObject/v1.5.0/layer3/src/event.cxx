///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#include "event.h"
#include "thread.h"

class EventThreadList
{
	private:
		int _entries_;
		Thread* _head_;
		Thread* _tail_;

	public:
		EventThreadList (void) :
			_entries_(0),
			_head_(0),
			_tail_(0)
		{
		}

		~EventThreadList (void)
		{
		}

		const Thread* add (Thread* x)
		{
			if (_head_ == 0)
			{
				_head_ = x;
				x->_eprev_ = 0;
			}
			else
			{
				_tail_->_enext_ = x;
				x->_eprev_ = _tail_;
			}

			_tail_ = x;
			x->_enext_ = 0;

			_entries_++;

			return x;
		}

		Thread* remove (Thread* x)
		{
			if (x->_eprev_ == 0)
			{
				_head_ = x->_enext_;
			}
			else
			{
				x->_eprev_->_enext_ = x->_enext_;
			}

			if (x->_enext_ == 0)
			{
				_tail_ = x->_eprev_;
			}
			else
			{
				x->_enext_->_eprev_ = x->_eprev_;
			}

			_entries_--;

			return x;
		}

		Thread* get (void)
		{
			return _entries_ ? remove(_head_) : 0;
		}

		int entries (void) const
		{
			return _entries_;
		}
};


void Event::_remove_ (Event* event, Thread* thread)
{
	event->_threads_waiting_->remove(thread);
}

Event::Event (const char* name) :
	_name_(name)
{
	_threads_waiting_ = new EventThreadList();
}
 
Event::~Event (void)
{
	delete _threads_waiting_; _threads_waiting_ = 0;
}

void Event::trigger (void)
{
	Thread* thread;

	while (thread = _threads_waiting_->get())
	{
		thread->_wait_event_ = 0;
		thread->activate();
	}
}

void Event::wait (void)
{
	_threads_waiting_->add(Thread::_current_thread_);

	Thread::_current_thread_->_wait_event_ = this;
	Thread::_current_thread_->deactivate();
}

