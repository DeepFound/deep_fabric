///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txstring.h"
#include "sys/txobjref.h"

TX_DEFINE_PARENT_TYPE(txObjRef,txObject)

txObjRef::txObjRef (unsigned long int ref) :
	_ref_(ref)
{
}

txObjRef::~txObjRef (void)
{
}

int txObjRef::equals (const txObject* obj) const
{
	return obj->isClass(txObjRef::Type) ?
		(_ref_ == ((txObjRef*) obj)->_ref_) : 0;
}

void txObjRef::readObject (txIn&)
{
}

void txObjRef::writeObject (txOut& out) const
{
	unsigned int h_loc = out.length(); 

	out.putNull(); 

	out.writeUInt(_ref_); 

	out.writeHeader(h_loc, out.length()-(h_loc+4), TX_OBJREF_INT32);
}

