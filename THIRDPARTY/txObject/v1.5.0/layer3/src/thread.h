///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if !defined ( __THREAD_H__ )
#define __THREAD_H__

#include <stdlib.h>

#include "txlist.h"

#if defined TX_NON_PREEMPT_STATS
	#include "thrdstats.h"
#endif

class Event;
class ThreadList;

#if defined TX_JUMP_SUPPORT
class JumpContext;
#else
class FiberContext;
#endif

typedef void (*THREAD_FUNC_PTR) (void*);

#define THREAD_PRIORITY       5        // default thread priority
#define THREAD_STACK_SIZE     1024*10  // 10240 words per stack frame
#define THREAD_PRIORITY_RANGE 32       // 2-31 are valid thread levels

enum ThrdState
{
	ZOMBIE   = 0,	// In the process of being killed.
	RUNNING  = 1,	// Currently running.
	ACTIVE   = 2,	// Can be scheduled to run.
	DEACTIVE = 3,	// Pending on an event or io.
	DEAD     = 4	// STATIC threads end up DEAD.
};

enum ThrdMode
{
	DYNAMIC,	// Thread was created using new.
	STATIC		// Thread was created in the data section.
};

class Thread: public txObject
{
	private:
		Thread* _pprev_;
		Thread* _pnext_;
		Thread* _eprev_;
		Thread* _enext_;

		int _priority_;
		ThrdMode _mode_;
		ThrdState _state_;
		const char* _name_;
		Event* _wait_event_;

		#if defined TX_JUMP_SUPPORT
			JumpContext* _context_;
		#else
			FiberContext* _context_;
		#endif

		static int _count_;
		static int _max_pri_;
		static void* _new_thread_;
		static Thread* _io_time_thread_;
		static Thread* _current_thread_;

		static txList* _inact_threads_;
		static ThreadList** _act_threads_;

	private:
		void _allocContext_ (THREAD_FUNC_PTR func, void* args, int size);
		void _deleteContext_ (void);

		inline ThrdState _changeStateTo_ (ThrdState state);

		static void _initThreads_ ();
		static Thread* _getThread_ (void);
		static void _putThread_ (Thread* thread);
		static void _deleteInactiveThreads_ (void);

	public:
		static int getActiveThreadCount (void);

	public:
		Thread (
			THREAD_FUNC_PTR func,
			void* args = 0,
			const char* name = 0,
			int priority = THREAD_PRIORITY,
			int size = THREAD_STACK_SIZE,
			ThrdState state = ACTIVE);

		~Thread (void);

		void *operator new (size_t size);
		void operator delete (void* t);

		const char* name (void) const { return _name_; };
		void name (const char* name) { _name_ = name; };

		ThrdState state (void) const { return _state_; };
		ThrdMode mode (void) const { return _mode_; };

		int priority (void) const { return _priority_; };
		void priority (int pri) { _priority_ = pri; };

		void deactivate (void);
		void activate (void);
		void schedule (void);
		void kill (void);

		#if defined TX_NON_PREEMPT_STATS
			ThreadStats stats;
		#endif

	friend class txThreadSS;
	friend class txThread;

	friend class EventThreadList;
	friend class ThreadList;
	friend class Event;

	#if defined TX_NON_PREEMPT_DEBUG
		friend class ThrdMgr;
	#endif
};

class ThreadList
{
	private:
		int _entries_;
		Thread* _head_;
		Thread* _tail_;

	public:
		ThreadList (void):
			_entries_(0),
			_head_(0),
			_tail_(0)
		{
		}

		~ThreadList (void)
		{
		}

		const Thread* add (Thread* x)
		{
			if (_head_ == 0)
			{
				_head_ = x;
				x->_pprev_ = 0;
			}
			else
			{
				_tail_->_pnext_ = x;
				x->_pprev_ = _tail_;
			}

			_tail_ = x;
			x->_pnext_ = 0;

			_entries_++;

			return x;
		}

		Thread* remove (Thread* x)
		{
			if (x->_pprev_ == 0)
			{
				_head_ = x->_pnext_;
			}
			else
			{
				x->_pprev_->_pnext_ = x->_pnext_;
			}

			if (x->_pnext_ == 0)
			{
				_tail_ = x->_pprev_;
			}
			else
			{
				x->_pnext_->_pprev_ = x->_pprev_;
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

#endif // __THREAD_H__
