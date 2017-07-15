///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXTIMER_H__ )
#define __TXTIMER_H__

class txTimerEnum
{
	public:
		enum STATUS
		{
			STOPPED = 0, RUNNING = 1
		};

		enum RETURN_STATUS
		{
			STOP = 0, CONTINUE = 1
		};
};

typedef txTimerEnum::RETURN_STATUS (*TX_TIMER_FUNC) (void*);

class txTimer
{
	private:
		long _interval_;
		long long _xprtime_;
		txTimerEnum::STATUS _status_;

		void* _context_;
		TX_TIMER_FUNC _function_;

		inline void _setupTimer_ (void);
		inline void _trigger_ (long long now);

		txTimer (const txTimer& obj);
		txTimer& operator= (const txTimer& obj);

	public:
		txTimer (void);

		txTimer (
			TX_TIMER_FUNC func,
			void* context,
			long interval);

		~txTimer (void);

		long long expiryTime (void) const
		{
			return _xprtime_;
		}

		txTimerEnum::STATUS status (void) const
		{
			return _status_;
		}

		void* context (void) const
		{
			return _context_;
		}

		long interval (void) const
		{
			return _interval_;
		}

		void trigger (long long now);

		void clear (void);

		void reset (
			TX_TIMER_FUNC func = 0,
			void* context = 0,
			long interval = 0);

	private:
		static void _syncTimers_ (long long);

	public:
		static long getMinWaitTime (void);

		static bool processExpiredTimers (void);

		static long processAndGetMinWait (void);

		static int currentTimeZone (void);

		static void currentTimeZone (int& mw, int& dst);

		static long long currentTime (void);

		static void currentTime (long& s, long& ms);

		static void setTimeZone (int mw, int dst = 0);

		static void setTimeOfDay (long sec, long ms);
};

#endif // __TXTIMER_H__
