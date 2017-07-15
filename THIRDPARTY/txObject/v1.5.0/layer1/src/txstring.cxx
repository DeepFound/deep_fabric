///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txString,txObject)

txString::txString (void) :
	_length_(0),
	_hash_(0),
	_data_(0)
{
}

txString::txString (const char* data) :
	_hash_(0)
{
	_resize_(data, strlen(data));
}

txString::txString (const char* data, unsigned long length) :
	_hash_(0)
{
	_resize_(data, length);
}

void txString::_resize_ (const char* data, unsigned long length)
{
	#if !defined OSTORE_SUPPORT
		_data_ = new char[length + 1];
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_data_ = new(TX_GET_OBJECT_CLUSTER(this),
				os_typespec::get_char(), length + 1)
					char[length + 1];
		}
		else
		{
			_data_ = new char[length + 1];
		}
	#endif

	memcpy(_data_, data, length);
	_data_[length] = '\0';

	_hash_ = 0;
	_length_ = length;
}

void txString::append (const char* data, unsigned long length)
{
	char* olddata = _data_;

	#if !defined OSTORE_SUPPORT
		_data_ = new char[_length_ + length + 1];
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_data_ = new(TX_GET_OBJECT_CLUSTER(this),
				os_typespec::get_char(), _length_ + length + 1)
					char[_length_ + length + 1];
		}
		else
		{
			_data_ = new char[_length_ + length + 1];
		}
	#endif

	memcpy(_data_, olddata, _length_);
	memcpy(_data_ + _length_, data, length);
	_data_[_length_ + length] = '\0';

	_hash_ = 0;
	_length_ = _length_ + length;
}

void txString::prepend (const char* data, unsigned long length)
{
	char* olddata = _data_;

	#if !defined OSTORE_SUPPORT
		_data_ = new char[_length_ + length + 1];
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_data_ = new(TX_GET_OBJECT_CLUSTER(this),
				os_typespec::get_char(), _length_ + length + 1)
					char[_length_ + length + 1];
		}
		else
		{
			_data_ = new char[_length_ + length + 1];
		}
	#endif

	memcpy(_data_, data, length);
	memcpy(_data_ + length, olddata, _length_);
	_data_[length + _length_] = '\0';

	_hash_ = 0;
	_length_ = length + _length_;
}

unsigned txString::hash (void) const
{
	if (_hash_)
	{
		return _hash_;
	}

	if (!length())
	{
		return 0;
	}

	((txString*) this)->_hash_ = txHash(data(), length());

	return _hash_;
}

int txString::equals (const txObject* rhs) const
{
	if (memcmp(className(), rhs->className(), 7) == 0)
	{
		txString* o = (txString*) rhs;

		if (_length_ == o->_length_)
		{
			return memcmp(data(), o->data(), _length_) == 0;
		}
	}

	return 0;
}

int txString::compare (const txObject* obj) const
{
	int v = memcmp(_data_, ((txString*) obj)->_data_, _length_);

	if (v == 0)
	{
		if (_length_ == ((txString*) obj)->_length_)
		{
			return 0;
		}
		else if (_length_ > ((txString*) obj)->_length_)
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

void txString::readObject (txIn& in)
{
	delete [] _data_; _data_ = 0;

	if (in.objectLength() || in._typeMatchCurrentHeader_(TX_EMPTY_STRING))
	{
		_resize_(in.cursor(), in.objectLength());
	}
	else
	{
		_hash_ = 0; _length_ = 0;
	}
}

void txString::writeObject (txOut& out) const
{
	if (_length_)
	{
		out.put(_data_, _length_, TX_ASCII_STRING);
	}
	else if (_data_)
	{
		out.writeUInt(TX_EMPTY_STRING);
	}
	else
	{
		out.writeHeader(out.length(), 0, TX_ASCII_STRING);
	}
}

