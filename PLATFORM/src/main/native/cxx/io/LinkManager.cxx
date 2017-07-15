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
#include <unistd.h>
#include <sstream>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "LinkManager.h"
#include "cxx/util/Logger.h"
#include "cxx/util/HashMap.cxx"

using namespace cxx::lang;
using namespace cxx::util;

#define EFSM_CAUSES_CRASH 1

static const ulongtype OVERLOAD_ON_RECV_MS = 250;
static const ulongtype OVERLOAD_ON_SEND_MS = 250;

/**
 * LinkManager
 * Default Constructor.
 */
cxx::io::LinkManager::LinkManager()
	:  m_iSockets(0),
	   m_iSocketsGrowth(10),
	   m_pcServerSockets(0),
	   m_pcOutgoing(0),
	   m_pcContext(0),
	   m_pcPollItems(0),
	   m_pcPollToCallback(0),
	   m_pcCallbacks(0),
	   m_pcRevSockMap(0),
	   m_pcFdToIp(0),
	   m_pcNotifCb(0),
	   m_pcNativeCbs(0) {

	m_pcServerSockets = new cxx::util::HashMap<cxx::lang::String*, socketInfo* >(16, true, true, false);
	if (0 == m_pcServerSockets) {
		throw LinkManagerException("Unable to allocate ServerSockets map.");
	}

	m_pcOutgoing = new cxx::util::HashMap<cxx::lang::String*, socketInfo* >(16, true, true, false);
	if (0 == m_pcOutgoing) {
		throw LinkManagerException("Unable to allocate Outgoing map.");
	}

	m_pcCallbacks = new cxx::util::HashMap<voidptr, socketCallback* >(16, false, false, false);
	if (0 == m_pcCallbacks) {
		throw LinkManagerException("Unable to allocate Callbacks map.");
	}

	m_pcRevSockMap = new cxx::util::HashMap<voidptr, cxx::lang::String* >(16, false, false, false);
	if (0 == m_pcRevSockMap) {
		throw LinkManagerException("Unable to allocate RevSocket map.");
	}

	m_pcFdToIp = new cxx::util::HashMap<uinttype, cxx::lang::String* >(16, false, true, false);
	if (0 == m_pcFdToIp) {
		throw LinkManagerException("Unable to allocate the FD map.");
	}

	m_pcContext = zmq_ctx_new();
	if (0 == m_pcContext) {
		throw LinkManagerException("Unable to create a ZMQ instance.");
	}

	m_pcPollItems = new zmq_pollitem_t[m_iSocketsGrowth];
	if (0 == m_pcPollItems) {
		throw LinkManagerException("Just cannot allocate pollitems array.");
	}

	m_pcPollToCallback = new socketCallbackPtr[m_iSocketsGrowth];
	if (0 == m_pcPollToCallback) {
		throw LinkManagerException("Just cannot allocate pollitem to callback array array.");
	}

	m_pcNativeCbs = new cxx::util::HashMap<inttype, socketCallback* >(16, false, false, false);
	if (0 == m_pcNativeCbs) {
		throw LinkManagerException("Just cannot allocate native sockets map.");
	}
}

/**
 * ~LinkManager
 * Destructor
 */
cxx::io::LinkManager::~LinkManager() {
	stopService();
}

/**
 * stopService
 * This permanently stops the LinkManager service. Lots of cleanup
 * and subsystem shutdown.
 * @return 0 on success.
 */
