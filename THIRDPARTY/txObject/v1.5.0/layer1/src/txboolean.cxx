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
#include "txboolean.h"

TX_DEFINE_PARENT_TYPE(txBoolean,txObject)

txBoolean::txBoolean (void) :
	_bool_(0)
{
}

txBoolean::txBoolean (txObjectType::Boolean v) :
	_bool_(v)
{
}

txBoolean::txBoolean (const txBoolean& obj)
{
	_bool_ = obj._bool_;
}

txBoolean& txBoolean::operator= (txBoolean& obj)
{
	if (this != &obj)
	{
		obj._bool_ = obj._bool_;
	}

	return *this;
}

txBoolean::~txBoolean (void)
{
}

int txBoolean::equals (const txObject* obj) const
{
	return obj->isClass(txBoolean::Type) ?
		(_bool_ == ((txBoolean*) obj)->_bool_) : 0;
}

int txBoolean::compare (const txObject* obj) const
{
	if (_bool_ == ((txBoolean*) obj)->_bool_)
	{
		return 0;
	}
	else if (_bool_ > ((txBoolean*) obj)->_bool_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txBoolean::readObject (txIn& in)
{
	// done in class txIn
}

void txBoolean::writeObject (txOut& out) const
{
	if (_bool_)
	{
		out.writeRawHeader(htonl(TX_BOOLEAN_TRUE));
	}
	else
	{
		out.writeRawHeader(htonl(TX_BOOLEAN_FALSE));
	}
}

