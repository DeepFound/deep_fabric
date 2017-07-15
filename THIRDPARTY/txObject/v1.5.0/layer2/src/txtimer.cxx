///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	#include <sys/time.h>

	static struct timeval TX_CURRENT_TIME;
#endif

#if defined TX_WIN
	#include <sys/timeb.h>

	static struct timeb TX_CURRENT_TIME;
#endif

#include "txlist.h"
#include "txtimer.h"

static txList THE_TIMER_LIST;
static txListIterator THE_TIMER_ITER(THE_TIMER_LIST);

static void setHighestMatchingIndex (long long time, txTimer* timer)
{
	int lo = 0;
	txTimer* timer_ptr;
	int hi = THE_TIMER_LIST.entries();

	if (!hi || (time <= ((txTimer*) THE_TIMER_LIST.first())->expiryTime()))
	{
		THE_TIMER_LIST.insertAt(0, (txObject*) timer);
		return;
	}

	THE_TIMER_ITER.reset();

	for (; timer_ptr = (txTimer*) THE_TIMER_ITER.next(); lo++)
	{
		if (time <= timer_ptr->expiryTime())
		{
			break;
		}	
	}

	THE_TIMER_LIST.insertAt(lo, (txObject*) timer);
}

txTimer::txTimer (void) :
	_interval_(0),
	_function_(0),
	_context_(0),
	_xprtime_(0),
	_status_(txTimerEnum::STOPPED)
{
}
	
txTimer::txTimer (TX_TIMER_FUNC function, void* context, long interval) :
	_interval_(interval),
	_function_(function),
	_context_(context),
	_xprtime_(0)
{
	_setupTimer_();
}
	
txTimer::~txTimer (void)
{
	clear();
}

void txTimer::_setupTimer_ (void)
{
	_status_ = txTimerEnum::RUNNING;

	_xprtime_ = currentTime() + _interval_;

	setHighestMatchingIndex(_xprtime_, this);
}

void txTimer::_trigger_ (long long now)
{
	if (_status_ == txTimerEnum::RUNNING)
	{
		if (_function_(_context_) == txTimerEnum::CONTINUE)
		{
			_status_ = txTimerEnum::RUNNING;

			if (_interval_ == 0)
			{
				_xprtime_ = now + 1;
			}
			else
			{
				while ((_xprtime_ += _interval_) < now);
			}

			setHighestMatchingIndex(_xprtime_, this);
		}
		else
		{
			_status_ = txTimerEnum::STOPPED;
		}
	}
}

void txTimer::trigger (long long now)
{
	THE_TIMER_LIST.removeReference((txObject*) this);

	_trigger_(now);
}

void txTimer::reset (TX_TIMER_FUNC function, void* context, long interval)
{
	if (interval)
	{
		clear();

		_interval_ = interval;

		if (function)
		{
			_function_ = function;
		}

		if (context)
		{
			_context_ = context;
		}

		_setupTimer_();
	}
}

void txTimer::clear (void)
{
	if (_status_ == txTimerEnum::RUNNING)
	{
		_status_ = txTimerEnum::STOPPED;
	}

	THE_TIMER_LIST.removeReference((txObject*) this);
}

void txTimer::_syncTimers_ (long long btotal)
{
	txTimer* timer;

	long asecs, amsecs;

	txTimer::currentTime(asecs, amsecs);

	long long delta, atotal = (asecs * 1000) + amsecs;  

	if (delta = (long long) (btotal - atotal))
	{
		txListIterator iter(THE_TIMER_LIST);

		while (timer = (txTimer*) iter.next())
		{
			timer->_xprtime_ = timer->_xprtime_ - delta;
		}
	}
}

