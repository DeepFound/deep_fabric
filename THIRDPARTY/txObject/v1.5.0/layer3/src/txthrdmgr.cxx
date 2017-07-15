///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txlist.h"
#include "txthrdmgr.h"

class WorkNode
{
	private:
		WorkNode* _prev_;
		WorkNode* _next_;

	public:
		TX_THREAD_FUNC func;
		void* context;

		WorkNode (TX_THREAD_FUNC f, void* c) :
			func(f), context(c)
		{
		}

		~WorkNode (void)
		{
		}

		void reset (TX_THREAD_FUNC f, void* c)
		{
			func = f; context = c;
		}

	friend class WorkList;
};

class WorkList
{
	private:
		int _entries_;
		WorkNode* _head_;
		WorkNode* _tail_;

	public:
		WorkList (void):
			_entries_(0),
			_head_(0),
			_tail_(0)
		{
		}

		~WorkList (void)
		{
		}

		const WorkNode* add (WorkNode* x)
		{
			if (_head_ == 0)
			{
				_head_ = x;
				x->_prev_ = 0;
			}
			else
			{
				_tail_->_next_ = x;
				x->_prev_ = _tail_;
			}

			_tail_ = x;
			x->_next_ = 0;

			_entries_++;

			return x;
		}

		WorkNode* remove (WorkNode* x)
		{
			if (x->_prev_ == 0)
			{
				_head_ = x->_next_;
			}
			else
			{
				x->_prev_->_next_ = x->_next_;
			}

			if (x->_next_ == 0)
			{
				_tail_ = x->_prev_;
			}
			else
			{
				x->_next_->_prev_ = x->_prev_;
			}

			_entries_--;

			return x;
		}

		WorkNode* get (void)
		{
			return _entries_ ? remove(_head_) : 0;
		}

		int entries (void) const
		{
			return _entries_;
		}
};

static WorkList THE_WORKNODE_REUSE_LIST;

txThreadManager::txThreadManager (const char* name, int min, int max,
	int low, int high, int priority, int size) :

	txThrdBase(name),
	_work_nodes_(0),
	_min_threads_(min),
	_max_threads_(max),
	_current_threads_(0),
	_stack_size_(size),
	_priority_(priority),
	_do_task_exit_flag_(0),
	_workQueue_(low, high)
{
}

void txThreadManager::_addWork_ (WorkNode* worknode)
{
	txEvent* event;

	_workQueue_.put(worknode);

	if (event = (txEvent*) _eventQueue_.get())
	{
		event->trigger();
	}
	else if ((_current_threads_ < _max_threads_) || !_max_threads_)
	{
		_current_threads_++;

		txThread::start(
			txThreadManager::_doWorkThread_,
			this, name(), _priority_, _stack_size_);
	}
}

WorkNode* txThreadManager::_getWork_ (void)
{
	WorkNode* work = 0;

	if (_workQueue_.entries())
	{
		_workQueue_.get((void*&) work);
	}

	return work;
}

txThreadManager::~txThreadManager (void)
{
	txList* queue;
	txEvent* event;
	WorkNode* node;

	_do_task_exit_flag_ = 1;

	_workQueue_.unregisterQueue(queue);

	if (queue)
	{
		while (node = (WorkNode*) queue->get())
		{
			delete node; node = 0;
		}

		delete queue; queue = 0;
	}

	while (event = (txEvent*) _eventQueue_.get())
	{
		event->trigger();
	}
}

void txThreadManager::_doWorkThread_ (void* obj)
{
	int nt;
	int nw;
	WorkNode* work;
	txEvent event("_doWorkThread_");
	txThreadManager* self = (txThreadManager*) obj; 

	self->_work_nodes_++;

	while (!self->_do_task_exit_flag_)
	{
		if (work = self->_getWork_())
		{
			work->func(work->context);

			THE_WORKNODE_REUSE_LIST.add(work);

			txThread::yield();
		}
		else
		{
			nt = self->numberOfThreads();
			nw = self->numberOfWorkNodes();

			if ((nt > self->_min_threads_) && (nw < nt))
			{
				break;
			}

			self->_eventQueue_.append(&event);

			txThread::yield(event);
		}
	}

	self->_current_threads_--;
	self->_work_nodes_--;
}

