///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXTHRDMGR_H__ )
#define __TXTHRDMGR_H__

#include "txevent.h"
#include "txeventq.h"
#include "txthread.h"
#include "sys/txthrdbase.h"

class WorkNode;

class txThreadManager : public txThrdBase
{
	private:
		int _priority_;
		int _stack_size_;
		int _work_nodes_;
		int _min_threads_;
		int _max_threads_;
		int _current_threads_;
		int _do_task_exit_flag_;

		txList _eventQueue_;
		txEventQueue _workQueue_;

	private:
		inline void _addWork_ (WorkNode* worknode);
		inline WorkNode* _getWork_ (void);
		static void _doWorkThread_ (void*);

		txThreadManager (const txThreadManager&);
		txThreadManager& operator= (const txThreadManager&);

	public:
		txThreadManager (
			const char* name = "default",
			int min_threads = 2,
			int max_threads = 5,
			int low_water_mark = 0,
			int high_water_mark = 0,
			int priority = txThread::DEFAULT_PRIORITY,
			int stack = txThread::DEFAULT_STACK_SIZE);

		~txThreadManager (void);

		int numberOfThreads (void);
		int numberOfWorkNodes (void);
		int numberOfActiveWorkNodes (void);

		void start (
			TX_THREAD_FUNC func,
			void* context = 0);
};

#if defined TX_PREEMPT_SUPPORT

class txNativeThreadManager : public txThrdBase
{
	private:
		int _priority_;
		int _stack_size_;
		int _work_nodes_;
		int _min_threads_;
		int _max_threads_;
		int _current_threads_;
		int _do_task_exit_flag_;

		txList _eventQueue_;
		txNativeEventQueue _workQueue_;

	private:
		inline void _addWork_ (WorkNode* worknode);
		inline WorkNode* _getWork_ (void);
		static void _doWorkThread_ (void*);

		txNativeThreadManager (const txThreadManager&);
		txNativeThreadManager&operator=(const txNativeThreadManager&);

	public:
		txNativeThreadManager (
			const char* name = "default",
			int min_threads = 2,
			int max_threads = 5,
			int low_water_mark = 0,
			int high_water_mark = 0,
			int priority = txNativeThread::DEFAULT_PRIORITY,
			int stack = txNativeThread::DEFAULT_STACK_SIZE);

		~txNativeThreadManager (void);

		int numberOfThreads (void);
		int numberOfWorkNodes (void);
		int numberOfActiveWorkNodes (void);

		void start (
			TX_THREAD_FUNC func,
			void* context = 0);
};

#endif // TX_PREEMPT_SUPPORT

#endif // __TXTHRDMGR_H__
