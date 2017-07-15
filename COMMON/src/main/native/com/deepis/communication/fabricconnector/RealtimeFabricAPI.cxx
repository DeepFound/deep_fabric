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
#include "RealtimeFabricAPI.h"
#include "cxx/util/HashMap.cxx"
#include "cxx/util/Logger.h"

using namespace cxx::lang;
using namespace cxx::util;

/**
 * createDomainSocket
 * Primary interface to the API is a non-blocking, streaming
 * UNIX domain socket. This will be named using the provided path name.
 * @param cName - the path name to call the socket.
 * @return 0 on success.
 */
inttype com::deepis::communication::fabricconnector::
RealtimeFabricAPIServer::createDomainSocket(cxx::lang::String cName) {
	m_cAddress  = cName;

	m_iServerSd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (-1 == m_iServerSd) {
		throw RealtimeFabricAPIServerException("Unable to create the domain socket.");
	}

	unlink(m_cAddress);

	if (-1 == cxx::io::setSocketCommonFlags(m_iServerSd)) {
		throw RealtimeFabricAPIServerException("Unable to set nonblocking domain socket.");
	}

	struct sockaddr_un addr = { 0 };
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, cName.c_str(), cName.size());

	if (-1 == bind(m_iServerSd, (struct sockaddr*)&addr, sizeof(addr))) {
		DEEP_LOG_ERROR(OTHER, "Unable to bind on the domain socket name: %s\n", m_cAddress.c_str());
		throw RealtimeFabricAPIServerException(m_cAddress);
	}

	if (-1 == listen(m_iServerSd, 5)) {
		DEEP_LOG_ERROR(OTHER, "Unable to listen on the domain socket name: %s\n", m_cAddress.c_str());
		throw RealtimeFabricAPIServerException(m_cAddress);
	}

	DEEP_LOG_DEBUG(OTHER, "createDomainSocket() successful: %s\n", m_cAddress.c_str());

	m_iEpollFd = epoll_create(cxx::io::MAXCONN);
	if (-1 == m_iEpollFd) {
		throw RealtimeFabricAPIServerException("Unable to create an epoll FD\n");
	}

	if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_ADD, m_iServerSd, EPOLLIN, &m_iServerSd)) {
		throw RealtimeFabricAPIServerException("Unable to add domain socket to the epoll set\n");
	}

	return 0;
}

/**
 * createTcpSocket
 * Primary interface to the API is a non-blocking, streaming
 * UNIX domain socket. This will be named using the provided path name.
 * @param cName - the path name to call the socket.
 * @return 0 on success.
 */
inttype com::deepis::communication::fabricconnector::
RealtimeFabricAPIServer::createTcpSocket(ushorttype iPort) {
	m_iServerSd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == m_iServerSd) {
		throw RealtimeFabricAPIServerException("Unable to create the TCP listening socket.");
	}

	if (-1 == cxx::io::setSocketCommonFlags(m_iServerSd)) {
		throw RealtimeFabricAPIServerException("Unable to set nonblocking TCP listening socket.");
	}

	struct sockaddr_in addr = { 0 };
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(iPort);

	inttype iVal    = 1;
	if (0 != setsockopt(m_iServerSd,
			    SOL_SOCKET,
			    SO_REUSEADDR,
			    &iVal,
			    sizeof(inttype))) {
		throw RealtimeFabricAPIServerException("Unable to set socket reuse option.");
	}

	errno = 0;

	if (-1 == bind(m_iServerSd, (struct sockaddr *)&addr, sizeof(addr))) {
		DEEP_LOG_ERROR(OTHER, "Unable to bind on the TCP IP Address. \n");
		throw RealtimeFabricAPIServerException("Unable to bind on the TCP IP Address.");
	}


	errno = 0;

	if (-1 == listen(m_iServerSd, 5)) {
		DEEP_LOG_ERROR(OTHER, "Unable to listen on the TCP IP Address.\n");
		throw RealtimeFabricAPIServerException("Unable to listen on the TCP IP Address.");
	}

	DEEP_LOG_DEBUG(OTHER, "createTcpSocket(%u) successful.\n", iPort);

	errno = 0;

	m_iEpollFd = epoll_create(cxx::io::MAXCONN);
	if (-1 == m_iEpollFd) {
		throw RealtimeFabricAPIServerException("Unable to create an epoll FD\n");
	}

	errno = 0;

	if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_ADD, m_iServerSd, EPOLLIN, &m_iServerSd)) {
		throw RealtimeFabricAPIServerException("Unable to add domain socket to the epoll set\n");
	}

	return 0;
}

