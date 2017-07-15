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
#include "txuchar8.h"

TX_DEFINE_PARENT_TYPE(txUChar8,txObject)

txUChar8::txUChar8 (void) :
	_char_(0)
{
}

txUChar8::txUChar8 (unsigned char v) :
	_char_(v)
{
}

txUChar8::txUChar8 (const txUChar8& obj)
{
	_char_ = obj._char_;
}

txUChar8& txUChar8::operator= (txUChar8& obj)
{
	if (this != &obj)
	{
		obj._char_ = obj._char_;
	}

	return *this;
}

txUChar8::~txUChar8 (void)
{
}

int txUChar8::equals (const txObject* obj) const
{
	return obj->isClass(txUChar8::Type) ?
		(_char_ == ((txUChar8*) obj)->_char_) : 0;
}

int txUChar8::compare (const txObject* obj) const
{
	if (_char_ == ((txUChar8*) obj)->_char_)
	{
		return 0;
	}
	else if (_char_ > ((txUChar8*) obj)->_char_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txUChar8::readObject (txIn& in)
{
	memcpy(&_char_, in.cursor() + 3, 1);
}

void txUChar8::writeObject (txOut& out) const
{
	out.writeRawHeader(htonl(TX_UNSIGNED_CHAR8));
	out.append("\0\0\0", 3);
	out.append((char*) &_char_, 1);
}

