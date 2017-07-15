///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txsmrtbuf.h"

txBufferBucket** txBufferMgr::_buckets_ = 0;

void txBufferMgr::_allocbuffers_ (void) 
{
	register int scale = 1;

	for (register int i = 0; i < 32; i++) 
	{
		_buckets_[i] = new txBufferBucket(scale <<= 1);
	}
}

txBufferMgr::txBufferMgr (void)
{
	_buckets_ = new txBufferBucket*[32]; 
	_allocbuffers_(); 
}

txBufferMgr::~txBufferMgr (void) 
{
	if (_buckets_)
	{
		for (int i = 0; i < 32; i++) 
		{
			delete _buckets_[i]; _buckets_[i] = 0; 
		}

		delete [] _buckets_; _buckets_ = 0;
	}
}

void txBufferMgr::put (txBufferHolder* buf) 
{
	if (_buckets_)
	{
		buf->put(); 
	}
	else
	{
		delete buf; buf = 0;
	}
}

txBufferHolder* txBufferMgr::get (unsigned long bufsize) 
{
	if (_buckets_)
	{
		register int i = 0;
		register unsigned long tmpbufsize = bufsize;

		while (tmpbufsize >>= 1) i++;

		return _buckets_[i]->get(); 
	}
	else
	{
		return 0;
	}
}

unsigned long txBufferMgr::totalAllocated (void) 
{
	register unsigned long ttl = 0; 

	for (register int i = 0; i < 32; i++) 
	{
		ttl += _buckets_[i]->allocsize * _buckets_[i]->numallocedbufs; 
	}

	return ttl; 
}

void txBufferMgr::free (unsigned long downttxize)
{
	if (_buckets_)
	{
		for (int i = 32; i != 0; i--)
		{
			if (totalAllocated() > downttxize)
			{
				_buckets_[i - 1]->free();
			}
			else
			{
				break;
			}
		}
	}
}

static txBufferMgr TX_BUFFER_MGR; 
 
void txSmartBuffer::_copy (const char* cbuf, unsigned long len) 
{
	if (len > _buf->allocsize) 
	{
		TX_BUFFER_MGR.put(_buf);
		_buf = TX_BUFFER_MGR.get(len); 
	}

	memcpy(_buf->cbuf, cbuf, len); 
	_length = len; 
}

void txSmartBuffer::_append (const char* cbuf, unsigned long len)
{
	txBufferHolder* newbuf;	
	unsigned long newlength = _length + len; 

	if (newlength > _buf->allocsize) 
	{
		newbuf = TX_BUFFER_MGR.get(newlength); 
		memcpy(newbuf->cbuf, _buf->cbuf, _length); 
		TX_BUFFER_MGR.put(_buf); _buf = newbuf; 
	}

	memcpy(_buf->cbuf + _length, cbuf, len);
	_length = newlength; 
}

void txSmartBuffer::_prepend (const char* cbuf, unsigned long len)
{
	txBufferHolder* newbuf;	
	unsigned long newlength = _length + len; 

	if (newlength > _buf->allocsize) 
	{
		newbuf = TX_BUFFER_MGR.get(newlength); 
		memcpy(newbuf->cbuf + len, _buf->cbuf, _length);
		TX_BUFFER_MGR.put(_buf); _buf = newbuf; 
	}
	else
	{
		memmove(_buf->cbuf + len, _buf->cbuf, _length); 
	}

	memcpy(_buf->cbuf, cbuf, len);
	_length = newlength; 
}

void txSmartBuffer::_place (const char* cbuf, unsigned long len,
	unsigned long at) 
{
	txBufferHolder* newbuf;	
	unsigned long newlength = _length; 

	if ((at+len) > _length) 
	{
		newlength = at + len; 
	}

	if (newlength > _buf->allocsize) 
	{
		newbuf = TX_BUFFER_MGR.get(newlength); 
		memcpy(newbuf->cbuf, _buf->cbuf, _length); 
		TX_BUFFER_MGR.put(_buf); _buf = newbuf; 
	}

	memcpy(_buf->cbuf+at, cbuf, len);
	_length = newlength; 
}

void txSmartBuffer::_insert (const char* cbuf, unsigned long len,
	unsigned long at) 
{
	txBufferHolder* newbuf;	
	unsigned long newlength = _length + len; 

	if (newlength > _buf->allocsize) 
	{
		newbuf = TX_BUFFER_MGR.get(newlength); 

		memcpy(newbuf->cbuf, _buf->cbuf, at);

		memcpy(
			newbuf->cbuf + at + len,
			_buf->cbuf + at,
			_length - at);

		TX_BUFFER_MGR.put(_buf); _buf = newbuf; 
	}
	else
	{
		memmove(
			_buf->cbuf + at + len,
			_buf->cbuf + at,
			_length - at);
	}

	memcpy(_buf->cbuf+at, cbuf, len);
	_length = newlength; 
}

txSmartBuffer::txSmartBuffer (unsigned long size) :
	_length(0) 
{
	_buf = TX_BUFFER_MGR.get(size); 
}

txSmartBuffer::txSmartBuffer (const char* cbuf, unsigned long size) :
	_length(size) 
{
	_buf = TX_BUFFER_MGR.get(size); 
	memcpy(_buf->cbuf, cbuf, size); 
}

txSmartBuffer::txSmartBuffer (const txSmartBuffer& b) :
	_length(b._length) 
{
	_buf = TX_BUFFER_MGR.get(_length); 
	memcpy(_buf->cbuf, b._buf->cbuf, _length); 
}

txSmartBuffer::~txSmartBuffer (void) 
{
	if (_buf)
	{
		TX_BUFFER_MGR.put(_buf); _buf = 0;
	}
}

void txSmartBuffer::clear (void)
{
	_length = 0;
	TX_BUFFER_MGR.put(_buf);
	_buf = TX_BUFFER_MGR.get(0); 
}

