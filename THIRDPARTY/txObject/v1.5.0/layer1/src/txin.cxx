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

#include <stdio.h>
#include <stdlib.h>

#include "txin.h"
#include "txlist.h"
#include "txint8.h"
#include "txchar8.h"
#include "txtree.h"
#include "txint16.h"
#include "txint32.h"
#include "txint64.h"
#include "txuint8.h"
#include "txfloat.h"
#include "txdouble.h"
#include "txstring.h"
#include "txuint16.h"
#include "txuint32.h"
#include "txuint64.h"
#include "txchar16.h"
#include "txuchar8.h"
#include "txgobject.h"
#include "txfactory.h"
#include "txuchar16.h"
#include "txboolean.h"
#include "txhashmap.h"
#include "txhashset.h"
#include "txbytearray.h"
#include "sys/txobjref.h"
#include "sys/txrefobj.h"

txIn::txIn (const txOut& out) :
	_userflags_(0),
	_map_context_(0),
	_generic_type_(0),
	_currentLength_(0),
	_currentHeader_(0),
	_datastream_(new txSmartBuffer(out.data(), out.length()))
{
	_init_data_ = (char*) _datastream_->data();
	_init_length_ = _datastream_->length();

	_cursor_ = _init_data_;
	_end_ = _init_data_ + _init_length_;
}

txIn::txIn (char* data, unsigned long length) :
	_userflags_(0),
	_map_context_(0),
	_generic_type_(0),
	_currentLength_(0),
	_currentHeader_(0),
	_datastream_(0) // not making a copy
{
	_init_data_ = data;
	_init_length_ = length;

	_cursor_ = _init_data_;
	_end_ = _init_data_ + _init_length_;
}

txIn::txIn (const char* data, unsigned long length) :
	_userflags_(0),
	_map_context_(0),
	_generic_type_(0),
	_currentLength_(0),
	_currentHeader_(0),
	_datastream_(new txSmartBuffer(data, length))
{
	_init_data_ = (char*) _datastream_->data();
	_init_length_ = _datastream_->length();

	_cursor_ = _init_data_;
	_end_ = _init_data_ + _init_length_;
}

txIn::~txIn (void)
{
	txObject* key;

	if (_map_context_)
	{
		txHashMapIterator iter(*_map_context_);

		while (key = (txObject*) iter.next())
		{
			iter.remove();

			delete key; key = 0;
		}

		delete _map_context_; _map_context_ = 0;
	}

	delete _datastream_; _datastream_ = 0;
}

void txIn::_reCalAsciiLength_ (void)
{
	if (_currentLength_)
	{
		while (*(_cursor_ + _currentLength_ - 1) == '\0')
		{
			_currentLength_--;
		}
	}
}

unsigned long txIn::_get_ (unsigned long expected_type)
{
	unsigned long o = 0;

	if (_readHeaderThenTypeMatch_(expected_type))
	{
		o = ntohl(*(unsigned long*) (_cursor_));
	}

	_cursor_ += _currentLength_;

	return o;
}

int txIn::_typeMatchCurrentHeader_ (unsigned long expected_type) const
{
	return ((_currentHeader_ & TX_FULLTYPE) ==
		(expected_type & TX_FULLTYPE));
}

int txIn::_readHeader_ (void)
{
	if (_cursor_ == _end_)
	{
		_currentLength_ = 0;
		return 0;
	}

	if ((_cursor_ + 4) > _end_)
	{
		_currentLength_ = 0;

		fprintf(stderr, "TXOBJECT[error] : txin unexpected end of ");
		fprintf(stderr, "stream\n");
		fflush(stderr);
		#if defined TX_STREAM_DEBUG
		TX_CRASH;
		#endif

		return 0;
	}

	_currentHeader_ = ntohl(*(unsigned long*) _cursor_);
	_currentLength_ = _currentHeader_ & TX_LENGTH;
	_cursor_ += 4;

	if ((_cursor_ + _currentLength_) > _end_)
	{
		fprintf(stderr, "TXOBJECT[error] : txin unexpected end of ");
		fprintf(stderr, "stream\n");
		fflush(stderr);
		#if defined TX_STREAM_DEBUG
		TX_CRASH;
		#endif

		return 0;
	}

	return 1;
}

