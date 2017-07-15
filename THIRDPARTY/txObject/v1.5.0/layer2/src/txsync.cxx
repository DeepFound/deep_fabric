///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if defined TX_SGI
	#define _ABI_SOURCE = 1
#endif

#if defined TX_SOL || TX_SUNOS || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	#include <sys/time.h>
	#include <unistd.h>
#endif

#if defined TX_SOL || TX_SGI || TX_HP || TX_DEC || TX_LINUX || TX_MAC
	#include <sys/select.h>
#endif

#if defined TX_WIN
	#include <winsock2.h>

	#define close closesocket
#endif

#include "txlist.h"
#include "txsync.h"

class FdObject
{
	public:
		FdObject* _prev_;
		FdObject* _next_;

		txSync::MODE mode;
		TX_IO_FUNC func;
		void* context;
		int fd;

	public:
		FdObject():
			_prev_(0),
			_next_(0)
		{
		}

		~FdObject()
		{
		}

	friend class fdList;
};

class fdList
{
	private:
		int _entries_;
		FdObject* _head_;
		FdObject* _tail_;

	public:
		fdList (void) :
			_entries_(0),
			_head_(0),
			_tail_(0)
		{
		}

		~fdList (void)
		{
		}

		const FdObject* add (FdObject* x)
		{
			if (_head_ == 0)
			{
				_head_ = x;
				x->_prev_ = 0;
			}
			else
			{
				_tail_->_next_ = x;
				x->_prev_ = _tail_;
			}

			_tail_ = x;
			x->_next_ = 0;

			_entries_++;

			return x;
		}

		FdObject* remove (FdObject* x)
		{
			if (x->_prev_ == 0)
			{
				_head_ = x->_next_;
			}
			else
			{
				x->_prev_->_next_ = x->_next_;
			}

			if (x->_next_ == 0)
			{
				_tail_ = x->_prev_;
			}
			else
			{
				x->_next_->_prev_ = x->_prev_;
			}

			_entries_--;

			return x;
		}

		FdObject* get (void)
		{
			return _entries_ ? remove(_head_) : 0;
		}

		int entries (void) const
		{
			return _entries_;
		}
};

static fd_set THE_ifdset;
static fd_set THE_ofdset;
static fd_set THE_efdset;

static int THE_MAX_FD = 0;
static fdList THE_EX_LIST;
static txList THE_FD_LIST;
static txListIterator THE_FD_ITER(THE_FD_LIST);

static void startupInputOutput (void)
{
	FD_ZERO(&THE_ifdset);
	FD_ZERO(&THE_ofdset);
	FD_ZERO(&THE_efdset);

#if defined TX_WIN
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	txSync::registerIO(0, 0, fd, txSync::IORead);
#endif
}

static void shutdownInputOutput (void)
{
	FdObject* fds;

	while (fds = (FdObject*) THE_FD_LIST.get())
	{
		close(fds->fd);
		delete fds; fds = 0;
	}

#if defined TX_WIN
	WSACleanup();
#endif
}

TX_STATIC_ALLOC_NOTIFY(startupInputOutput)
TX_STATIC_DEALLOC_NOTIFY(shutdownInputOutput)

static void setMaximum ()
{
	THE_MAX_FD = 0;

	FdObject* fds;
	txListIterator iter(THE_FD_ITER);

	while (fds = (FdObject*) iter.next())
	{
		if (fds->fd > THE_MAX_FD)
		{
			THE_MAX_FD = fds->fd;
		}
	}
}

unsigned long txSync::registerIO (TX_IO_FUNC func, void* context, int fd,
	MODE mode)
{
	FdObject* fds = 0;

	if (fd < 0)
	{
		return 0;
	}
	else if (fd > THE_MAX_FD)
	{
		THE_MAX_FD = fd;
	}

	switch (mode)
	{
		case IORead:
		{
			fds = new FdObject();
			FD_SET(fd, &THE_ifdset);
			THE_FD_LIST.append((txObject*) fds);
			break;
		}
		case IOWrite:
		{
			fds = new FdObject();
			FD_SET(fd, &THE_ofdset);
			THE_FD_LIST.append((txObject*) fds);
			break;
		}
		case IOExcept:
		{
			fds = new FdObject();
			FD_SET(fd, &THE_efdset);
			THE_FD_LIST.append((txObject*) fds);
			break;
		}
		case IONone:
		{
			break;
		}
	}

	if (fds)
	{
		fds->mode = mode;
		fds->fd = fd;
		fds->func = func;
		fds->context = context;
	}

	return (unsigned long) fds;
}

void txSync::unregisterIO (unsigned long id)
{
	FdObject* fds;
	txListIterator iter(THE_FD_LIST);

	while (fds = (FdObject*) iter.next())
	{
		if (id == (unsigned long) fds)
		{
			iter.remove();

			if (fds->mode == IORead)
			{
				FD_CLR(fds->fd, &THE_ifdset);
			}
			else if (fds->mode == IOWrite)
			{
				FD_CLR(fds->fd, &THE_ofdset);
			}
			else if (fds->mode == IOExcept)
			{
				FD_CLR(fds->fd, &THE_efdset);
			}

			delete fds; fds = 0;

			setMaximum();
		}
	}
}

bool txSync::IoAndTime (long min_timeout)
{
	static int fd;
	static fd_set t_ifdset;
	static fd_set t_ofdset;
	static fd_set t_efdset;
	static struct timeval xper = {0,0};

	memcpy(&t_ifdset, &THE_ifdset, sizeof(fd_set));
	memcpy(&t_ofdset, &THE_ofdset, sizeof(fd_set));
	memcpy(&t_efdset, &THE_efdset, sizeof(fd_set));

	if (min_timeout != -1)
	{
		xper.tv_sec = (long) (min_timeout * 0.001);
		xper.tv_usec = (min_timeout % 1000) * 1000;

		fd = select((THE_MAX_FD + 1),
			&t_ifdset, &t_ofdset, &t_efdset, &xper);
	}
	else
	{
		fd = select((THE_MAX_FD + 1),
			&t_ifdset, &t_ofdset, &t_efdset, 0);
	}

	if (fd > 0)
	{
		FdObject* fds;
		THE_FD_ITER.reset();

		while (fds = (FdObject*) THE_FD_ITER.next())
		{
			if (FD_ISSET(fds->fd, &t_ifdset))
			{
				THE_EX_LIST.add(fds);
			}
			else if (FD_ISSET(fds->fd, &t_ofdset))
			{
				THE_EX_LIST.add(fds);
			}
			else if (FD_ISSET(fds->fd, &t_efdset))
			{
				THE_EX_LIST.add(fds);
			}
		}

		while (fds = THE_EX_LIST.get())
		{
			fds->func(fds->context, fds->fd, (unsigned long) fds);
		}
	}

	return (fd != -1);
}

