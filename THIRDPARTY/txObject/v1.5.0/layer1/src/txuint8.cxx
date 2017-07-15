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

#include "txuint8.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txUInt8,txObject)

txUInt8::txUInt8 (void) :
	_int_(0)
{
}

txUInt8::txUInt8 (unsigned char v) :
	_int_(v)
{
}

txUInt8::txUInt8 (const txUInt8& obj)
{
	_int_ = obj._int_;
}

txUInt8& txUInt8::operator= (txUInt8& obj)
{
	if (this != &obj)
	{
		obj._int_ = obj._int_;
	}

	return *this;
}

txUInt8::~txUInt8 (void)
{
}

int txUInt8::equals (const txObject* obj) const
{
	return obj->isClass(txUInt8::Type) ?
		(_int_ == ((txUInt8*) obj)->_int_) : 0;
}

int txUInt8::compare (const txObject* obj) const
{
	if (_int_ == ((txUInt8*) obj)->_int_)
	{
		return 0;
	}
	else if (_int_ > ((txUInt8*) obj)->_int_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txUInt8::readObject (txIn& in)
{
	_int_ = *(in.cursor() + 3);
}

void txUInt8::writeObject (txOut& out) const
{
	out.writeRawHeader(htonl(TX_UNSIGNED_INT8));
	out.append("\0\0\0", 3);
	out.append((char*) &_int_, 1);
}

