///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXEVENT_H__ )
#define __TXEVENT_H__

#include "sys/txthrdbase.h"

class txEvent : public txThrdBase
{
	private:
		txEvent (const txEvent&);
		txEvent& operator= (const txEvent&);

	public:
		txEvent (const char* name = "default");
		~txEvent (void);

		int wait (long timeout);
		void wait (void);

		void trigger (void);
};

#if defined TX_PREEMPT_SUPPORT

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	#include <pthread.h>
#else
	#include <windows.h>
#endif

class txNativeEvent : public txThrdBase
{
	private:
		#if defined TX_WIN
			HANDLE _cond_;
		#elif defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
			int _signalled_;
			pthread_cond_t _cond_;
			pthread_mutex_t _mutex_;
		#endif

		txNativeEvent (const txNativeEvent&);
		txNativeEvent& operator= (const txNativeEvent&);

	public:
		txNativeEvent (const char* name = "default");

		~txNativeEvent (void);

		int wait (long time);
		void wait (void);

		void trigger (void);
};

#endif // TX_PREEMPT_SUPPORT

#endif // __TXEVENT_H__