ulongtype cxx::io::LinkManager::stopService(void) {
	if (0 != m_pcServerSockets) {
		cxx::util::Set<cxx::lang::String*>*       pcSet = m_pcServerSockets->keySet();
		cxx::util::Iterator<cxx::lang::String* >* pcIt  = pcSet->iterator();

		while (true == pcIt->hasNext()) {
			cxx::lang::String* pcName = pcIt->next();
			const socketInfo*  pcInfo = m_pcServerSockets->get(pcName);
			if (0 != pcInfo) {
				zmq_close(pcInfo->m_socket);
				zmq_close(pcInfo->m_monitor);
			}
		}

		delete pcIt;  pcIt  = 0;
		delete pcSet; pcSet = 0;
	}

	if (0 != m_pcServerSockets) {
		delete m_pcServerSockets; m_pcServerSockets = 0;
	}

	if (0 != m_pcOutgoing) {
		cxx::util::Set<cxx::lang::String*>*       pcSet = m_pcOutgoing->keySet();
		cxx::util::Iterator<cxx::lang::String* >* pcIt  = pcSet->iterator();
	
		while (true == pcIt->hasNext()) {
			cxx::lang::String* pcName = pcIt->next();
			const socketInfo*  pcInfo = m_pcOutgoing->get(pcName);
			if (0 != pcInfo) {
				zmq_close(pcInfo->m_socket);
				zmq_close(pcInfo->m_monitor);
			}
		}

		delete pcIt;  pcIt  = 0;
		delete pcSet; pcSet = 0;
	}

	if (0 != m_pcOutgoing) {
		delete m_pcOutgoing; m_pcOutgoing = 0;
	}

	if (0 != m_pcCallbacks) {
		delete m_pcCallbacks; m_pcCallbacks = 0;
	}

	if (0 != m_pcRevSockMap) {
		delete m_pcRevSockMap; m_pcRevSockMap = 0;
	}

	if (0 != m_pcFdToIp) {
		delete m_pcFdToIp; m_pcFdToIp = 0;
	}

	if (0 != m_pcNativeCbs) {
		delete m_pcNativeCbs; m_pcNativeCbs = 0;
	}

	if (0 != m_pcContext) {
		/**  Safe to terminate the zmq context. */
		boolean bTryAgain = false;
		do {
			/**
			 * This should be a blocking call to close
			 * any open sockets, unblock and terminate any
			 * worker threads.
			 */
			errno = 0;
			inttype status = zmq_ctx_term(m_pcContext);

			if (0 == status) {
				DEEP_LOG_INFO(OTHER, "ZMQ Terminated successfully\n");
				bTryAgain = false;
			} else if (-1 == status) {
				if (EFAULT == errno) {
					DEEP_LOG_ERROR(OTHER, "Invalid ZMQ Context to destroy!\n");
					abort();
				} else if (EINTR == errno) {
					DEEP_LOG_WARN(OTHER, "ZMQ Termination was interrupted. Trying again.\n");
					bTryAgain = true;
				}
			}
		} while (true == bTryAgain);
	}

	m_pcContext = 0;

	if (0 != m_pcPollItems) {
		delete [] m_pcPollItems; m_pcPollItems = 0;
	}

	if (0 != m_pcPollToCallback) {
		delete [] m_pcPollToCallback; m_pcPollToCallback = 0;
	}

	return 0;
}

/**
 * RenderPollItems
 * Render the poll array needed by the zmq context to poll on all of
 * our active sockets.
 * @return 0 on success, else error.
 */
ulongtype cxx::io::LinkManager::RenderPollItems() {
	if (m_iSockets > m_iSocketsGrowth) {
		/** Grow the array blocks. */
		m_iSocketsGrowth += 10;

		if (0 != m_pcPollItems) {
			delete [] m_pcPollItems; m_pcPollItems = 0;
		}

		if (0 != m_pcPollToCallback) {
			delete [] m_pcPollToCallback; m_pcPollToCallback = 0;
		}
	
		m_pcPollItems = new zmq_pollitem_t[m_iSocketsGrowth];
		if (0 == m_pcPollItems) {
			DEEP_LOG_ERROR(OTHER, "Just cannot allocate pollitems array.");
			abort();
		}

		m_pcPollToCallback = new socketCallbackPtr[m_iSocketsGrowth];
		if (0 == m_pcPollToCallback) {
			DEEP_LOG_ERROR(OTHER, "Just cannot allocate pollitem to callback array array.");
			abort();
		}
	}

	uinttype i = 0;

	cxx::util::Set<cxx::lang::String* >*      pcSet = m_pcServerSockets->keySet();
	cxx::util::Iterator<cxx::lang::String* >* pcIt  = pcSet->iterator();

	while (true == pcIt->hasNext()) {
		cxx::lang::String* pcName = pcIt->next();

		socketInfo* pcInfo = m_pcServerSockets->get(pcName);
		if (0 != pcInfo) {
			m_pcPollItems[i].socket  = pcInfo->m_socket;
			m_pcPollItems[i].fd      = 0;
			m_pcPollItems[i].events  = ZMQ_POLLIN;
			m_pcPollItems[i].revents = 0;

			m_pcPollToCallback[i] = m_pcCallbacks->get(pcInfo->m_socket);
			if (0 == m_pcPollToCallback[i]) {
				DEEP_LOG_ERROR(OTHER, "No Callback found for the registered socket!");
				abort();
			}

			i++;

			if (0 != pcInfo->m_monitor) {
				m_pcPollItems[i].socket  = pcInfo->m_monitor;
				m_pcPollItems[i].fd      = 0;
				m_pcPollItems[i].events  = ZMQ_POLLIN;
				m_pcPollItems[i].revents = 0;

				m_pcPollToCallback[i] = &pcInfo->m_cMonitorCb;
			}

			i++;
		}
	}

	delete pcIt;  pcIt  = 0;
	delete pcSet; pcSet = 0;

	pcSet = m_pcOutgoing->keySet();
	pcIt  = pcSet->iterator();

	while (true == pcIt->hasNext()) {
		cxx::lang::String* pcName = pcIt->next();

		socketInfo* pcInfo = m_pcOutgoing->get(pcName);
		if (0 != pcInfo) {
			m_pcPollItems[i].socket  = pcInfo->m_socket;
			m_pcPollItems[i].fd      = 0;
			m_pcPollItems[i].events  = ZMQ_POLLIN;
			m_pcPollItems[i].revents = 0;

			m_pcPollToCallback[i] = m_pcCallbacks->get(pcInfo->m_socket);
			if (0 == m_pcPollToCallback[i]) {
				DEEP_LOG_ERROR(OTHER, "No Callback found for the registered socket!");
				abort();
			}

			i++;

			if (0 != pcInfo->m_monitor) {
				m_pcPollItems[i].socket  = pcInfo->m_monitor;
				m_pcPollItems[i].fd      = 0;
				m_pcPollItems[i].events  = ZMQ_POLLIN;
				m_pcPollItems[i].revents = 0;

				m_pcPollToCallback[i] = &pcInfo->m_cMonitorCb;
			}

			i++;
		}
	}

	delete pcIt;  pcIt  = 0;
	delete pcSet; pcSet = 0;

	/** Now add any Native FDs */
	cxx::util::Set<inttype>*      pcFdSet = m_pcNativeCbs->keySet();
	cxx::util::Iterator<inttype>* pcIt2   = pcFdSet->iterator();

	while (true == pcIt2->hasNext()) {
		inttype iFd = pcIt2->next();

		socketCallback* pcCb = m_pcNativeCbs->get(iFd);
		if (0 != pcCb) {
			m_pcPollItems[i].socket  = 0;
			m_pcPollItems[i].fd      = iFd;
			m_pcPollItems[i].events  = ZMQ_POLLIN;
			m_pcPollItems[i].revents = 0;

			m_pcPollToCallback[i] = pcCb;

			i++;
		}
	}

	delete pcIt2;   pcIt2   = 0;
	delete pcFdSet; pcFdSet = 0;

	return 0;
}