/**
 * poll
 * Called from the thread managing the API server. Waits iTimeOut for
 * any socket events to process. We either accept a connection,
 * service client connection read events or we are posting write
 * events to the client connection. Client end is expected to close
 * the connection upon completion. POLL handles the REQ/REP
 * communications pattern. One REQ from a client, followed by a REP
 * from the Server API, before the next REQ can be sent to the server.
 * @param iTimeout - milliseconds to timeout waiting for an event
 * before relinquishing control.
 * @return 0 on success. Can throw RealtimeFabricAPIServerException
 * in error cases.
 */
inttype com::deepis::communication::fabricconnector::
RealtimeFabricAPIServer::poll(ulongtype iTimeout) {
	struct sockaddr_in stClientaddr = {0};
	socklen_t          iClientlen   = sizeof(stClientaddr);

	inttype iEventCount = epoll_wait(m_iEpollFd, m_stEvents, cxx::io::MAXEVENTS, iTimeout);
	if (-1 == iEventCount) {
		return -1;
	}

	for (inttype i=0; i<iEventCount; i++) {
		if (m_stEvents[i].data.ptr == &m_iServerSd) {
			if ((EPOLLHUP & m_stEvents[i].events) || (EPOLLERR & m_stEvents[i].events)) {
				/**
				 * EPOLLHUP and EPOLLERR are always monitored.
				 */
				close(m_iServerSd);
				DEEP_LOG_ERROR(OTHER, "Domain socket closed!\n");
				throw RealtimeFabricAPIServerException("Domain socket closed!");
			}

			inttype iClientSd = accept(m_iServerSd, (struct sockaddr*)&stClientaddr, &iClientlen);
			if (-1 == iClientSd) {
				DEEP_LOG_ERROR(OTHER, "Unable to get the Client SD.\n");
				throw RealtimeFabricAPIServerException("Unable to get the Client SD.");
			}

			ClientEvent* pcClient = new ClientEvent;
			pcClient->m_iSd = iClientSd;

			if (-1 == cxx::io::setSocketCommonFlags(iClientSd)) {
				throw RealtimeFabricAPIServerException("Unable to set nonblocking domain socket.");
			}

			if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_ADD, iClientSd, EPOLLIN, pcClient)) {
				throw RealtimeFabricAPIServerException("Unable to add client socket to epoll set.");
			}
		} else {
			/** Client Connection */
			ClientEvent* pcClient = static_cast<ClientEvent*>(m_stEvents[i].data.ptr);
			if (0 == pcClient) {
				throw RealtimeFabricAPIServerException("Unable to get the client event block\n");
			}

			if ((EPOLLHUP & m_stEvents[i].events) || (EPOLLERR & m_stEvents[i].events)) {
				/**
				 * EPOLLHUP and EPOLLERR are always monitored.
				 */
				close(pcClient->m_iSd);
				DEEP_LOG_DEBUG(OTHER, "Client socket %d has closed.\n", pcClient->m_iSd);
				delete pcClient; pcClient = 0;

			} else if (EPOLLIN  == m_stEvents[i].events) {
				pcClient->m_iEvent = EPOLLIN;

				/**
				 * We expect only one buffer from the
				 * Client. We assume MAXLEN which
				 * should be plenty. Hence we stop
				 * looking for more read events.
				 */
				if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_DEL, pcClient->m_iSd, 0, 0)) {
					throw RealtimeFabricAPIServerException("Unable to remove client EPOLLIN event.\n");
				}

				if (-1 == processClientEvents(pcClient)) {
					throw RealtimeFabricAPIServerException("Issues processing client events.");
				}

			} else if (EPOLLOUT == m_stEvents[i].events) {
				pcClient->m_iEvent = EPOLLOUT;
				if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_DEL, pcClient->m_iSd, 0, 0)) {
					throw RealtimeFabricAPIServerException("Unable to remove client EPOLLOUT event.\n");
				}

				if (-1 == processClientEvents(pcClient)) {
					throw RealtimeFabricAPIServerException("Issues processing client events.");
				}

			}
		}
	}

	return 0;
}