int txIn::_readHeaderThenTypeMatch_ (unsigned long type)
{
	return _readHeader_() && _typeMatchCurrentHeader_(type);
}

int txIn::_variableLengthOverlay_ (txObject& object)
{
	int matched = 0;
	unsigned long type = _currentHeader_ & TX_FULLTYPE;

	switch (type)
	{
		case TX_ASCII_STRING:
		{
			_reCalAsciiLength_();

			if (object.isClass(txString::Type)) matched = 1;
			break;
		}
		case TX_EMPTY_STRING:
		{
			_currentLength_ = 0;

			if (object.isClass(txString::Type)) matched = 1;
			break;
		}
		case TX_BYTE_ARRAYR0:
		{
			if (object.isClass(txByteArray::Type)) matched = 1;

			break;
		}
		case TX_BYTE_ARRAYR1:
		{
			_currentLength_ -= 3;

			if (object.isClass(txByteArray::Type)) matched = 1;
			break;
		}
		case TX_BYTE_ARRAYR2:
		{
			_currentLength_ -= 2;

			if (object.isClass(txByteArray::Type)) matched = 1;
			break;
		}
		case TX_BYTE_ARRAYR3:
		{
			_currentLength_ -= 1;

			if (object.isClass(txByteArray::Type)) matched = 1;
			break;
		}
		case TX_LIST:
		{
			if (object.isClass(txList::Type)) matched = 1;
			break;
		}
		case TX_SET:
		{
			if (object.isClass(txHashSet::Type)) matched = 1;
			break;
		}
		case TX_MAP:
		{
			if (object.isClass(txHashMap::Type)) matched = 1;
			break;
		}
		case TX_TREE:
		{
			if (object.isClass(txTree::Type)) matched = 1;
			break;
		}
		case TX_GTYPE_INT32:
		{
			txUInt32 lu; lu.readObject(*this); _cursor_ += 4;

			const txString* ct = txFactory::findTypeInFactory(lu);

			if (ct && object.isClass(*ct)) matched = 1;
			break;
		}
		case TX_GTYPE_INT64:
		case TX_GTYPE_STRING:
		case TX_GTYPE_OBJECT:
		{
			fprintf(stderr, "TXOBJECT[error] : txin non-supported");
			fprintf(stderr, " gtype\n");
			fflush(stderr);
			break;
		}
		case TX_REFOBJ_INT32:
		{
			matched = 1;

			if (!_map_context_)
			{
				_map_context_ = new txHashMap();
			}

			txUInt32* id = new txUInt32();

			id->readObject(*this);

			_cursor_ += 8;
			_currentLength_ -= 8;

			_map_context_->insertKeyAndValue(id, &object);

			break;
		}
		case TX_REFOBJ_INT64:
		case TX_REFOBJ_STRING:
		case TX_REFOBJ_OBJECT:
		{
			if (object.isClass(txRefObj::Type)) matched = 1;
			break;
		}
		case TX_OBJREF_INT32:
		case TX_OBJREF_INT64:
		case TX_OBJREF_STRING:
		case TX_OBJREF_OBJECT:
		{
			if (object.isClass(txObjRef::Type)) matched = 1;
			break;
		}
		default:
		{
			fprintf(stderr, "TXOBJECT[error] : txin unknown");
			fprintf(stderr, " type\n");
			fflush(stderr);
			#if defined TX_STREAM_DEBUG
			TX_CRASH;
			#endif
		}
	}

	return matched;
}

