///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXEVENTQ_H__ )
#define __TXEVENTQ_H__

#include "txlist.h"
#include "txevent.h"
#include "sys/txthrdbase.h"

class txEventQueue : public txThrdBase
{
	private:
		int _low_;
		int _high_;
		txList* _queue_;
		txEvent _put_event_;
		txEvent _get_event_;

	public:
		txEventQueue (int high = 0, int low = 0);
		txEventQueue (txList* queue, int high = 0, int low = 0);

		~txEventQueue (void);

		int put (void* message, unsigned long time = 0);
		int put (txList* queue);

		int get (void*& message, unsigned long time = 0);
		int get (txList* queue, int num = 0);

		int registerQueue (txList* queue);
		int unregisterQueue (txList*& queue);

		int entries (void);

		void flush (void);
};

#if defined TX_PREEMPT_SUPPORT

class txNativeEventQueue : public txThrdBase
{
	private:
		int _low_;
		int _high_;
		txList* _queue_;
		txNativeEvent _put_event_;
		txNativeEvent _get_event_;

	public:
		txNativeEventQueue (int high = 0, int low = 0);
		txNativeEventQueue (txList* queue, int high = 0, int low = 0);

		~txNativeEventQueue (void);

		int put (void* message, unsigned long time = 0);
		int put (txList* queue);

		int get (void*& message, unsigned long time = 0);
		int get (txList* queue, int num = 0);

		int registerQueue (txList* queue);
		int unregisterQueue (txList*& queue);

		int entries (void);

		void flush (void);
};

#endif // TX_PREEMPT_SUPPORT

#endif // __TXEVENTQ_H__