long txTimer::getMinWaitTime (void)
{
	long min_time = -1;

	if (THE_TIMER_LIST.entries())
	{
		long long now = currentTime();

		txTimer* t = (txTimer*) THE_TIMER_LIST.first();

		if (now > t->expiryTime())
		{
			min_time = 0;
		}
		else
		{
			min_time = (long) (t->expiryTime() - now);

			if ((min_time < 0 ) || (min_time > ((signed)t->interval())))
			{
				min_time = 1;

				txListIterator iter(THE_TIMER_LIST);

				while (t = (txTimer*) iter.next())
				{
					t->_xprtime_ = t->interval() + now;
				}
			}
		}
	}

	return min_time;
}

bool txTimer::processExpiredTimers (void)
{
	bool processed = false;

	if (THE_TIMER_LIST.entries() != 0)
	{
		long long now = currentTime();

		while (((txTimer*) THE_TIMER_LIST.first())->expiryTime() <= now)
		{
			((txTimer*) THE_TIMER_LIST.get())->_trigger_(now);

			if (THE_TIMER_LIST.entries() == 0)
			{
				processed = true;
				break;
			}
		}
	}

	return processed;
}

long txTimer::processAndGetMinWait (void)
{
	processExpiredTimers();

	return (long) getMinWaitTime();
}

int txTimer::currentTimeZone (void)
{
	int current_zone = 0;

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	struct timezone tz_var;
	struct timeval  tv_var;

	// If either tv or tz is NULL, the corresponding structure is not set or
	// returned.  (However, compilation warnings will result if tv is NULL.)

	gettimeofday(&tv_var, &tz_var);

	current_zone = tz_var.tz_minuteswest;
#endif

#if defined TX_WIN
#endif

	return current_zone;
}

void txTimer::currentTimeZone (int& mw, int& dst)
{
#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	struct timezone tz_var;

	struct timeval  tv_var;

	// If either tv or tz is NULL, the corresponding structure is not set or
	// returned.  (However, compilation warnings will result if tv is NULL.)

	gettimeofday(&tv_var, &tz_var);

	dst = tz_var.tz_dsttime;
	mw  = tz_var.tz_minuteswest;
#endif

#if defined TX_WIN
	dst = 0; mw = 0;
#endif
}

long long txTimer::currentTime (void)
{
	long long current_time;

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	gettimeofday(&TX_CURRENT_TIME, (struct timezone*) 0);

	current_time = TX_CURRENT_TIME.tv_sec; 
	current_time = (current_time * 1000) + (TX_CURRENT_TIME.tv_usec / 1000); 
#endif

#if defined TX_WIN
	ftime(&TX_CURRENT_TIME);

	current_time = TX_CURRENT_TIME.time; 
	current_time = (current_time * 1000) + (TX_CURRENT_TIME.millitm); 
#endif

	return current_time;
}

void txTimer::currentTime (long &sec, long &msec)
{
#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	gettimeofday(&TX_CURRENT_TIME, (struct timezone*) 0);

	sec = (long) TX_CURRENT_TIME.tv_sec;
	msec = (long) TX_CURRENT_TIME.tv_usec / 1000;
#endif

#if defined TX_WIN
	ftime(&TX_CURRENT_TIME);

	sec = (long) TX_CURRENT_TIME.time;
	msec = (long) TX_CURRENT_TIME.millitm;
#endif
}

void txTimer::setTimeZone (int mw, int dst)
{
	long long before_time = txTimer::currentTime();

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	struct timezone tz_var;
	tz_var.tz_dsttime = dst;
	tz_var.tz_minuteswest = mw;

	settimeofday((struct timeval*) 0, &tz_var);
#endif

#if defined TX_WIN
#endif

	txTimer::_syncTimers_(before_time);
}

void txTimer::setTimeOfDay (long secs, long msecs)
{
	long long before_time = txTimer::currentTime();

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	struct timeval tv_var;

	tv_var.tv_sec = secs;
	tv_var.tv_usec = msecs * 1000;

	settimeofday(&tv_var, (struct timezone*) 0);
#endif

#if defined TX_WIN
#endif

	txTimer::_syncTimers_(before_time);
}

