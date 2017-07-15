///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txstring.h"
#include "txstream.h"

TX_DEFINE_PARENT_TYPE(txStream,txObject)

txStream::txStream (void)
{
}
 
txStream::~txStream (void)
{
}

void txStream::readObject (txIn& in)
{
	restoreInners(in);
}

void txStream::writeObject (txOut& out) const
{
	unsigned int h_loc = out.length();

	out.putNull();

	out.writeUInt(classType().id());

	storeInners(out);

	out.writeHeader(h_loc, out.length()-(h_loc+4), TX_GTYPE_INT32);
}

void txStream::storeInners (txOut&) const
{
}

void txStream::restoreInners (txIn&)
{
}

