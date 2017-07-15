///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXSYNC_H__ )
#define __TXSYNC_H__

typedef void (*TX_IO_FUNC) (void* context, int fd, unsigned long id);

class txSync
{
	public:
		enum MODE
		{
			IONone, IORead, IOWrite, IOExcept
		};

	public:
		static unsigned long registerIO (
			TX_IO_FUNC func,
			void* context,
			int fd,
			MODE mode);

		static void unregisterIO (unsigned long id);

		static bool IoAndTime (long timeout);
};

#endif // __TXSYNC_H__
