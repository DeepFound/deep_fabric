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

#include "txchar16.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txChar16,txObject)

txChar16::txChar16 (void) :
	_char_(0)
{
}

txChar16::txChar16 (signed short v) :
	_char_(v)
{
}

txChar16::txChar16 (const txChar16& obj)
{
	_char_ = obj._char_;
}

txChar16& txChar16::operator= (txChar16& obj)
{
	if (this != &obj)
	{
		_char_ = obj._char_;
	}

	return *this;
}

txChar16::~txChar16 (void)
{
}

int txChar16::equals (const txObject* obj) const
{
	return obj->isClass(txChar16::Type) ?
		(_char_ == ((txChar16*) obj)->_char_) : 0;
}

int txChar16::compare (const txObject* obj) const
{
	if (_char_ == ((txChar16*) obj)->_char_)
	{
		return 0;
	}
	else if (_char_ > ((txChar16*) obj)->_char_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txChar16::readObject (txIn& in)
{
	memcpy(&_char_, in.cursor() + 2, 2);
}

void txChar16::writeObject (txOut& out) const
{
	out.writeRawHeader(htonl(TX_SIGNED_CHAR16));
	out.append("\0\0", 2);
	out.append((char*) &_char_, 2);
}

