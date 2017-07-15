///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	#include <sys/types.h>
	#include <netinet/in.h>
#else
	#include <winsock2.h>
#endif

#include "txint16.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txInt16,txObject)

txInt16::txInt16 (void) :
	_int_(0)
{
}

txInt16::txInt16 (signed short v) :
	_int_(v)
{
}

txInt16::txInt16 (const txInt16& obj)
{
	_int_ = obj._int_;
}

txInt16& txInt16::operator= (txInt16& obj)
{
	if (this != &obj)
	{
		obj._int_ = obj._int_;
	}

	return *this;
}

txInt16::~txInt16 (void)
{
}

int txInt16::equals (const txObject* obj) const
{
	return obj->isClass(txInt16::Type) ?
		(_int_ == ((txInt16*) obj)->_int_) : 0;
}

int txInt16::compare (const txObject* obj) const
{
	if (_int_ == ((txInt16*) obj)->_int_)
	{
		return 0;
	}
	else if (_int_ > ((txInt16*) obj)->_int_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txInt16::readObject (txIn& in)
{
	_int_ = (signed short) ntohl(*(unsigned long*) (in.cursor()));
}

void txInt16::writeObject (txOut& out) const
{
	out << _int_;
}

