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
#ifndef COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_CASSISHIM_H_
#define COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_CASSISHIM_H_

#include <sys/epoll.h>

#include "cxx/io/EncodeProtocol.h"
#include "cxx/lang/Runnable.h"
#include "cxx/io/MsgQueue.h"
#include "cxx/lang/Thread.h"
#include "cxx/io/SocketCommon.h"
#include "cxx/util/HashMap.h"
#include "cxx/util/ArrayList.h"
#include "cxx/util/UUID.h"

#include "com/deepis/db/store/relative/distributed/IMessageService.h"
#include "com/deepis/communication/common/FabricEnvelope.h"
#include "com/deepis/communication/common/MsgIds.h"
#include "com/deepis/communication/fabricconnector/FabricCassiJSONAPI.h"
#include "com/deepis/communication/fabricconnector/CassiDatabase.h"

using namespace com::deepis::communication::common;
using namespace com::deepis::db::store::relative::distributed;

namespace com { namespace deepis { namespace communication { namespace fabricconnector {

class FabricCassiMessageQueueException : public Exception {
public:
	FabricCassiMessageQueueException(void) {
	}

	FabricCassiMessageQueueException(const char* message)
		: Exception(message) {
	}
}; /** FabricCassiMessageQueueException */

/**
 * FabricCassiMessageQueue to RealtimeFabric commands and arguments.
 */
static const uinttype REGISTER_MSG_ID_CMD = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 1);

static const uinttype ADD_PEER_MSG_ID_CMD = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 2);
struct peerCmdArgs {
	~peerCmdArgs() {
		delete m_pcUUID; m_pcUUID = 0;
		m_pCtx = 0;
	}

	cxx::lang::String m_cAddress;
	cxx::util::UUID*  m_pcUUID;
	voidptr           m_pCtx;
};

static const uinttype DEL_PEER_MSG_ID_CMD = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 3);

static const uinttype GET_TOPO_MSG_ID_CMD = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 6);

static const uinttype BUNDLED_MSG_ID_CMD  = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 8);

static const uinttype LAST_MESSAGE_ID_CMD = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 10);

/*** End Commands ***/

class FabricCassiBridgeCb;
class JsonApi;
class JsonApiRequest;
class ApiCallback;
class FabricCassiAPIConsumer;

class FabricCassiMessageQueue : public cxx::lang::Runnable, public cxx::lang::Object {
public:
	FabricCassiMessageQueue(uinttype iServerId, void* pcLoaderList, ulongtype iMemoryCap);
	virtual ~FabricCassiMessageQueue();

	const uinttype getServerId(void) { return m_iServerId; }

	virtual void run(void);

	/** Bridge code registration */
	inttype registerBridge(FabricCassiBridgeCb* pcCb);
	inttype unregisterAllBridges(void);
	inttype registerTableMessageService(IMessageService* messageService);

	/** FABRIC Thread methods */
	inttype enqueue(cxx::io::encodeProtocol::reader* pcMsg);
	inttype dequeue(FabricEnvelope*& pcMsg, boolean& bMore);

	/**
	 * fabricHasWork
	 * Returns true if the fabric has queued items.
	 * @return Returns true if the fabric has queued items.
	 */
	boolean fabricHasWork(ulongtype iTimeout=0) {
		if (0 == iTimeout) {
			return ((false == m_cCassiToFabric.isEmpty()) ? true : false);
		}

		boolean bHasWork = false;
		m_cCassiToFabric.poller(iTimeout, bHasWork);

		return bHasWork;
	}

	inttype cassiToFabricEventFd(void) {
		return m_cCassiToFabric.eventFd();
	}

	/** CASSI/ENGINE Thread methods */
	inttype dequeue(cxx::io::encodeProtocol::reader*& pcMsg, boolean& bMore);
	inttype enqueue(FabricEnvelope* pcMsg);

	inttype registerMessageId(uinttype iMsgId);

	inttype sendPendingApiReply(JsonApiRequest* pcReq);

	ulongtype memoryCapacity(void) { return m_iMemoryCap; }

	/**
	 * cassiHasWork
	 * Returns true if the engine has queued items.
	 * @return Returns true if the engine has queued items.
	 */
	FORCE_INLINE boolean cassiHasWork() {
		return ((false == m_cFabricToCassi.isEmpty()) ? true : false);
	}

	inttype enableCassiPoller(void);
	inttype cassiPoller(ulongtype iTimeout, boolean& bHasWork);

	inttype signalExitCassi(void);

	struct SHIM_REP {
		static const uinttype MSG_ID          = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 1);

		static const inttype  REPLY_OK         =  1;
		static const inttype  REPLY_QUEUE_FULL = -1; /** In effect,
								 messages dropped at receiver. */

