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

#include "txint64.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txInt64,txObject)

txInt64::txInt64 (void) :
	_hi_(0), _lo_(0)
{
}

txInt64::txInt64 (signed long hi, signed long lo) :
	_hi_(hi), _lo_(lo)
{
}

txInt64::txInt64 (const txInt64& obj)
{
	_hi_ = obj._hi_;
	_lo_ = obj._lo_;
}

txInt64& txInt64::operator= (txInt64& obj)
{
	if (this != &obj)
	{
		obj._hi_ = obj._hi_;
		obj._lo_ = obj._lo_;
	}

	return *this;
}

txInt64::~txInt64 (void)
{
}

int txInt64::equals (const txObject* obj) const
{
	return obj->isClass(txInt64::Type) ?
		(_hi_ == ((txInt64*) obj)->_hi_) &&
		(_lo_ == ((txInt64*) obj)->_lo_) : 0;
}

int txInt64::compare (const txObject* obj) const
{
	if (_hi_ == ((txInt64*) obj)->_hi_)
	{
		if (_lo_ == ((txInt64*) obj)->_lo_)
		{
			return 0;
		}
		else if (_lo_ == ((txInt64*) obj)->_lo_)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	else if (_hi_ > ((txInt64*) obj)->_hi_)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

void txInt64::readObject (txIn& in)
{
	_hi_ = ntohl(*(unsigned long*) (in.cursor()));
	_lo_ = ntohl(*(unsigned long*) (in.cursor() + 4));
}

void txInt64::writeObject (txOut& out) const
{
	out.writeRawHeader(htonl(TX_SIGNED_INT64));
	out.writeUInt(_hi_); out.writeUInt(_lo_);
}

