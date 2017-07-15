///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if !defined ( __THRDMGR_H__ )
#define __THRDMGR_H__

#if defined TX_NON_PREEMPT_DEBUG
class Thread;
class txHashSet;

class ThrdMgr
{
	private:
		static txHashSet* _threads_;

	private:
		static void _startup_ (void);
		static void _shutdown_ (void);

	public:
		static void add (Thread* t);
		static void remove (Thread* t);

		static void log (void);
};
#endif

#endif // __THRDMGR_H__