int txThreadManager::numberOfThreads (void)
{
	return _current_threads_;
}

int txThreadManager::numberOfWorkNodes (void)
{
	return _workQueue_.entries();
}

int txThreadManager::numberOfActiveWorkNodes (void)
{
	return _work_nodes_;
}

void txThreadManager::start (TX_THREAD_FUNC func, void* context)
{
	if (!_do_task_exit_flag_)
	{
		WorkNode* work;

		if (!(work = (WorkNode*) THE_WORKNODE_REUSE_LIST.get()))
		{
			work = new WorkNode(func, context);
		}
		else
		{
			work->reset(func, context);
		}

		_addWork_(work);
	}
}

#if defined TX_PREEMPT_SUPPORT

txNativeThreadManager::txNativeThreadManager (const char* name, int min, int max,
	int low, int high, int priority, int size) :

	txThrdBase(name),
	_work_nodes_(0),
	_min_threads_(min),
	_max_threads_(max),
	_current_threads_(0),
	_stack_size_(size),
	_priority_(priority),
	_do_task_exit_flag_(0),
	_workQueue_(low, high)
{
}

void txNativeThreadManager::_addWork_ (WorkNode* worknode)
{
	txNativeEvent* event;

	_workQueue_.put(worknode);

	if (event = (txNativeEvent*) _eventQueue_.get())
	{
		event->trigger();
	}
	else if ((_current_threads_ < _max_threads_) || !_max_threads_)
	{
		_current_threads_++;

		txNativeThread::start(
			txNativeThreadManager::_doWorkThread_,
			this, name(), _priority_, _stack_size_);
	}
}

WorkNode* txNativeThreadManager::_getWork_ (void)
{
	WorkNode* work = 0;

	if (_workQueue_.entries())
	{
		_workQueue_.get((void*&) work);
	}

	return work;
}

txNativeThreadManager::~txNativeThreadManager (void)
{
	txList* queue;
	WorkNode* node;
	txNativeEvent* event;

	_do_task_exit_flag_ = 1;

	_workQueue_.unregisterQueue(queue);

	if (queue)
	{
		while (node = (WorkNode*) queue->get())
		{
			delete node; node = 0;
		}

		delete queue; queue = 0;
	}

	while (event = (txNativeEvent*) _eventQueue_.get())
	{
		event->trigger();
	}
}

void txNativeThreadManager::_doWorkThread_ (void* obj)
{
	int nt;
	int nw;
	WorkNode* work;
	txNativeEvent event("_doWorkThread_");
	txNativeThreadManager* self = (txNativeThreadManager*) obj; 

	self->_work_nodes_++;

	while (!self->_do_task_exit_flag_)
	{
		if (work = self->_getWork_())
		{
			work->func(work->context);

			THE_WORKNODE_REUSE_LIST.add(work);

			txNativeThread::yield();
		}
		else
		{
			nt = self->numberOfThreads();
			nw = self->numberOfWorkNodes();

			if ((nt > self->_min_threads_) && (nw < nt))
			{
				break;
			}

			self->_eventQueue_.append(&event);

			txNativeThread::yield(event);
		}
	}

	self->_current_threads_--;
	self->_work_nodes_--;
}

int txNativeThreadManager::numberOfThreads (void)
{
	return _current_threads_;
}

int txNativeThreadManager::numberOfWorkNodes (void)
{
	return _workQueue_.entries();
}

int txNativeThreadManager::numberOfActiveWorkNodes (void)
{
	return _work_nodes_;
}

void txNativeThreadManager::start (TX_THREAD_FUNC func, void* context)
{
	if (!_do_task_exit_flag_)
	{
		WorkNode* work;

		if (!(work = (WorkNode*) THE_WORKNODE_REUSE_LIST.get()))
		{
			work = new WorkNode(func, context);
		}
		else
		{
			work->reset(func, context);
		}

		_addWork_(work);
	}
}

#endif // TX_PREEMPT_SUPPORT