txObject* txIn::_variableLengthDestream_ (int& call)
{
	call = 1;
	txObject* o = 0;
	unsigned long type = _currentHeader_ & TX_FULLTYPE;

	switch (type)
	{
		case TX_ASCII_STRING:
		{
			_reCalAsciiLength_();

			o = (txObject*) new txString();
			break;
		}
		case TX_EMPTY_STRING:
		{
			_currentLength_ = 0;

			o = (txObject*) new txString();
			break;
		}
		case TX_BYTE_ARRAYR0:
		{
			o = (txObject*) new txByteArray();
			break;
		}
		case TX_BYTE_ARRAYR1:
		{
			_currentLength_ -= _currentLength_ - 1;
			o = (txObject*) new txByteArray();
			break;
		}
		case TX_BYTE_ARRAYR2:
		{
			_currentLength_ = _currentLength_ - 2;
			o = (txObject*) new txByteArray();
			break;
		}
		case TX_BYTE_ARRAYR3:
		{
			_currentLength_ = _currentLength_ - 3;
			o = (txObject*) new txByteArray();
			break;
		}
		case TX_LIST:
		{
			o = (txObject*) new txList();
			break;
		}
		case TX_SET:
		{
			o = (txObject*) new txHashSet();
			break;
		}
		case TX_MAP:
		{
			o = (txObject*) new txHashMap();
			break;
		}
		case TX_TREE:
		{
			o = (txObject*) new txTree();
			break;
		}
		case TX_GTYPE_INT32:
		{
			unsigned long length = _currentLength_ - 4;

			txUInt32 lu; lu.readObject(*this); _cursor_ += 4;

			if (!(o = (txObject*) txFactory::create(lu, *this, 1)))
			{
				if (_generic_type_)
				{
					o = new txGObject(lu, length);
				}
			}
			else
			{
				call = 0;
			}

			break;
		}
		case TX_GTYPE_INT64:
		case TX_GTYPE_STRING:
		case TX_GTYPE_OBJECT:
		{
			fprintf(stderr, "TXOBJECT[error] : txin non-supported");
			fprintf(stderr, " gtype\n");
			fflush(stderr);
			break;
		}
		case TX_REFOBJ_INT32:
		{
			call = 0;

			if (!_map_context_)
			{
				_map_context_ = new txHashMap();
			}

			txUInt32* id = new txUInt32();

			id->readObject(*this);

			_cursor_ += 4;

			*this >> o;
			
			_map_context_->insertKeyAndValue(id, o);

			break;
		}
		case TX_REFOBJ_INT64:
		case TX_REFOBJ_STRING:
		case TX_REFOBJ_OBJECT:
		{
			fprintf(stderr, "TXOBJECT[error] : txin non-supported");
			fprintf(stderr, " refobj\n");
			fflush(stderr);
			break;
		}
		case TX_OBJREF_INT32:
		{
			call = 0;

			txUInt32 id;

			id.readObject(*this);

			_cursor_ += 4;

			o = (txObject*) _map_context_->findValue(&id);

			break;
		}
		case TX_OBJREF_INT64:
		case TX_OBJREF_STRING:
		case TX_OBJREF_OBJECT:
		{
			fprintf(stderr, "TXOBJECT[error] : txin non-supported");
			fprintf(stderr, " objref\n");
			fflush(stderr);
			break;
		}
		default:
		{
			fprintf(stderr, "TXOBJECT[error] : txin unknown");
			fprintf(stderr, " type\n");
			fflush(stderr);
			#if defined TX_STREAM_DEBUG
			TX_CRASH;
			#endif
			o = 0;
		}
	}

	return o;
}

int txIn::getNull (void)
{
	_readHeaderThenTypeMatch_(TX_NULL);

	_cursor_ += _currentLength_;

	return 0;
}

txIn& txIn::operator>> (signed char& o)
{
	o = (signed char) _get_(TX_SIGNED_CHAR8);

	return *this;
}

txIn& txIn::operator>> (unsigned char& o)
{
	o = (unsigned char) _get_(TX_UNSIGNED_CHAR8);

	return *this;
}

txIn& txIn::operator>> (signed short int& o)
{
	o = (signed short int) _get_(TX_SIGNED_INT16);

	return *this; 
}

