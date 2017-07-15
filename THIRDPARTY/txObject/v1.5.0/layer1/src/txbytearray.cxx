///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txstring.h"
#include "txbytearray.h"

TX_DEFINE_PARENT_TYPE(txByteArray,txObject)

txByteArray::txByteArray (void) :
	_length_(0),
	_hash_(0),
	_data_(0)
{
}

txByteArray::txByteArray (const char* data, unsigned long length) :
	_hash_(0)
{
	_resize_(data, length);
}

void txByteArray::_resize_ (const char* data, unsigned long length)
{
	#if !defined OSTORE_SUPPORT
		_data_ = new char[length];
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_data_ = new(TX_GET_OBJECT_CLUSTER(this),
				os_typespec::get_char(), length)
					char[length];
		}
		else
		{
			_data_ = new char[length];
		}
	#endif

	memcpy(_data_, data, length);

	_hash_ = 0;
	_length_ = length;
}

void txByteArray::append (const char* data, unsigned long length)
{
	char* olddata = _data_;

	#if !defined OSTORE_SUPPORT
		_data_ = new char[_length_ + length];
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_data_ = new(TX_GET_OBJECT_CLUSTER(this),
				os_typespec::get_char(), _length_ + length)
					char[_length_ + length];
		}
		else
		{
			_data_ = new char[_length_ + length];
		}
	#endif

	memcpy(_data_, olddata, _length_);
	memcpy(_data_ + _length_, data, length);

	_hash_ = 0;
	_length_ = _length_ + length;
}

void txByteArray::prepend (const char* data, unsigned long length)
{
	char* olddata = _data_;

	#if !defined OSTORE_SUPPORT
		_data_ = new char[_length_ + length];
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_data_ = new(TX_GET_OBJECT_CLUSTER(this),
				os_typespec::get_char(), _length_ + length)
					char[_length_ + length];
		}
		else
		{
			_data_ = new char[_length_ + length];
		}
	#endif

	memcpy(_data_, data, length);
	memcpy(_data_ + length, olddata, _length_);

	_hash_ = 0;
	_length_ = length + _length_;
}

unsigned txByteArray::hash (void) const
{
	if (_hash_)
	{
		return _hash_;
	}

	if (!length())
	{
		return 0;
	}

	((txByteArray*) this)->_hash_ = txHash(data(), length());

	return _hash_;
}

int txByteArray::equals (const txObject* rhs) const
{
	if (rhs->isClass(txByteArray::Type))
	{
		txByteArray* o = (txByteArray*) rhs;

		if (_length_ == o->_length_)
		{
			return memcmp(data(), o->data(), _length_) == 0;
		}
	}

	return 0;
}

int txByteArray::compare (const txObject* obj) const
{
	int v = memcmp(_data_, ((txByteArray*) obj)->_data_, _length_);

	if (v == 0)
	{
		if (_length_ == ((txByteArray*) obj)->_length_)
		{
			return 0;
		}
		else if (_length_ > ((txByteArray*) obj)->_length_)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
	else if (v > 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void txByteArray::readObject (txIn& in)
{
	delete [] _data_; _data_ = 0;

	if (in.objectLength())
	{
		_resize_(in.cursor(), in.objectLength());
	}
	else
	{
		_hash_ = 0; _length_ = 0;
	}
}

void txByteArray::writeObject (txOut& out) const
{
	if (_length_)
	{
		out.put(_data_, _length_, TX_BYTE_ARRAY);
	}
	else
	{
		out.writeHeader(out.length(), 0, TX_BYTE_ARRAY);
	}
}

