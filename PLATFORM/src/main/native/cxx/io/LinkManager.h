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
#ifndef CXX_LANG_IO_LINKMANAGER_H_
#define CXX_LANG_IO_LINKMANAGER_H_

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <memory>
#include <functional>

#include <zmq.h>
#include <zmq_utils.h>

#include "cxx/io/EOFException.h"
#include "cxx/lang/String.h"
#include "cxx/util/HashMap.h"
#include "cxx/lang/Object.h"
#include "cxx/util/ArrayList.h"
#include "cxx/util/concurrent/atomic/AtomicInteger.h"

namespace cxx { namespace io {

class LinkManager;

/**
 * Exceptions to handle errors beyond the LinkManager API
 */
class LinkManagerException : public Exception {
public:
	LinkManagerException(void) {
	}

	LinkManagerException(const char* message)
		: Exception(message) {
	}
}; // LinkManagerException

/**
 * Used to send a message block
 */
class messageBlock : public cxx::lang::Object {
public:
	messageBlock() {
	}

	virtual ~messageBlock() {
	}

	virtual void canFree(voidptr pData) {
	}

	void addMessage(cxx::lang::String cMsg);
	void addMessage(voidptr pData, ulongtype iSize);

private:
	struct dataPair : public cxx::lang::Object {
		dataPair(voidptr   pData,
			 ulongtype iSize);
		~dataPair();

		voidptr   m_pData;
		ulongtype m_iSize;
	}; /** dataPair */

	friend class LinkManager;
	cxx::util::ArrayList<dataPair* > m_cMessages;

}; /** messageBlock */

/**
 * Used to register per socket read event handling.
 */
struct socketCallback : public cxx::lang::Object {
	virtual void readEvent(voidptr        pData,
			       ulongtype      iSize,
			       voidptr        sockId,
			       messageBlock*& pcResp,
			       voidptr        lmContext = 0) = 0;

	virtual void accepted(cxx::lang::String cAddress);
	virtual void connected(cxx::lang::String cAddress);
	virtual void disconnected(cxx::lang::String cAddress);
	virtual void nativeFdEvent(inttype iFd, shorttype iEvents);

}; // socketCallback

struct lmNotifications : public cxx::lang::Object {
	virtual void overloadWarningRecv(ulongtype iTimeLimit) {}
	virtual void overloadWarningSend(ulongtype iTimeLimit) {}
}; // lmNotifications

typedef socketCallback* socketCallbackPtr;

/**
 * The LinkManager that handles to server (listening sockets)
 * and any outgoing connecting sockets to other listeners.
 * Allows for unique address outgoing connections and unique
 * listening sockets per instance of a linkManager.
 */
class LinkManager : public cxx::lang::Object {
private:
	// Methods and Types
	ulongtype RenderPollItems();
	void     bufferCanBeFreed();

	friend void cxx::io::bufferCanBeFreed(voidptr pData, voidptr hint);

	struct socketInfo;

	struct monitorCb : public socketCallback {
		monitorCb();
		monitorCb(LinkManager* pcParent);
		virtual ~monitorCb();

		virtual void readEvent(voidptr        pData,
				       ulongtype      iSize,
				       voidptr        sockId,
				       messageBlock*& pcResp,
				       voidptr        lmContext);

		LinkManager* m_pcParent;
		socketInfo*  m_pcInfo;

	}; /** monitorCb */

	friend class monitorCb;

	struct socketInfo : public cxx::lang::Object {
		socketInfo();
		~socketInfo();

		voidptr           m_socket;
		voidptr           m_monitor;
		cxx::lang::String m_cMonitorSockName;
		monitorCb         m_cMonitorCb;
	};

	ulongtype monitorSocketState(socketInfo* pcInfo);

	void socketMonitorEvent(ushorttype        iEvent,
				uinttype          iValue,
				cxx::lang::String cAddress,
				socketInfo*       pcInfo);

	int getRemotePeerAddress(uinttype           iSocketFd,
				 cxx::lang::String& cAddress);

