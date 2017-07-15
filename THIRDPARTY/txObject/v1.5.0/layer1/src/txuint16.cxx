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

#include "txstring.h"
#include "txuint16.h"

TX_DEFINE_PARENT_TYPE(txUInt16,txObject)

txUInt16::txUInt16 (void) :
	_int_(0)
{
}

txUInt16::txUInt16 (unsigned short v) :
	_int_(v)
{
}

txUInt16::txUInt16 (const txUInt16& obj)
{
	_int_ = obj._int_;
}

txUInt16& txUInt16::operator= (txUInt16& obj)
{
	if (this != &obj)
	{
		obj._int_ = obj._int_;
	}

	return *this;
}

txUInt16::~txUInt16 (void)
{
}

int txUInt16::equals (const txObject* obj) const
{
	return obj->isClass(txUInt16::Type) ?
		(_int_ == ((txUInt16*) obj)->_int_) : 0;
}

int txUInt16::compare (const txObject* obj) const
{
	if (_int_ == ((txUInt16*) obj)->_int_)
	{
		return 0;
	}
	else if (_int_ > ((txUInt16*) obj)->_int_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txUInt16::readObject (txIn& in)
{
	_int_ = (unsigned short) ntohl(*(unsigned long*) (in.cursor()));
}

void txUInt16::writeObject (txOut& out) const
{
	out << _int_;
}