/**
 * addRepSocket
 * Add a listening socket.
 * @param cServer - a string for the address,
 * e.g. "tcp://1.1.1.1:8081"
 * @param pcCallback - callback to invoke for this address
 * @return 0 on success, else none-zero or throws exception.
 * For any internal errors, the LinkManagerException is thrown.
*/
ulongtype cxx::io::LinkManager::addRepSocket(const cxx::lang::String cServer,
					     socketCallback*         pcCallback,
					     voidptr&                sendId) {
	
	socketInfo* pcFind = m_pcServerSockets->get(const_cast<cxx::lang::String* >(&cServer));
	if (0 != pcFind) {
		throw cxx::io::LinkManagerException("Duplicate registration of a server address.");
	}

	voidptr pcZmqSocket = zmq_socket(m_pcContext, ZMQ_REP);
	if (0 == pcZmqSocket) {
		throw cxx::io::LinkManagerException("Unable to create the server socket.");
	}

	int rc = zmq_bind(pcZmqSocket, cServer.c_str());
	if (0 != rc) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to bind the server address to the socket.");
	}

	errno = 0;

	int iLinger = 0;

	rc = zmq_setsockopt(pcZmqSocket,
			    ZMQ_LINGER,
			    &iLinger,
			    sizeof(iLinger));
	if (0 != rc) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to set linger off option.");
	}

	socketInfo* pcInfo = new socketInfo;
	if (0 == pcInfo) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to allocate a sockInfo.");
	}

	pcInfo->m_cMonitorCb.m_pcParent = this;
	pcInfo->m_cMonitorCb.m_pcInfo   = pcInfo;
	pcInfo->m_socket                = pcZmqSocket;

	cxx::lang::String* pcServer = new cxx::lang::String(cServer);
	m_pcServerSockets->put(pcServer, pcInfo);

	m_pcCallbacks->put(pcZmqSocket, pcCallback);
	sendId                          = pcZmqSocket;
	m_iSockets++;

	if (0 != monitorSocketState(pcInfo)) {
		close(pcInfo->m_socket);
		throw cxx::io::LinkManagerException("Unable to add this socket to the poll items.");
	}

	m_pcRevSockMap->put(sendId, pcServer);
	cxx::lang::String* pcCheck = m_pcRevSockMap->get(sendId);
	if ((0 == pcCheck) || (pcCheck != pcServer)) {
		abort();
	}

	return 0;
}