/**
 * processClientEvents
 * Processes a socket context block that was asscociated in the EPOLL
 * FD set.
 * @param pcClient - the client context block.
 * @return 0 on success.
 */
inttype com::deepis::communication::fabricconnector::
RealtimeFabricAPIServer::processClientEvents(ClientEvent* pcClient) {
	errno = 0;

	if (EPOLLIN == pcClient->m_iEvent) {
		inttype iRead = read(pcClient->m_iSd, pcClient->m_iBuffer, MAXLEN);
		if (0 == iRead) {
			/** Nothing to read. Close the session. */
			close(pcClient->m_iSd);
			delete pcClient; pcClient = 0;
			return 0;
		}

		if (-1 == iRead) {
			switch (errno) {
			case EINTR:
			case EWOULDBLOCK:
				/**
				 * All these errors mean something
				 * blocked our read. So set the event and
				 * try again.
				 */
				if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_ADD, pcClient->m_iSd, EPOLLIN, pcClient)) {
					throw RealtimeFabricAPIServerException("Unable to add client socket to epoll set.");
				}

				break;

			default:
				/**
				 * Any other error means we close the connection.
				 */
				close(pcClient->m_iSd);
				delete pcClient; pcClient = 0;
				return 0;
			}
		}

		/**
		 * Call the client handler and see if we have data to write
		 * back to the client.
		 */
		pcClient->m_iWriteSize = handler(pcClient->m_iBuffer, iRead);
		if (0 != pcClient->m_iWriteSize) {
			/** We have a response to now send. */
			if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_ADD, pcClient->m_iSd, EPOLLOUT, pcClient)) {
				throw RealtimeFabricAPIServerException("Unable to add client socket to epoll set.");
			}
		} else {
			/** No response to send. Close the session. */
			close(pcClient->m_iSd);
			delete pcClient; pcClient = 0;
		}

	} else if (EPOLLOUT == pcClient->m_iEvent) {
		inttype iWrote = write(pcClient->m_iSd,
				       &pcClient->m_iBuffer[pcClient->m_iOffset],
				       pcClient->m_iWriteSize);

		if (-1 == iWrote) {
			switch (errno) {
			case EINTR:
			case EWOULDBLOCK:
				/**
				 * All these errors mean something
				 * blocked our write. So set the event and
				 * try again.
				 */
				if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_ADD, pcClient->m_iSd, EPOLLOUT, pcClient)) {
					throw RealtimeFabricAPIServerException("Unable to add client socket to epoll set.");
				}
				break;
			default:
				break;
			}
			
			/** Remote HUP? Close the connection. */
			close(pcClient->m_iSd);
			delete pcClient; pcClient = 0;

		} else if (pcClient->m_iWriteSize > iWrote) { /** We partially wrote the response. */
			if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_ADD, pcClient->m_iSd, EPOLLOUT, pcClient)) {
				throw RealtimeFabricAPIServerException("Unable to add client socket to epoll set.");
			}

			/*
			 * The previous write wrote only partial data to the socket.
			 */
			pcClient->m_iOffset    += iWrote;
			pcClient->m_iWriteSize -= iWrote;
		} else {
			/**
			 * All data is sent. We can now wait on more
			 * from the client. Client will close the
			 * connection if there is nothing more that it
			 * needs.
			 */
			if (-1 == cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_ADD, pcClient->m_iSd, EPOLLIN, pcClient)) {
				throw RealtimeFabricAPIServerException("Unable to add client socket to epoll set.");
			}

			pcClient->m_iWriteSize = 0;
			pcClient->m_iOffset    = 0;
			
		}
	}

	return 0;
}
