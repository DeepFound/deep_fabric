///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXLOCK_H__ )
#define __TXLOCK_H__

#include "sys/txthrdbase.h"

class txLock : public txThrdBase
{
	private:
		unsigned long _lock_id_;
		unsigned long _lock_count_;

		txLock (const txLock&);
		txLock& operator= (const txLock&);

	public:
		txLock (const char* name = "default");
		~txLock (void);

		void acquire (void);
		void release (void);

		void acquire (unsigned long id);
		void release (unsigned long id);

		unsigned long locked (void) { return _lock_id_; }
};

#if defined TX_PREEMPT_SUPPORT

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	#include <pthread.h>
#else
	#include <windows.h>
#endif

class txNativeLock : public txThrdBase
{
	private:
		unsigned long _lock_id_;
		unsigned long _lock_count_;

		#if defined TX_WIN
			HANDLE _mutex_;
		#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
			pthread_mutex_t _mutex_;
		#endif

	public:
		txNativeLock (const char* name = "default");

		~txNativeLock (void);

		void acquire (void);
		void release (void);

		void acquire (unsigned long id);
		void release (unsigned long id);

		unsigned long locked (void) { return _lock_id_; }
};

#endif // TX_PREEMPT_SUPPORT

#endif // __TXLOCK_H__