	// Member Variables
	uinttype                                              m_iSockets;
	uinttype                                              m_iSocketsGrowth;
	cxx::util::HashMap<cxx::lang::String*, socketInfo* >* m_pcServerSockets;
	cxx::util::HashMap<cxx::lang::String*, socketInfo* >* m_pcOutgoing;
	voidptr                                               m_pcContext;
	zmq_pollitem_t*                                       m_pcPollItems;
	socketCallbackPtr*                                    m_pcPollToCallback;
	cxx::util::HashMap<voidptr, socketCallback* >*        m_pcCallbacks;
	cxx::util::HashMap<voidptr, cxx::lang::String* >*     m_pcRevSockMap;
	cxx::util::HashMap<uinttype, cxx::lang::String* >*    m_pcFdToIp;
	lmNotifications*                                      m_pcNotifCb;
	cxx::util::HashMap<inttype, socketCallback* >*        m_pcNativeCbs;

public:
	LinkManager();
	virtual ~LinkManager();

	ulongtype addRepSocket(const cxx::lang::String cServer,
			       socketCallback*         pcCallback,
			       voidptr&                sendId);

	ulongtype addReqSocket(const cxx::lang::String cOutgoing,
			       socketCallback*         pcCallback,
			       voidptr&                sendId);

	ulongtype addNativeSocket(inttype         iFd,
				  socketCallback* pcCallback);

	ulongtype removeNativeSocket(inttype iFd);

	ulongtype close(voidptr& sendId);

	ulongtype poll(ulongtype iTimeout);

	ulongtype send(voidptr sendId, messageBlock* pcMsg);

	ulongtype getSocketName(voidptr sockId, cxx::lang::String& cName);

	ulongtype stopService(void);

	ulongtype registerNotifications(lmNotifications* pcNotifCb);

}; // LinkManager

void bufferCanBeFreed(voidptr pData, voidptr hint);
inttype closeLmContext(voidptr& lmContext);

} // io
} // cxx

/** monitorCb */
FORCE_INLINE cxx::io::LinkManager::monitorCb::monitorCb() :
	m_pcParent(0),
	m_pcInfo(0)
{
}

FORCE_INLINE cxx::io::LinkManager::monitorCb::~monitorCb() {
	m_pcParent = 0;
	m_pcInfo   = 0;
}

/** socketInfo */
FORCE_INLINE cxx::io::LinkManager::socketInfo::socketInfo() : m_socket(0),
							      m_monitor(0) {
}

FORCE_INLINE cxx::io::LinkManager::socketInfo::~socketInfo() {
	m_socket  = 0;
	m_monitor = 0;
	m_cMonitorSockName.clear();
}

/** messageBlock */
FORCE_INLINE void cxx::io::messageBlock::addMessage(cxx::lang::String cMsg) {
	addMessage((voidptr)cMsg.c_str(), cMsg.size());
}

FORCE_INLINE void cxx::io::messageBlock::addMessage(voidptr pData, ulongtype iSize) {
	dataPair* pcData = new dataPair(pData, iSize);
	m_cMessages.add(pcData);
}

/** messageBlock::dataPair */
FORCE_INLINE cxx::io::messageBlock::
dataPair::dataPair(voidptr   pData,
		   ulongtype iSize) : m_pData(pData),
				      m_iSize(iSize) {
}

FORCE_INLINE cxx::io::messageBlock::
dataPair::~dataPair() {
	m_pData = 0;
	m_iSize = 0;
}

/**
 * closeLmContext
 * Must be used to close the LinkManager context for the associated
 * message buffer. Otherwise the receiver will leak the message
 * buffer. Calling this signals that the owner has no further use for
 * the associated message.
 * lmContext - the Link Manager Context passed into the readEvent
 * method of the socketCallback object.
 */
FORCE_INLINE inttype cxx::io::
closeLmContext(voidptr& lmContext) {
	zmq_msg_t* pcMessage = reinterpret_cast<zmq_msg_t* >(lmContext);
	if (0 == pcMessage) {
		return -1;
	}

	zmq_msg_close(pcMessage);
	delete pcMessage; pcMessage = 0; lmContext = 0;

	return 0;
}

#endif /** CXX_LANG_IO_LINKMANAGER_H_ */
