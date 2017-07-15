///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if defined TX_NON_PREEMPT_STATS
#if !defined ( __THRD_STATS_H__ )
#define __THRD_STATS_H__

class ThreadStats
{
	public: 
		ThreadStats (void);

		long long getRunCPU (void);

		long long getRunTime (void);

		long long getWaitTime (void);

		long long getAliveTime (void);

		long long getActiveTime (void);

		long long getNumYields (void);

	private:
		long long _runCPU_;
		long long _runTime_;
		long long _waitTime_;
		long long _numYields_;
		long long _activeTime_;
		long long _aliveTimeStart_;
		long long _intervalCPUStart_;
		long long _intervalTimeStart_;

	private:
		void _startIntervalTime_ (void);
		void _startIntervalCPU_ (void);

		long long _getIntervalTime_ (void);
		long long _getIntervalCPU_ (void);

	friend class Thread;
};

#endif // __THRD_STATS_H__
#endif // TX_NON_PREEMPT_STATS
