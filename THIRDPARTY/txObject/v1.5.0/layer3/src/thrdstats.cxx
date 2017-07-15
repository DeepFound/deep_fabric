///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if defined TX_NON_PREEMPT_STATS
#include "txtimer.h"
#include "thrdstats.h"

long long ThreadStats::getRunCPU (void)
{
	return _getIntervalCPU_() + _runCPU_;
}

long long ThreadStats::getRunTime (void)
{
	return _getIntervalTime_() + _runTime_;
}

long long ThreadStats::getActiveTime (void)
{
	return _activeTime_;
}

long long ThreadStats::getAliveTime (void)
{
	return txTimer::currentTime() - _aliveTimeStart_;
}

long long ThreadStats::getWaitTime (void)
{
	return _waitTime_;
}

long long ThreadStats::getNumYields (void)
{
	return _numYields_;
}

void ThreadStats::_startIntervalTime_ (void)
{
	_intervalTimeStart_ = txTimer::currentTime();
}

long long ThreadStats::_getIntervalTime_ (void)
{
	return txTimer::currentTime() - _intervalTimeStart_;
}

void ThreadStats::_startIntervalCPU_ (void)
{ 
	_intervalCPUStart_ = txTimer::currentTime();
}

long long ThreadStats::_getIntervalCPU_ (void)
{
	return txTimer::currentTime() - _intervalCPUStart_;
}

ThreadStats::ThreadStats (void) : 
	_runCPU_(0),
	_runTime_(0),
	_numYields_(0),
	_waitTime_(0),
	_activeTime_(0),
	_intervalCPUStart_(0)
{
	_intervalTimeStart_ = _aliveTimeStart_ = txTimer::currentTime();
}

#endif // TX_NON_PREEMPT_STATS