/**
 * addReqSocket
 * Create an outgoing connetion to the specified address.
 * @param cOutgoing - a string for the address,
 * e.g. "tcp://1.1.1.1:8081"
 * @param pcCallback - callback to invoke for this address
 * @return 0 on success, else error.
 * For any errors, the LinkManagerException is thrown.
*/
ulongtype cxx::io::LinkManager::addReqSocket(const cxx::lang::String cOutgoing,
					     socketCallback*         pcCallback,
					     voidptr&                sendId) {
	socketInfo* pcFind = m_pcOutgoing->get(const_cast<cxx::lang::String* >(&cOutgoing));
	if (0 != pcFind) {
		throw cxx::io::LinkManagerException("Duplicate registration of a server address.");
	}

	voidptr pcZmqSocket = zmq_socket(m_pcContext, ZMQ_REQ);
	if (0 == pcZmqSocket) {
		throw cxx::io::LinkManagerException("Unable to create the outgoing socket.");
	}

	errno = 0;

	int rc = zmq_connect(pcZmqSocket, cOutgoing.c_str());
	if (0 != rc) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to start a connection to the outgoing address.");
	}

	errno = 0;

	int iLinger = 0;

	rc = zmq_setsockopt(pcZmqSocket,
			    ZMQ_LINGER,
			    &iLinger,
			    sizeof(iLinger));
	if (0 != rc) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to set linger off option.");
	}

	errno = 0;

	int iOptVal = 1;

	/** Enable TCP KEEPALIVES */
	rc = zmq_setsockopt(pcZmqSocket,
			    ZMQ_TCP_KEEPALIVE,
			    &iOptVal,
			    sizeof(iOptVal));
	if (0 > rc) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to set KEEPALIVEs active.");
	}

	iOptVal = 1; /** TCP_KEEPIDLE 1 second. */

	rc = zmq_setsockopt(pcZmqSocket,
			    ZMQ_TCP_KEEPALIVE_IDLE,
			    &iOptVal,
			    sizeof(iOptVal));
	if (0 > rc) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to set TCP_KEEPIDLE.");
	}

	iOptVal = 1; /** TCP_KEEPINTVL 1 */

	rc = zmq_setsockopt(pcZmqSocket,
			    ZMQ_TCP_KEEPALIVE_INTVL,
			    &iOptVal,
			    sizeof(iOptVal));
	if (0 > rc) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to set TCP_KEEPINTVL.");
	}

	iOptVal = 3; /** TCP_KEEPCNT 3 */
	rc = zmq_setsockopt(pcZmqSocket,
			    ZMQ_TCP_KEEPALIVE_CNT,
			    &iOptVal,
			    sizeof(iOptVal));
	if (0 > rc) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to set TCP_KEEPCNT.");
	}

#if 0
	/** Must contribute a change to support this option. */
	iOptVal = 3000; /** TCP_USER_TIMEOUT 3000 */
	rc = zmq_setsockopt(pcZmqSocket,
			    TCP_USER_TIMEOUT,
			    &iOptVal,
			    sizeof(iOptVal));
	if (0 > rc) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to set TCP_USER_TIMEOUT.");
	}
#endif

	socketInfo* pcInfo = new socketInfo;
	if (0 == pcInfo) {
		zmq_close(pcZmqSocket);
		throw cxx::io::LinkManagerException("Unable to allocate a sockInfo.");
	}

	pcInfo->m_cMonitorCb.m_pcParent = this;
	pcInfo->m_cMonitorCb.m_pcInfo   = pcInfo;
	pcInfo->m_socket                = pcZmqSocket;

	cxx::lang::String* pcOutgoing   = new cxx::lang::String(cOutgoing);
	m_pcOutgoing->put(pcOutgoing, pcInfo);

	m_pcCallbacks->put(pcZmqSocket, pcCallback);
	sendId                          = pcZmqSocket;
	m_iSockets++;

	if (0 != monitorSocketState(pcInfo)) {
		close(pcInfo->m_socket);
		throw cxx::io::LinkManagerException("Unable to add this socket to the poll items.");
	}

	m_pcRevSockMap->put(sendId, pcOutgoing);
	cxx::lang::String* pcCheck = m_pcRevSockMap->get(sendId);
	if ((0 == pcCheck) || (pcCheck != pcOutgoing)) {
		abort();
	}

	return 0;
}

/**
 * addNativeSocket
 * Use this to track native FDs that are not part of ZMQ but we want
 * to poll in the same loop as the ZMQ sockets.
 * @param iFd - the native FD
 * @param a socket callback to handle the events.
 * @return 0 on success, everything else is an error.
 */
