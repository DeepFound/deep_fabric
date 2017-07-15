///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txeventq.h"
#include "txthread.h"

txEventQueue::txEventQueue (int high, int low) :
	_put_event_("Queue Put"),
	_get_event_("Queue Get"),
	_high_(high),
	_low_(low)
{
	_queue_ = new txList();
}

txEventQueue::txEventQueue (txList* queue, int high, int low) :
	_put_event_("Queue Put"),
	_get_event_("Queue Get"),
	_queue_(queue),
	_high_(high),
	_low_(low)
{
}

txEventQueue::~txEventQueue (void)
{
	delete _queue_; _queue_ = 0;

	_put_event_.trigger();
	_get_event_.trigger();

	txThread::yield();
}

int txEventQueue::put (void* message, unsigned long time)
{
	int status = -1;

	if (!_queue_)
	{
		return status;
	}

	status = 1;

	_queue_->append((txObject*) message);

	_get_event_.trigger();

	if ( (_queue_ && _high_) && (_queue_->entries() >= _high_) )
	{
		if (time)
		{
			if (txThread::yield(_put_event_, time))
			{
				status = 0;
			}
		}
		else
		{
			txThread::yield(_put_event_);
		}
	}

	return status;
}

int txEventQueue::put (txList* queue)
{
	void* message;
	int status = -1;

	if (!_queue_)
	{
		return status;
	}

	status = 1;

	while (message = queue->get())
	{
		_queue_->append((txObject*) message);
	}

	_get_event_.trigger();

	if ( (_queue_ && _high_) && (_queue_->entries() >= _high_) )
	{
		txThread::yield(_put_event_);
	}

	return status;
}

int txEventQueue::get (void*& message, unsigned long time)
{
	int status = -1;

	while (_queue_ && status == -1)
	{
		if (!(message = _queue_->get()))
		{
			if (time)
			{
				if (!txThread::yield(_get_event_, time))
				{
					return 0;
				}
			}
			else
			{
				txThread::yield(_get_event_);
			}

			if (!_queue_)
			{
				return status;
			}
		}
		else
		{
			status = 1;
		}
	}

	if ( (_queue_ && _high_) && (_queue_->entries() <= _low_) )
	{
		_put_event_.trigger();
	}

	return status;
}

int txEventQueue::get (txList* queue, int num)
{
	void* message;
	int status = -1;

	if (!_queue_)
	{
		return status;
	}

	if (!num)
	{
		num = _queue_->entries();
	}

	for (int i = 0; i < num; i++)
	{
		while ((message = _queue_->get()) == 0)
		{
			if (_high_ && _queue_->entries() <= _low_)
			{
				_put_event_.trigger();
			}

			txThread::yield(_get_event_);

			if (!_queue_)
			{
				return status;
			}
		}

		queue->append((txObject*) message);

		status = 1;
	}

	if ( (_queue_ && _high_) && (_queue_->entries() <= _low_) )
	{
		_put_event_.trigger();
	}

	return status;
}

int txEventQueue::registerQueue (txList* queue)
{
	int status = -1;

	if (!_queue_)
	{
		_queue_ = queue;
		status = 1;
	}

	return status;
}

int txEventQueue::unregisterQueue (txList*& queue)
{
	int status = 1;

	queue = _queue_;

	if (!_queue_)
	{
		status = -1;
	}
	else
	{
		_queue_ = 0;
	}

	_put_event_.trigger();
	_get_event_.trigger();

	return status;
}

int txEventQueue::entries (void)
{
	int entries = 0;

	if (!_queue_)
	{
		return entries;
	}

	entries = _queue_->entries();

	return entries; 
}

void txEventQueue::flush (void)
{
	if (!_queue_)
	{
		return;
	}

	_queue_->clear();

	_put_event_.trigger();
	_get_event_.trigger();
}

#if TX_PREEMPT_SUPPORT

