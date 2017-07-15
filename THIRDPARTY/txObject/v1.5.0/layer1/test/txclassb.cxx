///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txclassb.h"
#include "txstring.h"

TX_DEFINE_STREAM_TYPE(txClassB,txStream)

txClassB::txClassB (void)
{
}

txClassB::txClassB (const char* data, int value) :
	_generic_(data, value)
{
}

txClassB::~txClassB (void)
{
}

const char* txClassB::data (void) const
{
	return _generic_.data();
}

int txClassB::value (void) const
{
	return _generic_.value();
}

void txClassB::storeInners (txOut& out) const
{
	out << _generic_;
}

void txClassB::restoreInners (txIn& in)
{
	in >> _generic_;
}