txIn& txIn::operator>> (signed long int& o) 
{
	o = (signed long int) _get_(TX_SIGNED_INT32); 

	return *this;
}

txIn& txIn::operator>> (unsigned short int& o)
{
	o = (unsigned short int) _get_(TX_UNSIGNED_INT16);

	return *this;
}

txIn& txIn::operator>> (unsigned long int& o)
{
	o = _get_(TX_UNSIGNED_INT32);

	return *this;
}

txIn& txIn::operator>> (float& o)
{
	o = 0;

	if (_readHeaderThenTypeMatch_(TX_FLOAT))
	{
		memcpy((char*) &o, _cursor_, 4);
	}

	_cursor_ = _cursor_ + _currentLength_;

	return *this;
}

txIn& txIn::operator>> (double& o)
{
	o = 0;

	if (_readHeaderThenTypeMatch_(TX_DOUBLE))
	{
		memcpy((char*) &o, _cursor_, 8);
	}

	_cursor_ = _cursor_ + _currentLength_;

	return *this;
}

txIn& txIn::operator>> (char*& o)
{
	get(o, TX_ASCII_STRING);

	return *this;
}

txObject* txIn::destream (void)
{
	int call = 1;
	txObject* o = 0;

	if (!_readHeader_())
	{
		_cursor_ = _cursor_ + _currentLength_;
	
		return o;
	}

	const char* expected_end = _cursor_ + _currentLength_;

	switch(_currentHeader_)
	{
		case TX_NULL:
		{
			o = 0;
			break;
		}
		case TX_BOOLEAN_FALSE:
		{
			o = (txObject*) new txBoolean(0);
			break;
		}
		case TX_BOOLEAN_TRUE:
		{
			o = (txObject*) new txBoolean(1);
			break;
		}
		case TX_SIGNED_CHAR8:
		{
			o = (txObject*) new txChar8();
			break;
		}
		case TX_UNSIGNED_CHAR8:
		{
			o = (txObject*) new txUChar8();
			break;
		}
		case TX_SIGNED_CHAR16:
		{
			o = (txObject*) new txChar16();
			break;
		}
		case TX_UNSIGNED_CHAR16:
		{
			o = (txObject*) new txUChar16();
			break;
		}
		case TX_SIGNED_INT8:
		{
			o = (txObject*) new txInt8();
			break;
		}
		case TX_UNSIGNED_INT8:
		{
			o = (txObject*) new txUInt8();
			break;
		}
		case TX_SIGNED_INT16:
		{
			o = (txObject*) new txInt16();
			break;
		}
		case TX_UNSIGNED_INT16:
		{
			o = (txObject*) new txUInt16();
			break;
		}
		case TX_SIGNED_INT32:
		{
			o = (txObject*) new txInt32();
			break;
		}
		case TX_UNSIGNED_INT32:
		{
			o = (txObject*) new txUInt32();
			break;
		}
		case TX_SIGNED_INT64:
		{
			o = (txObject*) new txInt64();
			break;
		}
		case TX_UNSIGNED_INT64:
		{
			o = (txObject*) new txUInt64();
			break;
		}
		case TX_FLOAT:
		{
			o = (txObject*) new txFloat();
			break;
		}
		case TX_DOUBLE:
		{
			o = (txObject*) new txDouble();
			break;
		}
		default:
		{
			o = _variableLengthDestream_(call);
		}
	}

	if (o && call)
	{
		o->readObject(*this);
	}

	_cursor_ = expected_end;

	return o;
}