txNativeEventQueue::txNativeEventQueue (int high, int low) :
	_put_event_("Queue Put"),
	_get_event_("Queue Get"),
	_high_(high),
	_low_(low)
{
	_queue_ = new txList();
}

txNativeEventQueue::txNativeEventQueue (txList* queue, int high, int low) :
	_put_event_("Queue Put"),
	_get_event_("Queue Get"),
	_queue_(queue),
	_high_(high),
	_low_(low)
{
}

txNativeEventQueue::~txNativeEventQueue (void)
{
	delete _queue_; _queue_ = 0;

	_put_event_.trigger();
	_get_event_.trigger();

	txNativeThread::yield();
}

int txNativeEventQueue::put (void* message, unsigned long time)
{
	int status = -1;

	if (!_queue_)
	{
		return status;
	}

	status = 1;

	_queue_->append((txObject*) message);

	_get_event_.trigger();

	if ( (_queue_ && _high_) && (_queue_->entries() >= _high_) )
	{
		if (time)
		{
			if (txNativeThread::yield(_put_event_, time))
			{
				status = 0;
			}
		}
		else
		{
			txNativeThread::yield(_put_event_);
		}
	}

	return status;
}

int txNativeEventQueue::put (txList* queue)
{
	void* message;
	int status = -1;

	if (!_queue_)
	{
		return status;
	}

	status = 1;

	while (message = queue->get())
	{
		_queue_->append((txObject*) message);
	}

	_get_event_.trigger();

	if ( (_queue_ && _high_) && (_queue_->entries() >= _high_) )
	{
		txNativeThread::yield(_put_event_);
	}

	return status;
}

int txNativeEventQueue::get (void*& message, unsigned long time)
{
	int status = -1;

	while (_queue_ && status == -1)
	{
		if (!(message = _queue_->get()))
		{
			if (time)
			{
				if (!txNativeThread::yield(_get_event_, time))
				{
					return 0;
				}
			}
			else
			{
				txNativeThread::yield(_get_event_);
			}

			if (!_queue_)
			{
				return status;
			}
		}
		else
		{
			status = 1;
		}
	}

	if ( (_queue_ && _high_) && (_queue_->entries() <= _low_) )
	{
		_put_event_.trigger();
	}

	return status;
}

int txNativeEventQueue::get (txList* queue, int num)
{
	void* message;
	int status = -1;

	if (!_queue_)
	{
		return status;
	}

	if (!num)
	{
		num = _queue_->entries();
	}

	for (int i = 0; i < num; i++)
	{
		while ((message = _queue_->get()) == 0)
		{
			if (_high_ && _queue_->entries() <= _low_)
			{
				_put_event_.trigger();
			}

			txNativeThread::yield(_get_event_);

			if (!_queue_)
			{
				return status;
			}
		}

		queue->append((txObject*) message);

		status = 1;
	}

	if ( (_queue_ && _high_) && (_queue_->entries() <= _low_) )
	{
		_put_event_.trigger();
	}

	return status;
}

int txNativeEventQueue::registerQueue (txList* queue)
{
	int status = -1;

	if (!_queue_)
	{
		_queue_ = queue;
		status = 1;
	}

	return status;
}

int txNativeEventQueue::unregisterQueue (txList*& queue)
{
	int status = 1;

	queue = _queue_;

	if (!_queue_)
	{
		status = -1;
	}
	else
	{
		_queue_ = 0;
	}

	_put_event_.trigger();
	_get_event_.trigger();

	return status;
}

int txNativeEventQueue::entries (void)
{
	int entries = 0;

	if (!_queue_)
	{
		return entries;
	}

	entries = _queue_->entries();

	return entries; 
}

void txNativeEventQueue::flush (void)
{
	if (!_queue_)
	{
		return;
	}

	_queue_->clear();

	_put_event_.trigger();
	_get_event_.trigger();
}

#endif // TX_PREEMPT_SUPPORT

