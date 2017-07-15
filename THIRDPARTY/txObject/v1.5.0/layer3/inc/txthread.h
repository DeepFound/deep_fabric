///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXTHEARD_H__ )
#define __TXTHEARD_H__

class txEvent;

typedef void (*TX_THREAD_FUNC) (void*);

class txThread
{
	public:
		static const int DEFAULT_PRIORITY;
		static const int DEFAULT_STACK_SIZE;

	public:
		static void start (
			TX_THREAD_FUNC func,
			void* context = 0,
			const char* name = "default",
			int priority = DEFAULT_PRIORITY,
			int stack_size = DEFAULT_STACK_SIZE);

		static int priority (void);

		static int priority (int pri);

		static unsigned long id (void);

		static const char* name (void);

		static const char* name (const char* name);

		static void yield (void);

		static void yield (long time);

		static void yield (txEvent& event);

		static int yield (txEvent& event, long time);

	public:
		// for experience users
		static void schedule (void);
};

#if defined TX_PREEMPT_SUPPORT

class txNativeEvent;

class txNativeThread
{
	public:
		static const int DEFAULT_PRIORITY;
		static const int DEFAULT_STACK_SIZE;

	public:
		static void start (
			TX_THREAD_FUNC func,
			void* context = 0,
			const char* name = "default",
			int priority = DEFAULT_PRIORITY,
			int stack_size = DEFAULT_STACK_SIZE);

		static int priority (void);

		static int priority (int pri);

		static unsigned long id (void);

		static const char* name (void);

		static const char* name (const char* name);

		static void yield (void);

		static void yield (long time);

		static void yield (txNativeEvent& event);

		static int yield (txNativeEvent& event, long time);
};

#endif // TX_PREEMPT_SUPPORT

#endif // __TXTHREAD_H__
