///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if !defined ( __EVENT_H__ )
#define __EVENT_H__

class Thread;
class EventThreadList;

class Event
{
	private:
		const char* _name_;
		EventThreadList* _threads_waiting_;

	private:
		static void _remove_ (Event* event, Thread* thread);

	public:
		Event (const char* name = 0);
		~Event (void);

		const char* name (void) const { return _name_; };
		void trigger (void);
		void wait (void);

	friend class Thread;
};

#endif // __EVENT_H__