		/** Fields */
		static const uinttype SERVER_ID_FIELD  =  0; /** UINT32 */
		static const uinttype REPLY_ID_FIELD   =  1; /** INT32  */
	}; /** SHIM_REP */

	struct PEER_CHANGE {
		enum State {
			DOWN = 0,
			UP   = 1,
		};

		static const uinttype MSG_ID          = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 3);

		static const inttype  SERVER_ID_FIELD = 0; /** UINT32 */
		static const inttype  STATE_FIELD     = 1; /** INT32 */
	};

	struct ADD_PEER_REP {
		static const uinttype MSG_ID          = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 4);
		static const inttype  STATUS_FIELD    = 0; /** INT32   */
		static const inttype  CONTEXT_FIELD   = 1; /** VOIDPTR */
	};

	struct DEL_PEER_REP {
		static const uinttype MSG_ID          = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 5);
		static const inttype  STATUS_FIELD    = 0; /** INT32   */
		static const inttype  CONTEXT_FIELD   = 1; /** VOIDPTR */
	};

	struct GET_TOPO_REP {
		static const uinttype MSG_ID          = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 7);
		static const inttype  CONTEXT_FIELD   = 0; /** VOIDPTR */
	};

	struct BUNDLE_REQ {
		static const uinttype MSG_ID          = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 9);
		static const inttype  CONTEXT_FIELD   = 0; /** VOIDPTR */
	};

	inttype signalPeerChange(uinttype                                    iServerId,
				 FabricCassiMessageQueue::PEER_CHANGE::State eState);

	inttype registerApiCallback(cxx::lang::String cPath,
				    ApiCallback* pcApiHandler);
	
	inttype unregisterApiCallback(cxx::lang::String cPath);

	inttype addPeerCmd(JsonApiRequest*    pcReq,
			   cxx::lang::String& cPath,
			   cxx::lang::String& cAddress,
			   cxx::util::UUID&   cUUID);

	inttype delPeerCmd(JsonApiRequest*  pcReq,
			   cxx::util::UUID& cUUID);

	inttype getTopo(JsonApiRequest*  pcReq);

	inttype processCassiCommand(JsonApiRequest* pcReq);

	inttype sendBundleQ(uinttype iPeerId, uinttype iMsgId, voidptr pBundle);

	inttype signalAddPeerRep(inttype iStatus, voidptr pCtx);
	inttype signalDelPeerRep(inttype iStatus, voidptr pCtx);
	inttype signalGetTopoRep(voidptr pCtx);
	inttype signalBundleReq(voidptr pBundleArray);

	inttype processFabricLocalBridge(cxx::io::encodeProtocol::reader* pcMsg);
	inttype processFabricLocal(cxx::io::encodeProtocol::reader* pcMsg);

	inttype lastMessage(void);

	struct BundleList {
		static const inttype BUNDLE_ARRAY_INIT_CAP = 2048;

		BundleList() : m_cBundle(BUNDLE_ARRAY_INIT_CAP, true),
			       m_iTotalByteSize(0) {}

		cxx::util::ArrayList<FabricEnvelope*> m_cBundle;
		ulongtype                             m_iTotalByteSize;
	};

private:

	

	inttype LoadBridges(void);
	inttype processBundleFromFabric(cxx::io::encodeProtocol::reader* pcMsg);

	static const uinttype CASSI_TO_FABRIC_Q_SIZE = 1024 << 4;
	static const uinttype FABRIC_TO_CASSI_Q_SIZE = 1024 << 4;

	struct EXIT_REQ {
		static const uinttype MSG_ID          = MSG_ID(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, 2);
	};

	cxx::io::MsgQueue  m_cCassiToFabric;
	cxx::io::MsgQueue  m_cFabricToCassi;

	inttype            m_iCassiEpollFd;
	struct epoll_event m_stCassiEvents;

	cxx::util::ArrayList<FabricCassiBridgeCb* >         m_cBridgeCbs;
	cxx::util::HashMap<uinttype, FabricCassiBridgeCb* > m_cMsgToBridgeCbs;
	void*                                               m_pcLoaderList;
	uinttype                                            m_iServerId;

	JsonApi*                                            m_pcJsonApi;
	cxx::lang::Thread*                                  m_pcJsonApiThread;
	cxx::lang::Thread*				    m_pcFabricCassiAPIConsumerThread;
	boolean 					    m_bStopFabricCassiAPIConsumerThread;

	cxx::util::HashMap<cxx::lang::String*,
			   com::deepis::communication::fabricconnector::ApiCallback* >* m_pcApiHandlers;
	ApiCallback*                                        m_pcFabricApiHandler;
	ApiCallback*                                        m_pcDelPeerApiHandler;
	ApiCallback*                                        m_pcCassiCmdApiHandler;
	ApiCallback*                                        m_pcTopoApiHandler;

	ulongtype                                           m_iMemoryCap;

	com::deepis::communication::fabricconnector::DistributedCassiDatabase* m_pcDcDb;

	friend class FabricCassiAPIConsumer;

}; /** FabricCassiMessageQueue */

class FabricCassiAPIConsumer : public cxx::lang::Runnable {
public:
	FabricCassiAPIConsumer(FabricCassiMessageQueue* pcParent);

	virtual void run(void);

private:
	FabricCassiMessageQueue* m_pcParent;

}; /** FabricCassiAPIConsumer */

class FabricCassiBridgeCb : public cxx::lang::Object {
public:
	FabricCassiBridgeCb(FabricCassiMessageQueue* pcCommon);
	virtual ~FabricCassiBridgeCb();

	virtual void remoteOverloaded(cxx::io::encodeProtocol::reader* pcMsg) {
	}

	virtual void shutdown() {
	}

	virtual void peerStateChanged(cxx::io::encodeProtocol::reader* pcMsg) {
	}

	virtual inttype dispatch(cxx::io::encodeProtocol::reader* pcMsg) {
		return 0;
	}

	virtual const uinttype* allMessageIds(void) {
		return 0;
	}

	virtual void setMessageService(IMessageService* messageService) {
		return;
	}

	inttype send(FabricEnvelope* pcMsg);

	FabricCassiMessageQueue* m_pcCommon;

private:
	
	FabricCassiMessageQueue::BundleList* m_pcBundle;

}; /** FabricCassiBridgeCb */

typedef FabricCassiBridgeCb* (*loaderFunc)(FabricCassiMessageQueue* pcCommon, voidptr pCtx);

struct LoadListEntry {
	loaderFunc m_pfLoader;
	voidptr    m_pCtx;

}; /** LoadListEntry  */


} } } } // namespace

#endif /** COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_CASSISHIM_H_ */
