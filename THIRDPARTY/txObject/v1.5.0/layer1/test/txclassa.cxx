///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txclassa.h"
#include "txstring.h"

TX_DEFINE_STREAM_TYPE(txClassA,txStream)

txClassA::txClassA (void)
{
}

txClassA::txClassA (const char* data, int value) :
	_integer_(value), _string_(data)
{
}

txClassA::~txClassA (void)
{
}

const char* txClassA::data (void) const
{
	return _string_.data();
}

int txClassA::value (void) const
{
	return _integer_.value();
}

void txClassA::storeInners (txOut& out) const
{
	out << _string_;
	out << _integer_;
}

void txClassA::restoreInners (txIn& in)
{
	in >> _string_;
	in >> _integer_;
}

