///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXBYTEARRAY_H__ )
#define __TXBYTEARRAY_H__

#include "txobject.h"

class txByteArray : public txObject
{
	TX_DEFINE_TYPE(txByteArray);

	private:
		char* _data_;
		unsigned _hash_;
		unsigned long _length_;

	private:
		void _resize_ (const char* data, unsigned long length);

	public:
		txByteArray (void);

		txByteArray (
			const char*,
			unsigned long);

		txByteArray (const txByteArray& obj)
		{
			_resize_(obj._data_, obj._length_);
		}

		txByteArray& operator= (txByteArray& obj)
		{
			if (this != &obj)
			{
				_resize_(obj._data_, obj._length_);
			}

			return *this;
		}

		~txByteArray (void)
		{
			delete [] _data_; _data_ = 0;
		}

		unsigned long length (void) const
		{
			return _length_;
		}

		const char* data (void) const
		{
			return _data_;
		}

		void data (const char* data, unsigned long length)
		{
			delete [] _data_; _data_ = 0;

			_resize_(data, length);
		}

		void append (const char* data)
		{
			append(data, strlen(data) + 1);
		}

		void append (const char* data, unsigned long len);

		void prepend (const char* data, unsigned long len);

		unsigned hash (void) const;

		int equals (const txObject* rhs) const;

		int compare (const txObject* obj) const;

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txByteArray)

#endif // __TXBYTEARRAY_H__