ulongtype cxx::io::LinkManager::addNativeSocket(inttype         iFd,
						socketCallback* pcCallback) {
	if (0 != m_pcNativeCbs->get(iFd)) {
		throw cxx::io::LinkManagerException("Duplicate registration of a native FD.");
	}

	m_pcNativeCbs->put(iFd, pcCallback);
	m_iSockets++;

	if (0 != RenderPollItems()) {
		throw cxx::io::LinkManagerException("Failed to add native FD to the poll items.");
	}

	return 0;
}

/**
 * removeNativeSocket
 * Use this to stop tracking native FDs that are not part of ZMQ.
 * @param iFd - the native FD
 * @return 0 on success, everything else is an error.
 */
ulongtype cxx::io::LinkManager::removeNativeSocket(inttype iFd) {
	if (0 == m_pcNativeCbs->get(iFd)) {
		throw cxx::io::LinkManagerException("No found: a registration of a native FD.");
	}

	m_pcNativeCbs->remove(iFd);
	m_iSockets--;

	if (0 != RenderPollItems()) {
		throw cxx::io::LinkManagerException("Failed to remove the native FD from the poll items.");
	}

	return 0;
}

/**
 * close
 * Closes the socket associated with the provided sendId.
 * @param sendId [in/out] - the sendId to close, once closed the callers
 * reference is zeroed.
 * @return -1 if not found, 0 if found and closed.
 */
ulongtype cxx::io::LinkManager::close(voidptr& sendId) {
	cxx::lang::String* pcName = m_pcRevSockMap->get(sendId);
	if (0 == pcName) {
		return 1;
	}

	socketInfo* pcInfo = m_pcServerSockets->get(const_cast<cxx::lang::String* >(pcName));

	if ((0 != pcInfo) && (sendId == pcInfo->m_socket)) {
		m_pcServerSockets->remove(pcName);

	} else {
		pcInfo = m_pcOutgoing->get(pcName);
		if ((0 == pcInfo) || (sendId != pcInfo->m_socket)) {
			/**
			 * Address is not found. No such registered
			 * socket.
			 */
			DEEP_LOG_ERROR(OTHER, "Close cannot find the address to close out (%s).\n",
				       pcName->c_str());
			return 1;
		}

		m_pcOutgoing->remove(pcName);
	}

	zmq_close(pcInfo->m_socket);
	zmq_close(pcInfo->m_monitor);

	m_pcCallbacks->remove(pcInfo->m_socket);

	m_iSockets--;

	if (0 != pcInfo->m_monitor) {
		m_pcCallbacks->remove(pcInfo->m_monitor);
		m_iSockets--;
	}

	RenderPollItems();

	m_pcRevSockMap->remove(pcInfo->m_socket);

	delete pcInfo; pcInfo = 0;

	return 0;
}

/**
 * poll
 * Poll the LinkManager for iTimeout milliseconds. Any sockets that
 * have events to process will have their registered callbacks
 * invoked.
 * @param iTimeout - Maximum time in milliseconds to block on the
 * poll.
 * @return 0 if no issues.
 * Exception thrown if the pollitems are invalid.
 */
ulongtype cxx::io::LinkManager::poll(ulongtype iTimeout) {
	errno = 0;

	if (0 == m_pcPollItems) {
		/**
		 * No pollitems, this can happen
		 * if we have not added any sockets yet
		 * and start polling.
		 */
		usleep(iTimeout * 1000);
		return 0;
	}

	int rc = zmq_poll(m_pcPollItems,
			  m_iSockets,
			  iTimeout);
	if (-1 == rc) {
		if (EFAULT == errno) {
			throw cxx::io::LinkManagerException("poll items is not valid!");
		}
	}
	
	ulongtype iStart = System::currentTimeMillis();

	for (uinttype i=0; i<m_iSockets; i++) {
		if (0 != m_pcPollItems[i].socket) {
			if (m_pcPollItems[i].revents & ZMQ_POLLIN) {
				socketCallback* pcCb   = m_pcPollToCallback[i];
				voidptr         socket = m_pcPollItems[i].socket;

				/** The Callback will own this message */
				zmq_msg_t* pcMessage = new zmq_msg_t;

				zmq_msg_init(pcMessage);
				zmq_msg_recv(pcMessage, socket, ZMQ_DONTWAIT);

				voidptr   pData = zmq_msg_data(pcMessage);
				ulongtype iSize = zmq_msg_size(pcMessage);

				messageBlock* pcResp    = 0;
				voidptr       lmContext = reinterpret_cast<voidptr>(pcMessage);

				pcCb->readEvent(pData, iSize, socket, pcResp, lmContext);

				if (0 != pcResp) {
					/** Reference is owned by the provider. */
					send(socket, pcResp);
				}

				if (0 != m_pcNotifCb) {
					ulongtype iDelta = System::currentTimeMillis() - iStart;
					if (OVERLOAD_ON_RECV_MS <= iDelta) {
						m_pcNotifCb->overloadWarningRecv(iDelta);
						iStart = System::currentTimeMillis();
					}
				}
			}

		} else if (0 != m_pcPollItems[i].fd) {
			socketCallback* pcCb   = m_pcPollToCallback[i];
			if (0 == pcCb) {
				throw cxx::io::LinkManagerException("No socket callback found!");
			}

			pcCb->nativeFdEvent(m_pcPollItems[i].fd,
					    m_pcPollItems[i].revents);

		} else {
			throw cxx::io::LinkManagerException("socket event in an unknown entry!");
		}
	}

	return 0;
}

