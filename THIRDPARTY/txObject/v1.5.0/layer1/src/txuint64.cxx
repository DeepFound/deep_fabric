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
#include "txuint64.h"

TX_DEFINE_PARENT_TYPE(txUInt64,txObject)

txUInt64::txUInt64 (void) :
	_hi_(0), _lo_(0)
{
}

txUInt64::txUInt64 (unsigned long hi, unsigned long lo) :
	_hi_(hi), _lo_(lo)
{
}

txUInt64::txUInt64 (const txUInt64& obj)
{
	_hi_ = obj._hi_;
	_lo_ = obj._lo_;
}

txUInt64& txUInt64::operator= (txUInt64& obj)
{
	if (this != &obj)
	{
		obj._hi_ = obj._hi_;
		obj._lo_ = obj._lo_;
	}

	return *this;
}

txUInt64::~txUInt64 (void)
{
}

int txUInt64::equals (const txObject* obj) const
{
	return obj->isClass(txUInt64::Type) ?
		(_hi_ == ((txUInt64*) obj)->_hi_) &&
		(_lo_ == ((txUInt64*) obj)->_lo_) : 0;
}

int txUInt64::compare (const txObject* obj) const
{
	if (_hi_ == ((txUInt64*) obj)->_hi_)
	{
		if (_lo_ == ((txUInt64*) obj)->_lo_)
		{
			return 0;
		}
		else if (_lo_ == ((txUInt64*) obj)->_lo_)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	else if (_hi_ > ((txUInt64*) obj)->_hi_)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

void txUInt64::readObject (txIn& in)
{
	_hi_ = ntohl(*(unsigned long*) (in.cursor()));
	_lo_ = ntohl(*(unsigned long*) (in.cursor() + 4));
}

void txUInt64::writeObject (txOut& out) const
{
	out.writeRawHeader(htonl(TX_UNSIGNED_INT64));
	out.writeUInt(_hi_); out.writeUInt(_lo_);
}

