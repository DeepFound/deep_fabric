///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txdouble.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txDouble,txObject)

txDouble::txDouble (void) :
	_double_(0)
{
}

txDouble::txDouble (double v) :
	_double_(v)
{
}

txDouble::txDouble (const txDouble& obj)
{
	_double_ = obj._double_;
}

txDouble& txDouble::operator= (txDouble& obj)
{
	if (this != &obj)
	{
		obj._double_ = obj._double_;
	}

	return *this;
}

txDouble::~txDouble (void)
{
}

int txDouble::equals (const txObject* obj) const
{
	return obj->isClass(txDouble::Type) ?
		(_double_ == ((txDouble*) obj)->_double_) : 0;
}

int txDouble::compare (const txObject* obj) const
{
	if (_double_ == ((txDouble*) obj)->_double_)
	{
		return 0;
	}
	else if (_double_ > ((txDouble*) obj)->_double_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txDouble::readObject (txIn& in)
{
	memcpy((char*) &_double_, in.cursor(), 8);
}

void txDouble::writeObject (txOut& out) const
{
	out << _double_;
}

