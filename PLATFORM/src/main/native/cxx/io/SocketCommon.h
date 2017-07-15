/**
 *    Copyright (C) 2010 Deep Software Foundation
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */
#ifndef CXX_IO_SOCKETCOMMON_H_
#define CXX_IO_SOCKETCOMMON_H_

#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#include "cxx/io/EOFException.h"
#include "cxx/lang/types.h"
#include "cxx/lang/Object.h"

namespace cxx { namespace io {

static const inttype MAXCONN   = 200;
static const inttype MAXEVENTS = 100;

inttype setSocketCommonFlags(inttype fd);

inttype modifyEpollContext(inttype  epollfd,
			   inttype  operation,
			   inttype  fd,
			   uinttype events,
			   voidptr  data);

} /*io */
} /** cxx */

/**
 * setSocketCommonFlags
 * Sets common socket flags.
 * @param iSd - the socket descriptor
 * @return 0 in success.
 */
FORCE_INLINE inttype cxx::io::setSocketCommonFlags(inttype iSd) {
	inttype iFlags = 0;

	errno = 0;

	iFlags = fcntl(iSd, F_GETFL, 0);
	if (-1 == iFlags) {
		return -1;
	}

	errno = 0;

	if (-1 == fcntl(iSd, F_SETFL, iFlags | O_NONBLOCK)) {
		return -1;
	}

	return 0;
}

/**
 * modifyEpollContext
 * Used to modify the epoll flags for a socket descriptor that is
 * registered with an epoll descriptor.
 * @param iEpollFd - the epoll descriptor
 * @param iOperation - EPOLL_CTL_ADD or EPOLL_CTL_DEL.
 * @param iSd - The socket descriptor
 * @param iEvents - EPOLLIN, EPOLLOUT, EPOLLET, etc or combos.
 * @param pvContext - context to associate with the socket events.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::modifyEpollContext(inttype  iEpollFd,
						 inttype  iOperation,
						 inttype  iSd,
						 uinttype iEvents,
						 voidptr  pvContext)
{
	struct epoll_event stServerListenEvent = {0};

	stServerListenEvent.events   = iEvents;
	stServerListenEvent.data.ptr = pvContext;

	errno = 0;

	if (-1 == epoll_ctl(iEpollFd, iOperation, iSd, &stServerListenEvent)) {
		return -1;
	}

	return 0;
}

#endif /** CXX_IO_SOCKETCOMMON_H_ */
