///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txstring.h"
#include "sys/txrefobj.h"

TX_DEFINE_PARENT_TYPE(txRefObj,txObject)

txRefObj::txRefObj (unsigned long int ref, const txObject* obj) :
	_ref_(ref), _object_(obj)
{
}

txRefObj::~txRefObj (void)
{
}

void txRefObj::readObject (txIn&)
{
}

void txRefObj::writeObject (txOut& out) const
{
	unsigned int h_loc = out.length(); 

	out.putNull(); 

	out.writeUInt(_ref_);

	((txObject*) _object_)->writeObject(out); 

	out.writeHeader(h_loc, out.length()-(h_loc+4), TX_REFOBJ_INT32);
}

