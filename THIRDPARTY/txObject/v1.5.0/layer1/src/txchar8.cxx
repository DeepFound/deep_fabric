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

#include "txchar8.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txChar8,txObject)

txChar8::txChar8 (void) :
	_char_(0)
{
}

txChar8::txChar8 (signed char v) :
	_char_(v)
{
}

txChar8::txChar8 (const txChar8& obj)
{
	_char_ = obj._char_;
}

txChar8& txChar8::operator= (txChar8& obj)
{
	if (this != &obj)
	{
		obj._char_ = obj._char_;
	}

	return *this;
}

txChar8::~txChar8 (void)
{
}

int txChar8::equals (const txObject* obj) const
{
	return obj->isClass(txChar8::Type) ?
		(_char_ == ((txChar8*) obj)->_char_) : 0;
}

int txChar8::compare (const txObject* obj) const
{
	if (_char_ == ((txChar8*) obj)->_char_)
	{
		return 0;
	}
	else if (_char_ > ((txChar8*) obj)->_char_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txChar8::readObject (txIn& in)
{
	memcpy(&_char_, in.cursor() + 3, 1);
}

void txChar8::writeObject (txOut& out) const
{
	out.writeRawHeader(htonl(TX_SIGNED_CHAR8));
	out.append("\0\0\0", 3);
	out.append((char*) &_char_, 1);
}