/**
 * bufferCanBeFreed
 * Called once the send message buffer has been successful
 * sent my ZMQ.
 * @param pData - The data reference
 * @param hint  - is a reference to the LinkManager
 */
void cxx::io::bufferCanBeFreed(voidptr pData, voidptr hint) {
	messageBlock* pcMsg = static_cast<messageBlock*>(hint);
	if (0 == pcMsg) {
		DEEP_LOG_ERROR(OTHER, "Not a valid messageBlock!");
		abort();
	}

	pcMsg->canFree(pData);
}

/**
 * send
 * Send the data buffer provided over the given socket "sendId"
 * @param sendId - the send Id of the connection we wish to use.
 * @param piBuffer - the Buffer contents to send
 * @param iBufferSize - the size of the buffer contents.
 * @return 0 on success, -1 on error.
 */
ulongtype cxx::io::LinkManager::send(voidptr sendId, messageBlock* pcMsg) {
	if ((0 == sendId) || (0 == pcMsg)) {
		return -1;
	}

	zmq_msg_t message;

	ulongtype iStart = System::currentTimeMillis();

	while (false == pcMsg->m_cMessages.isEmpty()) {
		cxx::io::messageBlock::dataPair* pcData =
			pcMsg->m_cMessages.remove(0);
		
		int rc = zmq_msg_init_data(&message,
					   pcData->m_pData,
					   pcData->m_iSize,
					   cxx::io::bufferCanBeFreed,
					   static_cast<voidptr>(pcMsg));
		if (0 != rc) {
			return 1;
		}

		delete pcData; pcData = 0;

		int flags = (pcMsg->m_cMessages.isEmpty() == false) ? ZMQ_SNDMORE : 0;

		while (true) {
			errno = 0;

			int rc = zmq_sendmsg(sendId, &message, flags);

			if ((32 == rc) || ((-1 == rc) && (EAGAIN == errno))) {
				continue;
			}

			if (-1 == rc) {
				if (EFSM == errno) {
					DEEP_LOG_ERROR(OTHER, "EFSM Failure on send.\n");
#if EFSM_CAUSES_CRASH
					abort();
#else
					return 2;
#endif
				}

				if (ETERM == errno) {
					DEEP_LOG_ERROR(OTHER, "Socket terminated before send.\n");
					return 3;
				}

				if (ENOTSUP == errno) {
					DEEP_LOG_ERROR(OTHER, "Not a supported socket type for this send.\n");
					return 3;
				}
			}

			break;
		}

		if (0 != m_pcNotifCb) {
			ulongtype iDelta = System::currentTimeMillis() - iStart;
			if (OVERLOAD_ON_SEND_MS <= iDelta) {
				m_pcNotifCb->overloadWarningSend(iDelta);
				iStart = System::currentTimeMillis();
			}
		}
	}

	pcMsg->m_cMessages.clear();
	return 0;
}

/**
 * getSocketName
 * Given the sockId, return the string name used to create the socket.
 * @param sockId [in] - the socket id
 * @param cName [out] - string to write the result into.
 * @return 0 if found, otherwise -1
 */
ulongtype cxx::io::LinkManager::getSocketName(voidptr sockId, cxx::lang::String& cName) {
	cxx::lang::String* pcName = m_pcRevSockMap->get(sockId);
	if (0 == pcName) {
		return 1;
	}

	cName = *pcName;

	return 0;
}

/**
 * monitorSocketState
 * Sets up the socket monitor name and and creates
 * the client end PAIR to listen for events.
 * @return 0 on success.
 */
