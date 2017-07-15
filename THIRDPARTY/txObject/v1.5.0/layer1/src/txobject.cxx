///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "txobject.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txObject,txRoot)

txObject::txObject (void)
{
}

txObject::~txObject (void)
{
}

int txObject::compare (const txObject* obj) const
{
	fprintf(stderr, "TXOBJECT[error] : compare must be defined if used\n");
	fflush(stderr); TX_CRASH; return 0;
}

const txObject* txObjectSequence::insert (txObject*)
{
	return 0;
}

const txObject* txObjectSequence::find (const txObject*) const
{
	return 0;
}

txObject* txObjectSequence::remove (const txObject*)
{
	return 0;
}

void txObjectSequence::removeAndDestroy (const txObject*)
{
}

void txObjectSequence::clearAndDestroy (void)
{
}

void txObjectSequence::clear (void)
{
}

int txObjectSequence::entries (void) const
{
	return 0;
}

txObject* txObjectIterator::remove (void)
{
	return 0;
}

const txObject* txObjectIterator::next (void)
{
	return 0;
}

void txObjectIterator::reset (void)
{
}

