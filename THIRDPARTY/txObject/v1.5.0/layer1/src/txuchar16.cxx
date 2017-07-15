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
#include "txuchar16.h"

TX_DEFINE_PARENT_TYPE(txUChar16,txObject)

txUChar16::txUChar16 (void) :
	_char_(0)
{
}

txUChar16::txUChar16 (unsigned short v) :
	_char_(v)
{
}

txUChar16::txUChar16 (const txUChar16& obj)
{
	_char_ = obj._char_;
}

txUChar16& txUChar16::operator= (txUChar16& obj)
{
	if (this != &obj)
	{
		obj._char_ = obj._char_;
	}

	return *this;
}

txUChar16::~txUChar16 (void)
{
}

int txUChar16::equals (const txObject* obj) const
{
	return obj->isClass(txUChar16::Type) ?
		(_char_ == ((txUChar16*) obj)->_char_) : 0;
}

int txUChar16::compare (const txObject* obj) const
{
	if (_char_ == ((txUChar16*) obj)->_char_)
	{
		return 0;
	}
	else if (_char_ > ((txUChar16*) obj)->_char_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txUChar16::readObject (txIn& in)
{
	memcpy(&_char_, in.cursor() + 2, 2);
}

void txUChar16::writeObject (txOut& out) const
{
	out.writeRawHeader(htonl(TX_UNSIGNED_CHAR16));
	out.append("\0\0", 2);
	out.append((char*) &_char_, 2);
}