void txIn::destream (txObject& object, int overlay_different_type)
{
	int matched = 0;

	if (!_readHeader_())
	{
		_cursor_ += _currentLength_;
		return;
	}

	const char* expected_end = _cursor_ + _currentLength_;

	switch(_currentHeader_)
	{
		case TX_NULL:
		{
			matched = 1;
			break;
		}
		case TX_BOOLEAN_FALSE:
		{
			if (object.isClass(txBoolean::Type)) matched = 1;
			break;
		}
		case TX_BOOLEAN_TRUE:
		{
			if (object.isClass(txBoolean::Type)) matched = 1;
			break;
		}
		case TX_SIGNED_CHAR8:
		{
			if (object.isClass(txChar8::Type)) matched = 1;
			break;
		}
		case TX_UNSIGNED_CHAR8:
		{
			if (object.isClass(txUChar8::Type)) matched = 1;
			break;
		}
		case TX_SIGNED_CHAR16:
		{
			if (object.isClass(txChar16::Type)) matched = 1;
			break;
		}
		case TX_UNSIGNED_CHAR16:
		{
			if (object.isClass(txUChar16::Type)) matched = 1;
			break;
		}
		case TX_SIGNED_INT8:
		{
			if (object.isClass(txInt8::Type)) matched = 1;
			break;
		}
		case TX_UNSIGNED_INT8:
		{
			if (object.isClass(txUInt8::Type)) matched = 1;
			break;
		}
		case TX_SIGNED_INT16:
		{
			if (object.isClass(txInt16::Type)) matched = 1;
			break;
		}
		case TX_UNSIGNED_INT16:
		{
			if (object.isClass(txUInt16::Type)) matched = 1;
			break;
		}
		case TX_SIGNED_INT32:
		{
			if (object.isClass(txInt32::Type)) matched = 1;
			break;
		}
		case TX_UNSIGNED_INT32:
		{
			if (object.isClass(txUInt32::Type)) matched = 1;
			break;
		}
		case TX_SIGNED_INT64:
		{
			if (object.isClass(txInt64::Type)) matched = 1;
			break;
		}
		case TX_UNSIGNED_INT64:
		{
			if (object.isClass(txUInt64::Type)) matched = 1;
			break;
		}
		case TX_FLOAT:
		{
			if (object.isClass(txFloat::Type)) matched = 1;
			break;
		}
		case TX_DOUBLE:
		{
			if (object.isClass(txDouble::Type)) matched = 1;
			break;
		}
		default:
		{
			matched = _variableLengthOverlay_(object);
		}
	}

	if (matched || overlay_different_type)
	{
		object.readObject(*this);
	}
	else
	{
		fprintf(stderr, "TXOBJECT[error] : txin overlay type ");
		fprintf(stderr, "mismatch\n");
		fflush(stderr);
		#if defined TX_STREAM_DEBUG
		TX_CRASH;
		#endif
	}

	_cursor_ = expected_end;
}

unsigned long txIn::get (char*& o, unsigned long expected_type)
{
	o = 0;
	const char* expected_end;

	if (!_readHeader_() || (TX_NULL == _currentHeader_))
	{
		return 0;
	}

	unsigned long type = _currentHeader_ & TX_FULLTYPE;

	expected_end = _cursor_ + _currentLength_;

	if (expected_type == TX_BYTE_ARRAY)
	{
		switch(type)
		{
			case TX_BYTE_ARRAYR0:
			{
				break;
			}
			case TX_BYTE_ARRAYR1:
			{
				_currentLength_ -= 3;
				break;
			}
			case TX_BYTE_ARRAYR2:
			{
				_currentLength_ -= 2;
				break;
			}
			case TX_BYTE_ARRAYR3:
			{
				_currentLength_ -= 1;
				break;
			}
			default:
			{
				fprintf(stderr, "TXOBJECT[error] : txin wrong");
				fprintf(stderr, " type\n");
				fflush(stderr);
				#if defined TX_STREAM_DEBUG
				TX_CRASH;
				#endif
			}
		}

		if (_currentLength_)
		{
			o = new char[_currentLength_];
			memcpy(o, _cursor_, _currentLength_);
		}
	}
	else if (_typeMatchCurrentHeader_(TX_ASCII_STRING) && _currentLength_)
	{
		_reCalAsciiLength_();

		o = new char[_currentLength_ + 1];

		memcpy(o, _cursor_, _currentLength_);

		o[_currentLength_] = '\0';
	}
	else if (_typeMatchCurrentHeader_(TX_EMPTY_STRING))
	{
		o = new char[1];

		o[0] = '\0';
	}
	else if (_currentLength_)
	{
		fprintf(stderr, "TXOBJECT[error] : txin wrong type\n");
		fflush(stderr);
		#if defined TX_STREAM_DEBUG
		TX_CRASH;
		#endif
	}

	_cursor_ = expected_end;

	return _currentLength_;
}

