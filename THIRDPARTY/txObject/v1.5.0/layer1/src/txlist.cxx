///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txlist.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txList,txObject)

txListNode::txListNode (void) :
	_info(0), _next(0)
{
}

txListNode::txListNode (txObject* x) :
	_info(x), _next(0)
{
}

txListNode::~txListNode (void)
{
}

void txList::_init_ (void)
{
	_head_._next = _tail_._next = &_tail_;
	_last_ = &_head_;
	_entries_ = 0;
}

const txListNode* txList::_at_ (int i) const
{
	if (i >= entries())
	{
		return 0;
	}

	register txListNode* node = _head_._next;

	while (i--)
	{
		node = node->_next;
	}

	return node;
}

void txList::_insertAfterNode_ (txListNode* l, txListNode* x)
{
	x->_next = l->_next;
	l->_next = x;

	if (l == _last_)
	{
		_last_ = x;
	}

	_entries_++;
}

txListNode* txList::_findLeft_ (const txListNode* x) const
{
	register txListNode* node = (txListNode*) &_head_;

	while (node->_next != &this->_tail_)
	{
		if (node->_next == x)
		{
			return node;
		}

		node = node->_next;
	}

	return 0;
}

txListNode* txList::_removeRight_ (txListNode* x)
{
	txListNode* node = x->_next;

	x->_next = node->_next;

	if (node == _last_)
	{
		_last_ = x;
	}

	_entries_--;

	return node;
}

txObject* txList::_stripNode_ (txListNode* node)
{
	if (!node) return 0;

	txObject* p = node->_info;
	delete node;
	return p;
}

const txObject* txList::append (txObject* x)
{
	#if !defined OSTORE_SUPPORT
		_insertAfterNode_(_last_, new txListNode(x));
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_insertAfterNode_(_last_,
				new(TX_GET_OBJECT_CLUSTER(this),
					txListNode::get_os_typespec())
						txListNode(x));
		}
		else
		{
			_insertAfterNode_(_last_, new txListNode(x));
		}
	#endif

	return x;
}

const txObject* txList::prepend (txObject* x)
{
	#if !defined OSTORE_SUPPORT
		_insertAfterNode_(&_head_, new txListNode(x));
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_insertAfterNode_(&_head_,
				new(TX_GET_OBJECT_CLUSTER(this),
					txListNode::get_os_typespec())
						txListNode(x));
		}
		else
		{
			_insertAfterNode_(&_head_, new txListNode(x));
		}
	#endif

	return x;
}

int txList::index (const txObject* x) const
{
	txListNode* node = _head_._next;

	for (int i = 0; node != &_tail_; i++)
	{
		if (node->_info == x)
		{
			return i;
		}

		node = node->next();
	}

	return -1;
}

const txObject* txList::insertAt (int i, txObject* x)
{
	if (i > entries())
	{
		return 0;
	}

	txListNode* prev = i ? (txListNode*) _at_(i-1) : &_head_;

	#if !defined OSTORE_SUPPORT
		_insertAfterNode_(prev, new txListNode(x));
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_insertAfterNode_(prev,
				new(TX_GET_OBJECT_CLUSTER(this),
					txListNode::get_os_typespec())
						txListNode(x));
		}
		else
		{
			_insertAfterNode_(prev, new txListNode(x));
		}
	#endif

	return x;
}

const txObject* txList::insertAfter (int i, txObject* x)
{
	if (i > entries())
	{
		return 0;
	}

	#if !defined OSTORE_SUPPORT
		_insertAfterNode_((txListNode*)_at_(i),
			new txListNode(x));
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_insertAfterNode_((txListNode*)_at_(i),
				new(TX_GET_OBJECT_CLUSTER(this),
					txListNode::get_os_typespec())
						txListNode(x));
		}
		else
		{
			_insertAfterNode_((txListNode*)_at_(i),
				new txListNode(x));
		}
	#endif

	return x;
}

const txObject* txList::findReference (const txObject* x) const
{
	register txListNode* node = _head_._next;

	while (node != &_tail_)
	{
		if (node->_info == x)
		{
			return node->_info;
		}

		node = node->next();
	}

	return 0;
}

txObject* txList::removeReference (const txObject* x)
{
	register txListNode* prev = &_head_;

	while (prev != _last_)
	{
		if (prev->next()->_info == x)
		{
			return _stripNode_(_removeRight_(prev));
		}

		prev = prev->next();
	}

	return 0;
}

const txObject* txList::find (const txObject* x) const
{
	register txListNode* node = _head_._next;
	register TX_COMPARE_FLAG flag = TX_GET_COMPARE_FLAG(_flags_);

	while (node != &_tail_)
	{
		if (flag == TX_COMPARE_V2K)
		{
			if (_compare_(node->_info, x) == 1)
			{
				return node->_info;
			}
		}
		else // TX_COMPARE_K2V is true
		{
			if (_compare_(x, node->_info) == 1)
			{
				return node->_info;
			}
		}

		node = node->next();
	}

	return 0;
}

txObject* txList::remove (const txObject* x)
{
	register txListNode* prev = &_head_;
	register TX_COMPARE_FLAG flag = TX_GET_COMPARE_FLAG(_flags_);

	while (prev != _last_)
	{
		if (flag == TX_COMPARE_V2K)
		{
			if (_compare_(prev->next()->_info, x) == 1)
			{
				return _stripNode_(_removeRight_(prev));
			}
		}
		else // TX_COMPARE_K2V is true
		{
			if (_compare_(x, prev->next()->_info) == 1)
			{
				return _stripNode_(_removeRight_(prev));
			}
		}

		prev = prev->next();
	}

	return 0;
}

void txList::clear (void)
{
	register txListNode* n;
	register txListNode* node = _head_._next;

	while (node != &_tail_)
	{
		n = node->next();
		delete node;
		node = n;
	}

	_init_();
}

void txList::clearAndDestroy (void)
{
	register txListNode* n;
	register txListNode* node = _head_._next;

	while (node != &_tail_)
	{
		n = node->next();
		delete node->_info;
		delete node;
		node = n;
	}

	_init_();
}

void txList::readObject (txIn& in)
{
	register const char* end = in.cursor() + in.objectLength();

	signed short int flag; in >> flag;

	TX_SET_AUTODEL_FLAG(_flags_, (TX_AUTODEL_FLAG) flag);

	register txObject* o;

	while (in.cursor() < end)
	{
		in >> o;

		append(o);
	}
}


void txList::writeObject (txOut& out) const
{
	register txObject* node;
	unsigned int hdr = out.length();

	out.putNull();

	out << (signed short int) TX_GET_AUTODEL_FLAG(_flags_);

	txListIterator iter((txList&) *this);

	while (node = (txObject*) iter.next())
	{
		out.put(node);
	}

	out.writeHeader(hdr, out.length() - (hdr + 4), TX_LIST);
}

txObject* txListIterator::remove (void)
{
	if (!_active_()) return 0;

	_c_ = _l_->_findLeft_(_c_);

	return txList::_stripNode_(_l_->_removeRight_(_c_));
}

