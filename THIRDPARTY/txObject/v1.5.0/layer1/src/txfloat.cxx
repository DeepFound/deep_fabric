///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txfloat.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txFloat,txObject)

txFloat::txFloat (void) :
	_float_(0)
{
}

txFloat::txFloat (float v) :
	_float_(v)
{
}

txFloat::txFloat (const txFloat& obj)
{
	_float_ = obj._float_;
}

txFloat& txFloat::operator= (txFloat& obj)
{
	if (this != &obj)
	{
		obj._float_ = obj._float_;
	}

	return *this;
}

txFloat::~txFloat (void)
{
}

int txFloat::equals (const txObject* obj) const
{
	return obj->isClass(txFloat::Type) ?
		(_float_ == ((txFloat*) obj)->_float_) : 0;
}

int txFloat::compare (const txObject* obj) const
{
	if (_float_ == ((txFloat*) obj)->_float_)
	{
		return 0;
	}
	else if (_float_ > ((txFloat*) obj)->_float_)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txFloat::readObject (txIn& in)
{
	memcpy((char*) &_float_, in.cursor(), 4);
}

void txFloat::writeObject (txOut& out) const
{
	out << _float_;
}