unsigned long txIn::get (char* o, unsigned long len, unsigned long expected_type)
{
	const char* expected_end;
	unsigned long nbytes_copied = 0;

	if (!_readHeader_() || (TX_NULL == _currentHeader_))
	{
		return 0;
	}

	unsigned long type = _currentHeader_ & TX_FULLTYPE;

	expected_end = _cursor_ + _currentLength_;

	if (expected_type == TX_BYTE_ARRAY)
	{
		switch(type)
		{
			case TX_BYTE_ARRAYR0:
			{
				break;
			}
			case TX_BYTE_ARRAYR1:
			{
				_currentLength_ -= 3;
				break;
			}
			case TX_BYTE_ARRAYR2:
			{
				_currentLength_ -= 2;
				break;
			}
			case TX_BYTE_ARRAYR3:
			{
				_currentLength_ -= 1;
				break;
			}
			default:
			{
				fprintf(stderr, "TXOBJECT[error] : txin wrong");
				fprintf(stderr, " type\n");
				fflush(stderr);
				#if defined TX_STREAM_DEBUG
				TX_CRASH;
				#endif
			}
		}

		nbytes_copied = len <= _currentLength_ ? len : _currentLength_;

		if (nbytes_copied)
		{
			memcpy(o, _cursor_, nbytes_copied);
		}
	}
	else if (_typeMatchCurrentHeader_(TX_ASCII_STRING) && _currentLength_)
	{
		_reCalAsciiLength_();

		nbytes_copied = len <= _currentLength_ ? len : _currentLength_;

		if (nbytes_copied)
		{
			memcpy(o, _cursor_, nbytes_copied);

			o[nbytes_copied] = '\0';
		}
	}
	else if (_typeMatchCurrentHeader_(TX_EMPTY_STRING) && len)
	{
		nbytes_copied = 1;

		o[0] = '\0';
	}
	else if (_currentLength_)
	{
		fprintf(stderr, "TXOBJECT[error] : txin wrong type\n");
		fflush(stderr);
		#if defined TX_STREAM_DEBUG
		TX_CRASH;
		#endif
	}

	if (nbytes_copied && nbytes_copied < len)
	{
		memset(&o[nbytes_copied], 0, len - nbytes_copied);
	}

	_cursor_ = expected_end;

	return nbytes_copied;
}

const char* txIn::cursor (void) const
{
	return _cursor_;
}

void txIn::cursor (unsigned long index)
{
	_currentLength_ = 0;

	_cursor_ = _init_data_ + index;
}

unsigned long txIn::index (void) const
{
	return (unsigned long) (_cursor_ - (unsigned long) _init_data_);
}

unsigned long txIn::objectLength (void) const
{
	return _currentLength_;
}

void txIn::flush (void)
{
	if (_datastream_)
	{
		_datastream_->clear();
	}

	_init_length_ = 0;
	_init_data_ = 0;
	_cursor_ = 0;
}

const char* txIn::data (void) const
{
	return _init_data_;	
}

unsigned long txIn::length (void) const
{
	return _init_length_;	
}

void txIn::genericTypingOn (void)
{
	_generic_type_ = 1;
}

void txIn::genericTypingOff (void)
{
	_generic_type_ = 0;
}

int txIn::genericTyping (void) const
{
	return _generic_type_;
}

void txIn::userFlags (unsigned long flags)
{
	_userflags_ = flags;
}

unsigned long txIn::userFlags (void) const
{
	return _userflags_;
}

