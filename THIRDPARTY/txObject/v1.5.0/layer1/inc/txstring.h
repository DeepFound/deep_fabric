///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXSTRING_H__ )
#define __TXSTRING_H__

#include <string.h>

#include "txobject.h"

class txString : public txObject
{
	TX_DEFINE_TYPE(txString)

	private:
		char* _data_;
		unsigned _hash_;
		unsigned long _length_;

	private:
		void _resize_ (const char* data, unsigned long length);

	public:
		txString (void);

		txString (
			const char*);

		txString (
			const char*,
			unsigned long);

		txString (const txString& obj)
		{
			_resize_(obj._data_, obj._length_);
		}

		txString& operator= (txString& obj)
		{
			if (this != &obj)
			{
				_resize_(obj._data_, obj._length_);
			}

			return *this;
		}

		~txString (void)
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

		void data (const char* data)
		{
			delete [] _data_; _data_ = 0;

			_resize_(data, strlen(data));
		}

		void data (const char* data, unsigned long length)
		{
			delete [] _data_; _data_ = 0;

			_resize_(data, length);
		}

		void append (const char* data)
		{
			append(data, strlen(data));
		}

		void append (const char* data, unsigned long len);

		void prepend (const char* data)
		{
			prepend(data, strlen(data));
		}

		void prepend (const char* data, unsigned long len);

		unsigned hash (void) const;

		int equals (const txObject* obj) const;

		int compare (const txObject* obj) const;

		void readObject (txIn& in);
		void writeObject (txOut& out) const;
};

TX_DECLARE_STREAM_TYPE_OPER(txString)

#endif // __TXSTRING_H__
