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

#include "txout.h"
#include "txint8.h"
#include "txint16.h"
#include "txint32.h"
#include "txint64.h"
#include "txuint8.h"
#include "txchar8.h"
#include "txfloat.h"
#include "txobject.h" 
#include "txdouble.h"
#include "txuint16.h"
#include "txuint32.h"
#include "txuint64.h"
#include "txchar16.h"
#include "txuchar8.h"
#include "txuchar16.h"
#include "txboolean.h"
#include "txhashmap.h" 
#include "sys/txobjref.h" 
#include "sys/txrefobj.h" 

txOut::txOut (void) :
	_ref_objs_(0), _userflags_(0), _datastream_(512), _map_context_(0)
{
}

txOut::~txOut (void) 
{
	if (_map_context_)
	{
		txHashMapIterator iter(*_map_context_);

		while (iter.next())
		{
			delete iter.value();
		}

		delete _map_context_; _map_context_ = 0; 
	}
}

unsigned long txOut::_put_ (txObject* obj)
{
	txObjRef* oid;

	if (_ref_objs_ && obj)
	{
		_validateRefContainers_();

		if (oid = (txObjRef*) _map_context_->findValue(obj))
		{
			oid->writeObject(*this);
		}
		else
		{
			oid = new txObjRef((unsigned long) obj);

			_map_context_->insertKeyAndValue(obj, oid);

			txRefObj robj(oid->ref(), obj);

			robj.writeObject(*this);
		}
	}
	else if (obj)
	{
		obj->writeObject(*this);
	}
	else
	{
		putNull();
	}

	return length();
}

void txOut::_prependUInt_ (unsigned long o)
{
	long length = htonl(o);

	_datastream_.prepend((char*) &length, 4);
}

void txOut::_writeStr_ (const char* str, unsigned long slen)
{
	unsigned long padlen = slen & 0x3;

	_datastream_.append(str, slen);

	if (padlen)
	{
		_datastream_.append("\0\0\0\0", 4-padlen);
	}
}

void txOut::_writeHeader_ (unsigned long o)
{
	writeUInt(o);
}

void txOut::_writeHeader_ (unsigned long ot, unsigned long ol)
{
	writeUInt(_calHeader_(ot, ol));
}

void txOut::_writeUChar_ (unsigned char c)
{
	_datastream_.append("\0\0\0", 3);
	_datastream_.append((char*) &c, 1);
}

void txOut::_writeChar_ (signed char c)
{
	_datastream_.append("\0\0\0", 3);
	_datastream_.append((char*) &c, 1);
}

void txOut::_validateRefContainers_ (txOut* out)
{
	txObject* obj;

	if (!_map_context_)
	{
		_map_context_ = new txHashMap();
	}

	if (out && out->_map_context_)
	{
		txHashMapIterator iter(*out->_map_context_);

		while (obj = (txObject*) iter.next())
		{
			_map_context_->insertKeyAndValue(
				obj, (txObject*) iter.value());
		}
	}
}

unsigned long txOut::_calHeader_ (unsigned long ot, unsigned long ol)
{
	return ot|ol; 
}

void txOut::writeUInt (unsigned long int o)
{
	unsigned long value = htonl(o);

	_datastream_.append((char*) &value, 4);
}

void txOut::writeRawHeader (unsigned long header)
{
	_datastream_.append((char*) &header, 4);
}

void txOut::writeHeader (unsigned long at, unsigned long ol, unsigned long ot)
{
	unsigned int header = htonl(_calHeader_(ot, ol));

	_datastream_.place((char*) &header, 4, at); 
}
 
void txOut::putNull (void)
{
	_datastream_.append("\0\0\0\0", 4); 
}

txOut& txOut::operator<< (signed char o)
{
	writeRawHeader(htonl(TX_SIGNED_CHAR8));

	_writeChar_(o);

	return *this;
}

txOut& txOut::operator<< (unsigned char o)
{
	writeRawHeader(htonl(TX_UNSIGNED_CHAR8));

	_writeUChar_(o);

	return *this;
}

txOut& txOut::operator<< (signed short int o)
{
	writeRawHeader(htonl(TX_SIGNED_INT16));

	writeUInt((unsigned int) o);

	return *this;
}

txOut& txOut::operator<< (signed long int o)
{
	writeRawHeader(htonl(TX_SIGNED_INT32));

	writeUInt((unsigned int) o);

	return *this;
}

txOut& txOut::operator<< (unsigned short int o)
{
	writeRawHeader(htonl(TX_UNSIGNED_INT16));

	writeUInt(o);

	return *this;
}

txOut& txOut::operator<< (unsigned long int o)
{
	writeRawHeader(htonl(TX_UNSIGNED_INT32));

	writeUInt(o);

	return *this;
}

txOut& txOut::operator<< (float o)
{
	_writeHeader_(TX_FLOAT);

	_datastream_.append((char*) &o, 4);

	return *this;
}

txOut& txOut::operator<< (double o)
{
	_writeHeader_(TX_DOUBLE);

	_datastream_.append((char*) &o, 8);

	return *this;
}

txOut& txOut::operator<< (const char* o)
{
	if (o)
	{
		unsigned long l = strlen(o);

		if (!l)
		{
			_writeHeader_(TX_EMPTY_STRING);
		}
		else
		{
			put(o, l, TX_ASCII_STRING);
		}
	}
	else
	{
		putNull();
	}

	return *this;
}
 
txOut& txOut::operator<< (const txObject& o)
{
	_put_(&(txObject&) o);

	return *this;
}
 
txOut& txOut::operator<< (const txObject* o)
{
	_put_((txObject*) o);

	return *this;
}
 
void txOut::put (const txObject* o)
{
	_put_((txObject*) o);
}

void txOut::put (const txObject& o)
{
	_put_((txObject*) &o);
}

void txOut::put (const char* o, unsigned long l, unsigned long int type)
{
	if (o)
	{
		if (type != TX_BYTE_ARRAY) // ASCII, UTF8, UTF16
		{
			if (!l)
			{
				_writeHeader_(TX_EMPTY_STRING);
			}
			else
			{
				_writeHeader_(type, (l+3) & TX_LENGTH);

				_writeStr_(o, l);
			}
		}
		else
		{
			_writeHeader_(TX_BYTE_ARRAY|(l&3),(l+3)&TX_LENGTH);

			_writeStr_(o, l);
		}
	}
	else
	{
		putNull();
	}
}

void txOut::append (const txOut& out)
{
	_datastream_.append(out.data(), out.length());

	_validateRefContainers_(&((txOut&) out));
}

void txOut::append (const char* data, int length)
{
	_datastream_.append(data, length);
}

unsigned int txOut::length (void) const
{
	return _datastream_.length();
}

const char* txOut::data (void) const
{
	return _datastream_.data();
}

void txOut::flush (void)
{
	_datastream_.clear();

	if (_map_context_)
	{
		_map_context_->clear();
	}
}

int txOut::objectReferencing (void) const
{
	return _ref_objs_;
}

void txOut::objectReferencingOff (void)
{
	_ref_objs_ = 0;
}

void txOut::objectReferencingOn (void)
{
	_ref_objs_ = 1;
}

unsigned long txOut::userFlags (void) const
{
	return _userflags_;
}

void txOut::userFlags (unsigned long flags)
{
	_userflags_ = flags;
}

