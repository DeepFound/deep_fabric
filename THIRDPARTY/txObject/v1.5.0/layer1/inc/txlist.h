///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if !defined ( __TXLIST_H__ )
#define __TXLIST_H__

#include "txobject.h"

class txListNode : public txObject
{
	TX_PERSIST

	public:
		txObject* _info;
		txListNode* _next;

	public:
		txListNode (void);

		txListNode (txObject* x);

		~txListNode (void);

		txListNode* next (void) const
		{
			return _next;
		}

		txListNode* clear (void)
		{
			txListNode* node = _next;
			_next = 0;
			return node;
		}
};

class txList : public txObjectSequence
{
	TX_DEFINE_TYPE(txList)

	private:
		int _entries_;
		txListNode _head_;
		txListNode _tail_;
		txListNode* _last_;
		unsigned long _flags_;

	private:
		void _init_ (void);

		const txListNode* _at_ (int i) const;

		void _insertAfterNode_ (txListNode* l, txListNode* x);

		txListNode* _findLeft_ (const txListNode* x) const;

		txListNode* _removeRight_ (txListNode* x);

		txListNode* _removeFirst_ (void)
		{
			return (!entries() ? 0 : _removeRight_(&_head_));
		}

		static txObject* _stripNode_ (txListNode* node);

		static int _compare_ (const txObject* v1, const txObject* v2)
		{
			return (v1->equals(v2));
		}

	public:
		txList (
			TX_AUTODEL_FLAG autodel = TX_AUTODEL_OFF,
			TX_COMPARE_FLAG compare = TX_COMPARE_V2K)
		{
			TX_SET_AUTODEL_FLAG(_flags_, autodel);
			TX_SET_COMPARE_FLAG(_flags_, compare);

			_init_();
		}

		~txList (void)
		{
			if (TX_GET_AUTODEL_FLAG(_flags_))
			{
				clearAndDestroy();
			}
			else
			{
				clear();
			}
		}

		const txObject* at (int n) const
		{
			return _at_(n)->_info;
		}

		const txObject* first (void) const
		{	
			return (!entries() ? 0 : _head_._next->_info);
		}

		const txObject* last (void) const
		{
			return (!entries() ? 0 : _last_->_info);
		}

		const txObject* append (txObject* x);

		const txObject* prepend (txObject* x);

		int index (const txObject* x) const;

		const txObject* insertAt (int i, txObject* x);

		const txObject* insertAfter (int i, txObject* x);

		const txObject* findReference (const txObject* x) const;

		txObject* removeReference (const txObject* x);

		virtual const txObject* find (const txObject* x) const;

		virtual txObject* remove (const txObject* x);

		virtual const txObject* insert (txObject* x)
		{
			return append(x);
		}

		virtual void removeAndDestroy (const txObject* x)
		{
			txObject* node;

			node = txList::remove(x);

			delete node; node = 0;
		}

		txObject* get (void)
		{
			return _entries_ ? _stripNode_(_removeFirst_()) : 0;
		}

		int entries (void) const
		{
			return _entries_;
		}

		void clear (void);

		void clearAndDestroy (void);

		void readObject (txIn& in);
		void writeObject (txOut& out) const;

	friend class txListIterator;
};

TX_DECLARE_STREAM_TYPE_OPER(txList)

class txListIterator : public txObjectIterator
{
	TX_PERSIST

	private:
		txList* _l_;
		txListNode* _c_;

	private:
		void _forward_ (void)
		{
			_c_ = _c_->_next;
		}

		int _active_ (void) const
		{
			return _c_ != &_l_->_head_ && _c_ != &_l_->_tail_;
		}

	public:
		txListIterator (void) :
			_l_(0), _c_(0)
		{
		}

		txListIterator (txList &s) :
			_l_(&s), _c_(&s._head_)
		{
		}

		const txObject* key (void) const
		{
			return (_active_() ? _c_->_info : 0);
		}

		const txObject* next (void)
		{
			_forward_();

			return (_c_ == &_l_->_tail_ ? 0 : _c_->_info);
		}

		txObject* remove (void);

		void reset (void)
		{
			_c_ = &_l_->_head_;
		}

		void reset (txList &l)
		{
			_l_ = &l;
			reset();
		}
};

#endif // __TXLIST_H__
