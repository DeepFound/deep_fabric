///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txclassc.h"
#include "txstring.h"

TX_DEFINE_STREAM_TYPE(txClassC,txClassB)

txClassC::txClassC (void)
{
}

txClassC::txClassC (const char* data, int value) :
	txClassB(data, value)
{
}

txClassC::~txClassC (void)
{
}

const txList* txClassC::list (void) const
{
	return &_list_;
}

void txClassC::storeInners (txOut& out) const
{
	txClassB::storeInners(out);

	out << _list_;
}

void txClassC::restoreInners (txIn& in)
{
	txClassB::restoreInners(in);

	in >> _list_;
}

