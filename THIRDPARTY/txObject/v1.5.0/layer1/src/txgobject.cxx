///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txstring.h"
#include "txgobject.h"

TX_DEFINE_STREAM_TYPE(txGObject,txStream)

txGObject::txGObject (void) :
	_length_(0)
{
}

txGObject::txGObject (const txUInt32& type, unsigned long length) :
	_type_(type), _length_(length)
{
}

txGObject::~txGObject (void)
{
	_attrs_.clearAndDestroy();
}


void txGObject::writeObject (txOut& out) const
{
	unsigned int h_loc = out.length();

	out.putNull();

	out.writeUInt(_type_.value());

	storeInners(out);

	out.writeHeader(h_loc, out.length()-(h_loc+4), TX_GTYPE_INT32);
}

void txGObject::storeInners (txOut& out) const
{
	txObject* o;
	txListIterator iter((txList&) _attrs_);

	while (o = (txObject*) iter.next())
	{
		out << o; 
	}
}

void txGObject::restoreInners (txIn& in)
{
	txObject* o;
	const char* end = in.cursor() + _length_;

	while (in.cursor() < end)
	{
		in >> o;

		_attrs_.append(o);
	}
}

