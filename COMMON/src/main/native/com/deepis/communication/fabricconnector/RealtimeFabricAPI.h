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
#ifndef CXX_FABRIC_REALTIMEFABRICAPI_H_
#define CXX_FABRIC_REALTIMEFABRICAPI_H_

#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "cxx/io/EOFException.h"
#include "cxx/io/SocketCommon.h"
#include "cxx/lang/types.h"
#include "cxx/util/HashMap.h"
#include "cxx/util/ArrayList.h"
#include "cxx/util/UUID.h"

namespace com { namespace deepis { namespace communication { namespace fabricconnector {

static const inttype MAXLEN    = 256*1024;

class RealtimeFabricAPIServerException : public Exception {
public:
	RealtimeFabricAPIServerException(void) {
	}

	RealtimeFabricAPIServerException(const char* message)
		: Exception(message) {
	}
};

/**
 * RealtimeFabricAPIServer
 */
class RealtimeFabricAPIServer : public cxx::lang::Object {
public:
	RealtimeFabricAPIServer();
	virtual ~RealtimeFabricAPIServer();

	inttype createDomainSocket(cxx::lang::String cName);
	inttype createTcpSocket(ushorttype iPort);

	inttype poll(ulongtype iTimeout);

	virtual inttype handler(bytetype* piReadBuffer,
				inttype   iReadSize);

private:
	struct ClientEvent {
		ClientEvent();
		~ClientEvent();

		inttype           m_iSd;
		uinttype          m_iEvent;
		inttype           m_iOffset;
		bytetype          m_iBuffer[MAXLEN];
		inttype           m_iWriteSize;
	}; /** Client Event */

	inttype processClientEvents(ClientEvent* pcEvent);

	cxx::lang::String  m_cAddress;
	inttype            m_iServerSd;
	inttype            m_iEpollFd;
	struct epoll_event m_stEvents[cxx::io::MAXEVENTS];

}; /** RealtimeFabricAPIServer */

} // fabricconnector
} // communication
} // deepis
} // com

/** RealtimeFabricAPIServer::ClientEvent */
FORCE_INLINE com::deepis::communication::fabricconnector::
RealtimeFabricAPIServer::ClientEvent::ClientEvent()
	: m_iSd(-1),
	  m_iEvent(0),
	  m_iOffset(0),
	  m_iWriteSize(0) {

	memset(m_iBuffer, 0, MAXLEN);
}

FORCE_INLINE com::deepis::communication::fabricconnector::
RealtimeFabricAPIServer::ClientEvent::~ClientEvent() {
	m_iSd        = -1;
	m_iEvent     = 0;
	m_iOffset    = 0;
	m_iWriteSize = 0;
}

/** RealtimeFabricAPIServer */

/**
 * RealtimeFabricAPIServer
 * Constructs the API Server.
 */
FORCE_INLINE com::deepis::communication::fabricconnector::
RealtimeFabricAPIServer::RealtimeFabricAPIServer()
	: m_iServerSd(-1), m_iEpollFd(-1) {

	memset(m_stEvents, 0, sizeof(m_stEvents));
}

/**
 * ~RealtimeFabricAPIServer
 * Destructor
 */
FORCE_INLINE com::deepis::communication::fabricconnector::
RealtimeFabricAPIServer::~RealtimeFabricAPIServer() {
	unlink(m_cAddress);

	if (-1 != m_iServerSd) {
		close(m_iServerSd);
	}
}

/**
 * handler
 * Overload this to handle the socket events by the governing thread
 * context.
 * @param piReadBuffer- buffer pointer with the data to process
 * @param iReadSize - the size of the read data.
 * @return 0 if nothing to return to the client. Otherwise write at
 * most 64KiB of data into the ReadBuffer and return the length
 * written to indicate the data should be returned to the client.
 */
FORCE_INLINE inttype com::deepis::communication::fabricconnector::
RealtimeFabricAPIServer::handler(bytetype* piReadBuffer,
				 inttype   iReadSize) {
	return 0;
}

#endif /** CXX_FABRIC_REALTIMEFABRICAPI_H_ */