ulongtype cxx::io::LinkManager::monitorSocketState(socketInfo* pcInfo) {

	std::stringstream ss;
	ss << "inproc://monitor." << pcInfo->m_socket;

	pcInfo->m_cMonitorSockName = ss.str();

	errno = 0;

	int rc = zmq_socket_monitor(pcInfo->m_socket, pcInfo->m_cMonitorSockName.c_str(), ZMQ_EVENT_ALL);
	if (-1 == rc) {
		return 1;
	}

	errno = 0;

	pcInfo->m_monitor = zmq_socket(m_pcContext, ZMQ_PAIR);
	if (0 == pcInfo->m_monitor) {
		DEEP_LOG_WARN(OTHER, "Unable to create monitor socket.\n");
		return 2;
	}

	errno = 0;

	rc = zmq_connect(pcInfo->m_monitor, pcInfo->m_cMonitorSockName.c_str());
	DEEP_LOG_DEBUG(OTHER, "Connect to socket monitor inproc %s, rc=%d\n",
		       pcInfo->m_cMonitorSockName.c_str(), rc);

	m_pcCallbacks->put(pcInfo->m_monitor, &pcInfo->m_cMonitorCb);
	m_iSockets++;

	if (0 != RenderPollItems()) {
		close(pcInfo->m_monitor);
		return 3;
	}

	m_pcRevSockMap->put(pcInfo->m_monitor, &pcInfo->m_cMonitorSockName);

	return 0;
}

/**
 * socketMonitorEvent
 * Main handler for processing all monitor socket events for the
 * sockets managed by this LinkManager instance.
 * @param iEvent - the Event to dispatch upon
 * @param iValue - the events associated values. This varies
 * from event to event.
 * @param cAddress - the IP Address associated with the socket.
 * @param pcInfo - LinkManager socketInfo for this ZMQ socket.
 */
void cxx::io::LinkManager::socketMonitorEvent(ushorttype        iEvent,
					      uinttype          iValue,
					      cxx::lang::String cAddress,
					      socketInfo*       pcInfo) {
	if (0 == pcInfo) {
		DEEP_LOG_ERROR(OTHER, "No pcInfo found for %s!\n", cAddress.c_str());
		return;
	}

	socketCallback* pcCb = m_pcCallbacks->get(pcInfo->m_socket);
	if (0 == pcCb) {
		DEEP_LOG_ERROR(OTHER, "No Callback found for %s!\n", cAddress.c_str());
		return;
	}

	const char *piEvent = 0;

	switch (iEvent) {
	case ZMQ_EVENT_CONNECTED:
		piEvent = "CONNECTED";
		pcCb->connected(cAddress);
		break;
	case ZMQ_EVENT_CONNECT_DELAYED:
		piEvent = "CONNECT DELAYED";
		break;
	case ZMQ_EVENT_CONNECT_RETRIED:
		piEvent = "CONNECT RETRIED";
		break;
	case ZMQ_EVENT_LISTENING:
		piEvent = "LISTENING";
		break;
	case ZMQ_EVENT_BIND_FAILED:
		piEvent = "BIND FAILED";
		break;
	case ZMQ_EVENT_ACCEPTED:
		piEvent = "ACCEPTED";
		if (0 == getRemotePeerAddress(iValue, cAddress)) {
			/**
			 * We keep a stash of the remote IP address
			 * We do this as on disconnect the FD is
			 * closed and can not be queried for the
			 * remote disconnect event.
			 */
			cxx::lang::String* pcAddress = m_pcFdToIp->get(iValue);
			if (0 != pcAddress) {
				delete pcAddress; pcAddress = 0;
			}

			pcAddress = new cxx::lang::String(cAddress);

			m_pcFdToIp->put(iValue, pcAddress);

			pcCb->accepted(cAddress);
		}
		break;
	case ZMQ_EVENT_ACCEPT_FAILED:
		piEvent = "ACCEPT FAILED";
		break;
	case ZMQ_EVENT_CLOSED:
		piEvent = "CLOSED";
		break;
	case ZMQ_EVENT_CLOSE_FAILED:
		piEvent = "CLOSE FAILED";
		break;
	case ZMQ_EVENT_DISCONNECTED:
		piEvent = "DISCONNECTED";
		{
			cxx::lang::String* pcAddress = m_pcFdToIp->get(iValue);
			if (0 != pcAddress) {
				cAddress = *pcAddress;

				pcCb->disconnected(*pcAddress);

				m_pcFdToIp->remove(iValue);
				delete pcAddress; pcAddress = 0;
			} else {
				pcCb->disconnected(cAddress);
			}
		}
		break;
	case ZMQ_EVENT_MONITOR_STOPPED:
		piEvent = "MONITOR STOPPED";
		break;
	}

	DEEP_LOG_DEBUG(OTHER, "Event: %s (%u) address: %s, value: #%u\n",
		       piEvent,
		       iEvent,
		       cAddress.c_str(),
		       iValue);
}

