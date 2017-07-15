///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXTHRDSS_H__ )
#define __TXTHRDSS_H__

class txThreadSS
{
	private:
		static int _shutdown_;

		static void _process_ (void*);

		#if defined TX_NON_PREEMPT_SUPPORT
			static void _initHomeThreading_ (void);
		#endif

		#if !defined TX_NON_PREEMPT_SUPPORT
			#if defined TX_PREEMPT_SUPPORT
				static void _initNativeThreading_ (void);
			#endif
		#endif

	public:
		static void startup (void);

		static void shutdown (void);
		
		static int isShutdown (void)
		{
			return _shutdown_;
		}
};

#endif // __TXTHRDSS_H__
