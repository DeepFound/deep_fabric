///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXSMRTBUF_H__ )
#define __TXSMRTBUF_H__

#include <string.h>

class txBufferBucket;
 
class txBufferHolder 
{
	public:
		char* cbuf;
		unsigned long allocsize;
		txBufferHolder* nextbuf;
		txBufferBucket* mybucket;

		txBufferHolder (unsigned long bufsize, txBufferBucket* bucket,
			txBufferHolder* next = 0):
			allocsize(bufsize), nextbuf(next), mybucket(bucket)
		{
			cbuf = new char[bufsize]; 
		}
	
		~txBufferHolder (void) 
		{
			delete nextbuf; nextbuf = 0; 
			delete [] cbuf; cbuf = 0; 
		}
	
		inline void put (void);
};

class txBufferBucket 
{
	public:
		unsigned long allocsize;
		unsigned long numallocedbufs;

		txBufferHolder* nextfreebuf;

		txBufferBucket (unsigned long bsize) : 
			allocsize(bsize), numallocedbufs(0), nextfreebuf(0) 
		{
		}

		~txBufferBucket (void) 
		{
			delete nextfreebuf; nextfreebuf = 0;
			numallocedbufs = 0;
		}

		txBufferHolder* get (void) 
		{
			txBufferHolder* retbuf = 0; 

			if (nextfreebuf)
			{
				retbuf = nextfreebuf; 
				nextfreebuf = retbuf->nextbuf; 
				retbuf->nextbuf = 0; 
			}
			else
			{
				retbuf = new txBufferHolder(allocsize, this); 
				numallocedbufs++; 
			}

			return retbuf; 
		}

		void put (txBufferHolder* buf) 
		{
			buf->nextbuf = nextfreebuf; 
			nextfreebuf = buf; 
		}

		void free (void)
		{
			txBufferHolder* tmpbuf = nextfreebuf;

			while (tmpbuf)
			{
				tmpbuf = tmpbuf->nextbuf;
				numallocedbufs--;
			}

			delete nextfreebuf; nextfreebuf = 0; 
		}
};

inline void txBufferHolder::put (void)
{
	mybucket->put(this);
}

class txBufferMgr 
{
	private: 
		static txBufferBucket** _buckets_;

	private: 
		static void _allocbuffers_ (void); 

	public: 
		txBufferMgr (void);

		~txBufferMgr (void); 

		static void put (txBufferHolder*); 

		static txBufferHolder* get (unsigned long); 

		static unsigned long totalAllocated (void); 

		static void free (unsigned long downttxize);
};
 
class txSmartBuffer
{
	protected: 
		txBufferHolder* _buf;
		unsigned long _length;

		void _copy (const char*, unsigned long);

		void _append (const char*, unsigned long);

		void _prepend (const char*, unsigned long);

		void _place (const char*, unsigned long, unsigned long); 

		void _insert (const char*, unsigned long, unsigned long); 

	public: 
		txSmartBuffer (unsigned long size = 0);

		txSmartBuffer (const char* cbuf, unsigned long size);

		txSmartBuffer (const txSmartBuffer& b);

		txSmartBuffer& operator= (const txSmartBuffer& b); 

		~txSmartBuffer (void); 

		void append (const txSmartBuffer& b)
		{
			_append(b._buf->cbuf, b._length); 
		}

		void append (const char* cbuf)
		{
			_append(cbuf, strlen((const char*) cbuf) + 1); 
		}

		void append (const char* cbuf, unsigned long len)
		{
			_append(cbuf, len); 
		}

		void prepend (const txSmartBuffer& b)
		{
			_prepend(b._buf->cbuf, b._length); 
		}

		void prepend (const char* cbuf)
		{
			_prepend(cbuf, strlen((const char*) cbuf) + 1); 
		}

		void prepend (const char* cbuf, unsigned long len)
		{
			_prepend(cbuf, len); 
		}

		void place (const txSmartBuffer& b, unsigned long at)
		{
			_place(b._buf->cbuf, b._length, at); 
		}

		void place (const char* cbuf, unsigned long len,
			unsigned long at)
		{
			_place(cbuf, len, at); 
		}

		void insert (const txSmartBuffer& b, unsigned long at)
		{
			_insert(b._buf->cbuf, b._length, at); 
		}

		void insert (const char* cbuf, unsigned long len,
			unsigned long at)
		{
			_insert(cbuf, len, at); 
		}

		const char* data (void) const
		{
			return _buf->cbuf;
		}

		unsigned long length (void) const
		{
			return _length;
		}

		const char* at (unsigned long at)
		{
			return _buf->cbuf + at;
		}

		void clear (void);
};

#endif // __TXSMRTBUF_H__