/**
 * getRemotePeerAddress
 * Gets the IP address of the remove connecting peer.
 * This is only useful on ACCEPTing sockets.
 * @param iSocketFd [in] - the kernel FD
 * @param cAddress [out] - location to write out the address.
 * @return 0 on success.
 */
int cxx::io::LinkManager::getRemotePeerAddress(uinttype           iSocketFd,
					       cxx::lang::String& cAddress) {
	struct sockaddr_in peerAddr;
	socklen_t          len = sizeof(struct sockaddr);

	errno = 0;

	int rc = getpeername(iSocketFd,
			     (struct sockaddr*)&peerAddr,
			     &len);
	if (0 == rc) {
		std::stringstream ss;

		ss << "tcp://"
		   << inet_ntoa(peerAddr.sin_addr)
		   << ":"
		   << ntohs(peerAddr.sin_port);

		cAddress = ss.str();
		return 0;
	}

	return -1;
}

/**
 * registerNotifications
 * Register a notifications callback to get LinkManager events.
 * @param pcNotifCb - the callback reference to register.
 * @retutrn 0 on success.
 */
ulongtype cxx::io::LinkManager::registerNotifications(lmNotifications* pcNotifCb) {
	m_pcNotifCb = pcNotifCb;
	return 0;
}

/** socketCallback */

/**
 * socketCallback::accepted
 * Default behavor for the accepted event for a REP socket.
 * @param cAddress - the address of the remote.
 * This will be an ephemeral address typically.
 */
void cxx::io::socketCallback::accepted(cxx::lang::String cAddress) {
	DEEP_LOG_INFO(OTHER, "Accepted connection from %s.\n", cAddress.c_str());
}

/**
 * socketCallback::connected
 * Default behavior for the connected event for a REQ socket.
 * @param cAddress - the address of the remote REP socket.
 */
void cxx::io::socketCallback::connected(cxx::lang::String cAddress) {
	DEEP_LOG_INFO(OTHER, "Connected to %s.\n", cAddress.c_str());
}

/**
 * socketCallback::disconnected
 * Default behavior for the disconnected event for a REQ socket.
 * @param cAddress - the address of the remote REP socket.
 */
void cxx::io::socketCallback::disconnected(cxx::lang::String cAddress) {
	DEEP_LOG_INFO(OTHER, "Disconnected from %s.\n", cAddress.c_str());
}

/**
 * nativeFdEvent
 * Called when a native FD has an event occur.
 * @param iFd - the native linux FD
 * @param iEvent - the event flags
 */
void cxx::io::socketCallback::nativeFdEvent(inttype iFd, shorttype iEvents) {
	DEEP_LOG_INFO(OTHER, "nativeFdEvent(iFd=%d, iEvents=%d)\n", iFd, iEvents);
}

/** monitorCb */

/**
 * monitorCb::readEvent
 * Process the two event messages for the next socket event.
 * The first message is a 16bit value for the event + a 32bit
 * value associated with the event. This value might be a socket fd,
 * or errno value for example. The next message contains the
 * IP Address string for this event.
 * @param pData - raw data for the first message.
 * @param iSize - raw data size.
 * @param sockId - ZMQ socket ID
 * @param pcResp - NO USED.
 * @param lmContext - buffer handle
 */
void cxx::io::LinkManager::monitorCb::readEvent(voidptr        pData,
						ulongtype      iSize,
						voidptr        sockId,
						messageBlock*& pcResp,
						voidptr        lmContext) {
	ubytetype*  piData  = reinterpret_cast<ubytetype*>(pData);
	ushorttype* piEvent = reinterpret_cast<ushorttype*>(piData);
	uinttype*   piValue = reinterpret_cast<uinttype*>(piData+2);

	zmq_msg_t cMsg;
	zmq_msg_init(&cMsg);

	/**
	 * Now fetch the next message in the sequence, This
	 * must be the address string.
	 */
	if (zmq_msg_recv (&cMsg, sockId, 0) == -1) {
		/** No further events which is wrong. */
		DEEP_LOG_ERROR(OTHER, "monitorCb::readEvent, missing address for event #%u, value #%u\n",
			      *piEvent,
			      *piValue);
		return;
	}

	char*     piAddress = reinterpret_cast<char *>(zmq_msg_data (&cMsg));
	ulongtype iSize2    = zmq_msg_size(&cMsg);

	cxx::lang::String cAddress(piAddress, iSize2);

	/** Now call the LinkManager dispatcher for this event. */
	m_pcParent->socketMonitorEvent(*piEvent, *piValue, cAddress, m_pcInfo);

	cxx::io::closeLmContext(lmContext);
}
