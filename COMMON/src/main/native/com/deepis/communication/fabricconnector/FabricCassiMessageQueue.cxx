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
#include "FabricCassiMessageQueue.h"
#include "cxx/util/Logger.h"
#include "com/deepis/communication/fabricconnector/CassiServiceBridge.h"
#include "cxx/util/HashMap.cxx"
#include <sstream>

using namespace cxx::lang;
using namespace cxx::util;
using namespace com::deepis::communication::fabricconnector;
using namespace com::deepis::communication::common;

#include "com/deepis/communication/fabricconnector/fabricApiHandlers.h"



/**
 * FabricCassiMessageQueue
 * Constructor. Create the FabCassi instance and launches the API thread.
 * @param iServerId - instance ID for this Fabric.
 * @param pcLoaderList - list of bridge code launch methods and
 * context parameters to pass in to each.
 */
FabricCassiMessageQueue::FabricCassiMessageQueue(uinttype  iServerId,
						 void*     pcLoaderList,
						 ulongtype iMemoryCap) :
	m_cCassiToFabric(CASSI_TO_FABRIC_Q_SIZE, true),
	m_cFabricToCassi(FABRIC_TO_CASSI_Q_SIZE, true),
	m_iCassiEpollFd(-1),
	m_pcLoaderList(pcLoaderList),
	m_iServerId(iServerId),
	m_pcJsonApi(0),
	m_pcJsonApiThread(0),
	m_pcFabricCassiAPIConsumerThread(0),
	m_bStopFabricCassiAPIConsumerThread(false),
	m_pcApiHandlers(0),
	m_pcFabricApiHandler(0),
	m_pcDelPeerApiHandler(0),
	m_pcCassiCmdApiHandler(0),
	m_pcTopoApiHandler(0),
	m_iMemoryCap(iMemoryCap),
	m_pcDcDb(0) {

	m_pcJsonApi = new JsonApi(iServerId, (ushorttype)(8080+iServerId));
	if (0 == m_pcJsonApi) {
		throw FabricCassiMessageQueueException("Unable to create the JSON API.");
	}

	m_pcApiHandlers =
		new cxx::util::HashMap<cxx::lang::String*,
				       com::deepis::communication::fabricconnector::ApiCallback* >(16, true, false, false);
	if (0 == m_pcApiHandlers) {
		throw FabricCassiMessageQueueException("Unable to create the API handler map.");
	}

	m_pcFabricApiHandler = new com::deepis::communication::fabricconnector::FabricApiHandler(this);
	if (0 == m_pcFabricApiHandler) {
		throw FabricCassiMessageQueueException("Unable to create the Fabric API handler.");
	}

	m_pcDelPeerApiHandler = new DelPeerApiHandler(this);
	if (0 == m_pcDelPeerApiHandler) {
		throw FabricCassiMessageQueueException("Unable to create the Fabric delPeer API handler.");
	}

	m_pcDcDb = new com::deepis::communication::fabricconnector::DistributedCassiDatabase(this);
	if (0 == m_pcDcDb) {
		throw FabricCassiMessageQueueException("Unable to create the Distrivuted Cassi Database handler");
	}

	m_pcTopoApiHandler = new FabricTopoApiHandler(this);
	if (0 == m_pcTopoApiHandler) {
		throw FabricCassiMessageQueueException("Unable to create the Fabric Topology API handler.");
	}

	m_pcCassiCmdApiHandler = new CassiCommandHandler(this);
	if (0 == m_pcCassiCmdApiHandler) {
		throw FabricCassiMessageQueueException("Unable to create the generic cassi command handler.");
	}

	inttype iStatus = registerApiCallback("/fabric/peer", m_pcFabricApiHandler);
	if (0 != iStatus) {
		throw FabricCassiMessageQueueException("Unable to register the Fabric API handler.");
	}

	iStatus = registerApiCallback("/fabric/topology", m_pcTopoApiHandler);
	if (0 != iStatus) {
		throw FabricCassiMessageQueueException("Unable to register the Fabric Topology API handler.");
	}

	iStatus = registerApiCallback("/cassi/commands", m_pcCassiCmdApiHandler);
	if (0 != iStatus) {
		throw FabricCassiMessageQueueException("Unable to register the generic cassi command handler.");
	}

	m_pcJsonApiThread = new cxx::lang::Thread(m_pcJsonApi);
	if (0 == m_pcJsonApiThread) {
		throw FabricCassiMessageQueueException("Unable to create the JSON API Thread.");
	}

	
	m_pcJsonApiThread->start();
}

/**
 * ~FabricCassiMessageQueue
 * Destructor.
 */
FabricCassiMessageQueue::~FabricCassiMessageQueue() {
	if (-1 != m_iCassiEpollFd) {
		close(m_iCassiEpollFd);
	}

	m_pcJsonApi->stopService();
	m_pcJsonApiThread->join();

	delete m_pcApiHandlers;        m_pcApiHandlers = 0;
	delete m_pcJsonApiThread;      m_pcJsonApiThread = 0;
	delete m_pcJsonApi;            m_pcJsonApi = 0;
	delete m_pcFabricApiHandler;   m_pcFabricApiHandler = 0;
	delete m_pcDelPeerApiHandler;  m_pcDelPeerApiHandler = 0;
	delete m_pcTopoApiHandler;     m_pcTopoApiHandler = 0;
	delete m_pcCassiCmdApiHandler; m_pcCassiCmdApiHandler = 0;

	m_pcLoaderList = 0;
	m_iServerId    = 0;

	if (0 != m_pcDcDb) {
		delete m_pcDcDb; m_pcDcDb = 0;
	}
}

/**
 * registerBridge
 * Bridge code registration.
 * @param pcCb - bridge callback to handle fabric to cassi thread
 * messages.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::registerBridge(FabricCassiBridgeCb* pcCb) {
	m_cBridgeCbs.add(pcCb);

	const uinttype* pcIds = pcCb->allMessageIds();
	if (0 != pcIds) {
		for (uinttype i=0; 0 != pcIds[i]; i++) {
			registerMessageId(pcIds[i]);

			if (0 == m_cMsgToBridgeCbs.get(pcIds[i])) {
				/** One registration per bridge */
				m_cMsgToBridgeCbs.put(pcIds[i], pcCb);
			} else {
				DEEP_LOG_ERROR(OTHER, "One Message ID registration possible for ID: %u\n",
					       pcIds[i]);
				abort();
			}
		}
	}

	return 0;
}

/**
 * unregisterAllBridges
 * Called when the Cassi thread exits. This unloads and deleted any
 * bridges creaed in LoadBridges.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::unregisterAllBridges(void) {
	cxx::util::Iterator<FabricCassiBridgeCb* >* pcIt = m_cBridgeCbs.iterator();

	while (true == pcIt->hasNext()) {
		FabricCassiBridgeCb* pcCb = pcIt->next();
		if (0 == pcCb) {
			continue;
		}

		pcIt->remove();
		delete pcCb; pcCb = 0;
	}

	delete pcIt; pcIt = 0;

	cxx::util::Set<uinttype>*      pcSet2 = m_cMsgToBridgeCbs.keySet();
	cxx::util::Iterator<uinttype>* pcIt2  = pcSet2->iterator();
	while (true == pcIt2->hasNext()) {
		pcIt2->next();
		pcIt2->remove();
	}

	delete pcIt2;  pcIt2  = 0;
	delete pcSet2; pcSet2 = 0;

	return 0;
}

/**
 * registerTableMessageService
 * called when a new table is created, registers the table's message service with bridges
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::registerTableMessageService(IMessageService* messageService) {
	cxx::util::Iterator<FabricCassiBridgeCb* >* pcIt = m_cBridgeCbs.iterator();

	while (true == pcIt->hasNext()) {
		FabricCassiBridgeCb* pcCb = pcIt->next();
		if (0 == pcCb) {
			continue;
		}

		pcCb->setMessageService(messageService);
	}

	delete pcIt; pcIt = 0;

	return 0;
}

/**
 * dequeue
 * For CASSI to dequeue messages. bMore indicates if the queue
 * contains more, messages.
 * @param pcMsg [out] - return reference to a reader object.
 * @param bMore [out] - returns true if more messages in the queue.
 * @return 0 on success, -1 if empty, anything else is an error.
 */
inttype FabricCassiMessageQueue::dequeue(cxx::io::encodeProtocol::reader*& pcMsg, boolean& bMore) {
	if (true == m_cFabricToCassi.isEmpty()) {
		bMore = false;
		return -1;
	}

	voidptr pMsg = 0;

	if (0 != m_cFabricToCassi.dequeue(pMsg)) {
		return -2;
	}

	bMore = (true == m_cFabricToCassi.isEmpty()) ? false : true;

	pcMsg = static_cast<cxx::io::encodeProtocol::reader* >(pMsg);
	if (0 == pcMsg) {
		return -3;
	}

	return 0;
}

/**
 * enqueue
 * For FABRIC to enqueue messages to CASSI.
 * @param pcMsg - The reader object to enqueue.
 * @return 0 on success, -1 if the queue is full.
 */
inttype FabricCassiMessageQueue::enqueue(cxx::io::encodeProtocol::reader* pcMsg) {
	if (true == m_cFabricToCassi.isFull()) {
		return -1;
	}

	if (0 != m_cFabricToCassi.enqueue(pcMsg)) {
		return -2;
	}

	return 0;
}

/**
 * dequeue
 * FABRIC Dequeues a writer object from CASSI. Writer will contain
 * envelope instructions on how to forward this message.
 * @param pcMsg [out] - a writer message to forward to another fabric.
 * @param bMore [out] - returns true if more messages exist.
 * @return 0 on success, -1 if empty, anything else is an error.
 */
inttype FabricCassiMessageQueue::dequeue(FabricEnvelope*& pcMsg, boolean& bMore) {
	if (true == m_cCassiToFabric.isEmpty()) {
		bMore = false;
		return -1;
	}

	voidptr pMsg = 0;

	if (0 != m_cCassiToFabric.dequeue(pMsg)) {
		return -2;
	}

	bMore = (true == m_cCassiToFabric.isEmpty()) ? false : true;

	pcMsg = static_cast<FabricEnvelope* >(pMsg);
	if (0 == pcMsg) {
		return -3;
	}

	return 0;
}

/**
 * enqueue
 * For CASSI to enqueue writer messages to the FABRIC for forwarding.
 * @param pcMsg [in] - The writer message to send.
 * @return 0 on success, -1 if the queue is full.
 */
inttype FabricCassiMessageQueue::enqueue(FabricEnvelope* pcMsg) {
	if (true == m_cCassiToFabric.isFull()) {
		return -1;
	}

	if (0 != m_cCassiToFabric.enqueue(pcMsg)) {
		return -2;
	}
	return 0;
}

/**
 * enableCassiPoller
 * Enable epolling of the Cassi Queue.
 * @return 0 on success
 */
inttype FabricCassiMessageQueue::enableCassiPoller(void) {
	inttype iFd = m_cFabricToCassi.eventFd();
	errno = 0;

	m_iCassiEpollFd = epoll_create(cxx::io::MAXCONN);
	if (-1 == m_iCassiEpollFd) {
		DEEP_LOG_ERROR(OTHER, "Unable to create an epoll FD\n");
		return -1;
	}

	errno = 0;

	inttype iStatus = cxx::io::modifyEpollContext(m_iCassiEpollFd, EPOLL_CTL_ADD, iFd, EPOLLIN, 0);
	if (0 != iStatus) {
		DEEP_LOG_ERROR(OTHER, "Unable to add CASSI poller FD to the epoll fd.\n");
		return -2;
	}

	return 0;
}

/**
 * cassiPoller
 * Poll for work items from the Fabric. If enableCassiPoller was
 * called then we efficiently wait for events to happen so we can
 * check. Otherwise we run in a hot mode.
 * @param iTimeout - milliseconds to timeout waiting for an event.
 * @param bHasWork [out] - true or false if there are items on the
 * queue.
 * @return 0 on success, errors mean bad things happened.
 */
inttype FabricCassiMessageQueue::cassiPoller(ulongtype iTimeout, boolean& bHasWork) {
	if (-1 == m_iCassiEpollFd) {
		/** Run hot */
		bHasWork = cassiHasWork();
		return 0;
	}

	errno = 0;

	inttype iEventCount = epoll_wait(m_iCassiEpollFd, &m_stCassiEvents, 1, iTimeout);
	if (-1 == iEventCount) {
		if (EINTR != errno) {
			return -1;
		}
	}

	/** Whether we have any events or not, check the Queue. */
	bHasWork = cassiHasWork();

	return 0;
}

/**
 * registerMessageId
 * Sends a command envelope to the FABRIC thread to have all
 * messages matching the message ID sent to the ENGINE thread.
 * @param iMsgId - the message ID.
 * @return 0 in success, else queue full or error
 */
inttype FabricCassiMessageQueue::registerMessageId(uinttype iMsgId) {
	FabricEnvelope* pcCmd = new FabricEnvelope(REGISTER_MSG_ID_CMD, (voidptr)(ulongtype)iMsgId);
	if (0 == pcCmd) {
		DEEP_LOG_ERROR(OTHER, "Unable to allocate a command envelope\n");
		abort();
	}

	pcCmd->setFlags(cxx::io::encodeProtocol::header::ONE_MSG);

	return enqueue(pcCmd);
}

/**
 * LoadBridges
 * Loads any Bridge code specified by the function methods.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::LoadBridges(void) {
	if (0 == m_pcLoaderList) {
		return 0;
	}

	LoadListEntry* pcLoaderList = reinterpret_cast<LoadListEntry* >(m_pcLoaderList);
	for (inttype i=0; 0 != pcLoaderList[i].m_pfLoader; i++) {
		FabricCassiBridgeCb* pcCb = (pcLoaderList[i].m_pfLoader)(this, pcLoaderList[i].m_pCtx);
		inttype status = registerBridge(pcCb);
		if (0 != status) {
			return -1;
		}
	}

	return 0;
}

/**
 * processFabricLocalBridge
 * These are messages sent to FabCassi that orginate from the Fabric
 * Thread. These messages need to be delivered to all bridge
 * callbacks.
 * @param pcMsg - reference to the reader message.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::processFabricLocalBridge(cxx::io::encodeProtocol::reader* pcMsg) {
	cxx::util::Iterator<FabricCassiBridgeCb* >* pcIt = m_cBridgeCbs.iterator();
	uinttype iMsgId  = pcMsg->getMsgId();

	while (true == pcIt->hasNext()) {
		FabricCassiBridgeCb* pcCb = pcIt->next();
		if (0 == pcCb) {
			continue;
		}

		switch (iMsgId) {
		case SHIM_REP::MSG_ID:
			DEEP_LOG_INFO(OTHER, "-VE response from remote FABRIC\n");
			pcCb->remoteOverloaded(pcMsg);
			break;

		case EXIT_REQ::MSG_ID:
			DEEP_LOG_WARN(OTHER, "CASSI Thread has been signaled to exit.\n");
			pcCb->shutdown();
			break;

		case PEER_CHANGE::MSG_ID:
			DEEP_LOG_DEBUG(OTHER, "CASSI, Peer State change received.\n");
			pcCb->peerStateChanged(pcMsg);
			break;

		}
	}

	delete pcIt; pcIt = 0;

	return 0;
}

/**
 * processFabricLocal
 * These are messages sent to FabCassi that orginate from the Fabric
 * Thread.
 * @param pcMsg - reference to the reader message.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::processFabricLocal(cxx::io::encodeProtocol::reader* pcMsg) {
	uinttype        iMsgId  = pcMsg->getMsgId();
	JsonApiRequest* pcReq   = 0;
	inttype         iRetVal = 0;
	peerCmdArgs*    pcArgs  = 0;
	voidptr         pCtx    = 0;

	switch (iMsgId) {
	case ADD_PEER_REP::MSG_ID:
		DEEP_LOG_DEBUG(OTHER, "CASSI, ADD PEER REP received.\n");

		if (0 != pcMsg->getInt32Field(FabricCassiMessageQueue::ADD_PEER_REP::STATUS_FIELD, iRetVal)) {
			throw FabricCassiMessageQueueException("ADD_PEER_REP: no status found.");
		}

		if (0 != pcMsg->getVoidPtr(FabricCassiMessageQueue::ADD_PEER_REP::CONTEXT_FIELD, pCtx)) {
			throw FabricCassiMessageQueueException("ADD_PEER_REP: no context pointer found.");
		}

		pcArgs = reinterpret_cast<peerCmdArgs* >(pCtx);
		if (0 == pcArgs) {
			throw FabricCassiMessageQueueException("ADD_PEER_REP: not a peerCmdArgs block.");
		}

		pcReq = reinterpret_cast<JsonApiRequest* >(pcArgs->m_pCtx);
		if (0 != iRetVal) {
			pcReq->m_eRetCode    = JsonApi::HANDLER_BAD_REQUEST;
			pcReq->m_cErrorTitle = "Unable to ADD this peer IP Address.";
		}

		break;

	case DEL_PEER_REP::MSG_ID:
		DEEP_LOG_DEBUG(OTHER, "CASSI, DEL PEER REP received.\n");

		if (0 != pcMsg->getInt32Field(FabricCassiMessageQueue::DEL_PEER_REP::STATUS_FIELD, iRetVal)) {
			throw FabricCassiMessageQueueException("DEL_PEER_REP: no status found.");
		}

		if (0 != pcMsg->getVoidPtr(FabricCassiMessageQueue::DEL_PEER_REP::CONTEXT_FIELD, pCtx)) {
			throw FabricCassiMessageQueueException("DEL_PEER_REP: no context pointer found.");
		}

		pcArgs = reinterpret_cast<peerCmdArgs* >(pCtx);
		if (0 == pcArgs) {
			throw FabricCassiMessageQueueException("DEL_PEER_REP: not a peerCmdArgs block.");
		}

		pcReq = reinterpret_cast<JsonApiRequest* >(pcArgs->m_pCtx);
		if (0 != iRetVal) {
			pcReq->m_eRetCode    = JsonApi::HANDLER_NOT_FOUND;
			pcReq->m_cErrorTitle = "Unable to DELETE this peer.";
		}

		break;

	case GET_TOPO_REP::MSG_ID:
		DEEP_LOG_DEBUG(OTHER, "CASSI, GET TOPO REP received.\n");

		if (0 != pcMsg->getVoidPtr(FabricCassiMessageQueue::GET_TOPO_REP::CONTEXT_FIELD, pCtx)) {
			throw FabricCassiMessageQueueException("GET_TOPO_REP: no context pointer found.");
		}

		pcReq = reinterpret_cast<JsonApiRequest* >(pCtx);
		if (0 == pcReq) {
			throw FabricCassiMessageQueueException("Not a valid JSON Request block.");
		}

		break;

	case BUNDLE_REQ::MSG_ID:
		processBundleFromFabric(pcMsg);
		return 0;

	default:
		return processFabricLocalBridge(pcMsg);
	}

	inttype iStatus = sendPendingApiReply(pcReq);
	if (0 != iStatus) {
		DEEP_LOG_ERROR(OTHER, "Error enqueuing the REPLY to the API.\n");
		delete pcReq; pcReq = 0;
	}

	return 0;
}

/**
 * run
 * Main entry point for CASSI thread. TBD
 */
void FabricCassiMessageQueue::run(void) {
	/** Load up any bridge code */
	if (0 != LoadBridges()) {
		DEEP_LOG_ERROR(OTHER, "Unable to load up the bridge code.\n");
		abort();
	}

	/** Enable this threads poller mechanism */
	if (0 != enableCassiPoller()) {
		DEEP_LOG_ERROR(OTHER, "Unable to intialize the epoller for CASSI.\n");
		abort();
	}

	/** Start API consumer thread */
	FabricCassiAPIConsumer* fabricCassiAPIConsumerInstance = new FabricCassiAPIConsumer(this);
	m_pcFabricCassiAPIConsumerThread = new Thread(fabricCassiAPIConsumerInstance);
	m_pcFabricCassiAPIConsumerThread->start();

	
	DEEP_LOG_INFO(OTHER, "#%u FabricCassiMessageQueue Thread Running.\n", m_iServerId);

	boolean bExit = false;

	while (false == bExit) {
		boolean bHasWork = false;
		if (0 != cassiPoller(100, bHasWork)) {
			DEEP_LOG_ERROR(OTHER, "Error polling for work in CASSI.\n");
			abort();
		}

		if (true == bHasWork) {
			/**
			 * The aim is to consume the entire queue as
			 * quickly as possible.
			 */
			boolean bHasMore = false;

			do {
				cxx::io::encodeProtocol::reader* pcMsg = 0;

				inttype iStatus = dequeue(pcMsg, bHasMore);

				if ((0 == iStatus) && (0 != pcMsg)) {
					uinttype iMsgId = pcMsg->getMsgId();
#if DEEP_DEBUG_PACKET_FLOW
					DEEP_LOG_DEBUG(OTHER, "#%u FabricCassiMessageQueue Thread has work. MsgId=0x%x"
						       " (first=%d, last=%d, reply=%d)\n",
						       m_iServerId,
						       iMsgId,
						       pcMsg->isFirst(),
						       pcMsg->isLast(),
						       pcMsg->isReply());
#endif
					boolean del = true;

					if (true == HAS_PREFIX(msgPrefix::REALTIME_FABRIC_CASSI_SHIM, iMsgId)) {
						/** Dealing with Fabric to Cassi internal messages. */

						if (EXIT_REQ::MSG_ID == iMsgId) {
							bHasMore = false;
							bExit    = true;

							/** Clean up API consumer thread */
							m_bStopFabricCassiAPIConsumerThread = true;
							m_pcFabricCassiAPIConsumerThread->join();
							delete m_pcFabricCassiAPIConsumerThread;
							delete fabricCassiAPIConsumerInstance;
						}

						inttype iLocStatus = processFabricLocal(pcMsg);
						if (0 != iLocStatus) {
							throw FabricCassiMessageQueueException("Error processing local msg.");
						}

					} else {

						if (true == pcMsg->isLast()) {
							lastMessage();
						}

						FabricCassiBridgeCb* pcCb = m_cMsgToBridgeCbs.get(iMsgId);
						if (0 != pcCb) {
							del = (pcCb->dispatch(pcMsg) == 0);
						} else {
							del = true;
						}
					}

					if (true == del) {
						delete pcMsg;
					}

					pcMsg = 0;

				} else if (-1 == iStatus) {
					/** Empty queue. No big deal */
				} else {
					DEEP_LOG_ERROR(OTHER, "CASSI error process work queue.\n");
					abort();
				}

			} while (true == bHasMore);
		} else {
			pthread_yield();
		}
	}

	if (0 != unregisterAllBridges()) {
		DEEP_LOG_ERROR(OTHER, "Issue unregistering the bridge code callbacks.\n");
	}

	DEEP_LOG_INFO(OTHER, "CASSI exited loop.\n");
}

/**
 * signalExitCassi
 * Signal to the CASSI thread that it needs to shutdown and cleanup
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::signalExitCassi(void) {
	cxx::io::encodeProtocol::renderReader cSignalExit(FabricCassiMessageQueue::EXIT_REQ::MSG_ID);
	cxx::io::encodeProtocol::reader*      pcReader = cSignalExit.mutableReader();
	if (0 == pcReader) {
		DEEP_LOG_ERROR(OTHER, "Unable to create Exit message to CASSI!\n");
		abort();
	}

	return enqueue(pcReader);
}

/**
 * signalPeerChange
 * Signal to the CASSI thread a change in peer/server state.
 * @param iServerId - ID of the peer/server.
 * @param eState - UP/DOWN state of the server,
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::signalPeerChange(uinttype                                    iServerId,
						  FabricCassiMessageQueue::PEER_CHANGE::State eState) {
	cxx::io::encodeProtocol::renderReader cSignalChange(FabricCassiMessageQueue::PEER_CHANGE::MSG_ID);

	cSignalChange.setUint32Field(FabricCassiMessageQueue::PEER_CHANGE::SERVER_ID_FIELD,
				     iServerId);
	cSignalChange.setInt32Field(FabricCassiMessageQueue::PEER_CHANGE::STATE_FIELD,
				    eState);

	cxx::io::encodeProtocol::reader* pcReader = cSignalChange.mutableReader();
	if (0 == pcReader) {
		DEEP_LOG_ERROR(OTHER, "Unable to create Peer change message to CASSI!\n");
		abort();
	}

	return enqueue(pcReader);
}

/**
 * signalAddPeerRep
 * Signal to the CASSI thread a status reply for the ADD PEER Request.
 * @param iStatus - The return status.
 * @param pCtx - context block sent from the FabCassi Thread.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::signalAddPeerRep(inttype iStatus, voidptr pCtx){
	cxx::io::encodeProtocol::renderReader cSignal(FabricCassiMessageQueue::ADD_PEER_REP::MSG_ID);

	cSignal.setInt32Field(FabricCassiMessageQueue::ADD_PEER_REP::STATUS_FIELD,
			      iStatus);
	cSignal.setVoidPtr(FabricCassiMessageQueue::ADD_PEER_REP::CONTEXT_FIELD,
			   pCtx);

	cxx::io::encodeProtocol::reader* pcReader = cSignal.mutableReader();
	if (0 == pcReader) {
		DEEP_LOG_ERROR(OTHER, "Unable to create Peer change message to CASSI!\n");
		abort();
	}

	return enqueue(pcReader);
}

/**
 * signalDelPeerRep
 * Signal to the CASSI thread a status reply for the DEL PEER Request.
 * @param iStatus - The return status.
 * @param pCtx - context block sent from the FabCassi Thread.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::signalDelPeerRep(inttype iStatus, voidptr pCtx){
	cxx::io::encodeProtocol::renderReader cSignal(FabricCassiMessageQueue::DEL_PEER_REP::MSG_ID);

	cSignal.setInt32Field(FabricCassiMessageQueue::DEL_PEER_REP::STATUS_FIELD,
			      iStatus);
	cSignal.setVoidPtr(FabricCassiMessageQueue::DEL_PEER_REP::CONTEXT_FIELD,
			   pCtx);

	cxx::io::encodeProtocol::reader* pcReader = cSignal.mutableReader();
	if (0 == pcReader) {
		DEEP_LOG_ERROR(OTHER, "Unable to create Peer change message to CASSI!\n");
		abort();
	}

	return enqueue(pcReader);
}

/** FabricCassiBridgeCb */

/**
 * FabricCassiBridgeCb
 * Constructor
 * @param pcCommon - Reference to the FabricCassiMessageQueue instance
 * that this object is registered with.
 */
FabricCassiBridgeCb::FabricCassiBridgeCb(FabricCassiMessageQueue* pcCommon) :
	m_pcCommon(pcCommon),
	m_pcBundle(0) {
}

/**
 * ~FabricCassiBridgeCb
 * Destructor
 */
FabricCassiBridgeCb::~FabricCassiBridgeCb() {
	m_pcCommon    = 0;

	if (0 != m_pcBundle) {
		cxx::util::Iterator<FabricEnvelope* >* pcIt = m_pcBundle->m_cBundle.iterator();

		while (true == pcIt->hasNext()) {
			FabricEnvelope* pcQMsg = pcIt->next();
			if (0 == pcQMsg) {
				DEEP_LOG_ERROR(OTHER, "Unexpected value for message reference\n");
				abort();
			}

			pcIt->remove();
			delete pcQMsg; pcQMsg = 0;
		}

		delete pcIt; pcIt = 0;

		delete m_pcBundle; m_pcBundle = 0;
	}
}

/**
 * send
 * Ensures that if we are building batches of messages, to bundle them
 * up and send the complete bundle to the fabric. This is used to
 * avoid incomplete bundles being sent that can cause the fabric
 * thread to abort.
 * @param pcMsg - the message to send.
 * @return 0 in success.
 */
inttype FabricCassiBridgeCb::send(FabricEnvelope* pcMsg) {
	uinttype iFlags = 0;

	pcMsg->getFlags(iFlags);

	iFlags = iFlags ^ cxx::io::encodeProtocol::header::VERSION;

	if (0 != m_pcBundle) {
		if ((cxx::io::encodeProtocol::header::FIRST_FLAG & iFlags) ==
		    cxx::io::encodeProtocol::header::FIRST_FLAG) {
			DEEP_LOG_ERROR(OTHER, "First of a bundle and bundle already in progress! Multi-threading issue?\n");
			abort();

		} else if ((cxx::io::encodeProtocol::header::LAST_FLAG & iFlags) ==
		    cxx::io::encodeProtocol::header::LAST_FLAG) {

			m_pcBundle->m_cBundle.add(pcMsg);
			m_pcBundle->m_iTotalByteSize += pcMsg->getSize();

			if (0 != m_pcCommon->sendBundleQ(pcMsg->getPeerId(), pcMsg->getMsgId(), m_pcBundle)) {
				DEEP_LOG_ERROR(OTHER, "Unable to forward the bundle.\n");
				abort();
			}

			m_pcBundle = 0;

		} else {
			m_pcBundle->m_cBundle.add(pcMsg);
			m_pcBundle->m_iTotalByteSize += pcMsg->getSize();
		}

	} else {
		if (0 == iFlags) {
			DEEP_LOG_ERROR(OTHER, "No inprogress bundle and message does not contain a FIRST flag!\n");
			abort();

		} else if ((cxx::io::encodeProtocol::header::ONE_MSG & iFlags) ==
			   cxx::io::encodeProtocol::header::ONE_MSG) {
			if (0 != m_pcCommon->enqueue(pcMsg)) {
				DEEP_LOG_ERROR(OTHER, "Unable to enqueue the message to the Fabric.\n");
				return -3;
			}

		} else if ((cxx::io::encodeProtocol::header::FIRST_FLAG & iFlags) ==
			   cxx::io::encodeProtocol::header::FIRST_FLAG) {
			m_pcBundle = new FabricCassiMessageQueue::BundleList;

			m_pcBundle->m_cBundle.add(pcMsg);
			m_pcBundle->m_iTotalByteSize += pcMsg->getSize();

		} else {
			DEEP_LOG_ERROR(OTHER, "Bundle and message has inappropriate flags!\n");
			abort();
		}
	}

	return 0;
}

/**
 * registerApiCallback
 * Registers an API Callback handler for the specific path.
 * @param cPath - the path for the API JSON Request
 * @param pcApiHandler - a reference of the callback to associate with
 * this path.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::registerApiCallback(cxx::lang::String cPath,
						     ApiCallback*      pcApiHandler) {
	if (0 != m_pcApiHandlers->get(&cPath)) {
		return -1;
	}

	cxx::lang::String* pcPath = new cxx::lang::String(cPath);

	m_pcApiHandlers->put(pcPath, pcApiHandler);

	return 0;
}

/**
 * unregisterApiCallback
 * Registers an API Callback handler for the specific path.
 * @param cPath - the path for the API JSON Request
 * @param pcApiHandler - a reference of the callback to associate with
 * this path.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::unregisterApiCallback(cxx::lang::String cPath) {
	if (0 == m_pcApiHandlers->get(&cPath)) {
		return -1;
	}

	m_pcApiHandlers->remove(&cPath);

	return 0;
}

/**
 * addPeerCmd
 * Sends the addPeer command and arguments to the RealtimeFabric
 * thread to add the required peer as a connection.
 * @param cPath    - the command API path
 * @param cAddress - the IP Address, e.g. tcp://x.x.x.x:yyyyy
 * @param cUUID    - a UUID to associate with this connection.
 * @return 0 in success, else queue full or error
 */
 inttype FabricCassiMessageQueue::addPeerCmd(JsonApiRequest*    pcReq,
					     cxx::lang::String& cPath,
					     cxx::lang::String& cAddress,
					     cxx::util::UUID&   cUUID) {
	peerCmdArgs* pcArgs = new peerCmdArgs;
	pcArgs->m_cAddress  = cAddress;
	pcArgs->m_pcUUID    = new cxx::util::UUID(cUUID.getMostSignificantBits(),
						  cUUID.getLeastSignificantBits());
	pcArgs->m_pCtx      = pcReq;

	if (0 == pcArgs->m_pcUUID) {
		throw FabricCassiMessageQueueException("Unable to create UUID for addPeer command.");
	}

	FabricEnvelope* pcCmd = new FabricEnvelope(ADD_PEER_MSG_ID_CMD, (voidptr)pcArgs);
	if (0 == pcCmd) {
		throw FabricCassiMessageQueueException("Unable to create addPeer command arguments.");
	}

	pcCmd->setFlags(cxx::io::encodeProtocol::header::ONE_MSG);
	if (0 == enqueue(pcCmd)) {
		cxx::lang::String cDelPath;

		DelPeerApiHandler* pcHdlr = static_cast<DelPeerApiHandler*>(m_pcDelPeerApiHandler);

		if (0 == pcHdlr->addMapping(cPath, cUUID, cDelPath)) {
			inttype iStatus = registerApiCallback(cDelPath, m_pcDelPeerApiHandler);
			if (0 != iStatus) {
				throw FabricCassiMessageQueueException("Unable to register the Fabric API handler.");
			}
		}
	}

	return 0;
}

/**
 * delPeerCmd
 * Sends the delPeer command and arguments to the RealtimeFabric
 * thread to add the required peer as a connection.
 * @param cUUID    - a UUID to associate with this connection.
 * @return 0 in success, else queue full or error
 */
inttype FabricCassiMessageQueue::delPeerCmd(JsonApiRequest*  pcReq,
					    cxx::util::UUID& cUUID) {
	peerCmdArgs* pcArgs = new peerCmdArgs;
	pcArgs->m_pcUUID    = new cxx::util::UUID(cUUID.getMostSignificantBits(),
						  cUUID.getLeastSignificantBits());
	pcArgs->m_pCtx      = pcReq;

	if (0 == pcArgs->m_pcUUID) {
		throw FabricCassiMessageQueueException("Unable to create UUID for delPeer command.");
	}

	FabricEnvelope* pcCmd = new FabricEnvelope(DEL_PEER_MSG_ID_CMD, (voidptr)pcArgs);
	if (0 == pcCmd) {
		throw FabricCassiMessageQueueException("Unable to create addPeer command arguments.");
	}

	pcCmd->setFlags(cxx::io::encodeProtocol::header::ONE_MSG);

	return enqueue(pcCmd);
}

/**
 * sendPendingApiReply
 * Call this to post a deffered/pending reply to an API request.
 * @param pcReq - the original request with any return status/error
 * title that can be modified for return to sender.
 * @return 0 on success.
 */
FORCE_INLINE inttype FabricCassiMessageQueue::sendPendingApiReply(JsonApiRequest* pcReq) {
	inttype iStatus = m_pcJsonApi->enqueueREP(pcReq);
	return iStatus;
}

/**
 * getTopo
 * Get the current Fabric Topology
 * @param pcReq - The GET request block.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::getTopo(JsonApiRequest* pcReq) {
	FabricEnvelope* pcCmd = new FabricEnvelope(GET_TOPO_MSG_ID_CMD, (voidptr)pcReq);
	if (0 == pcCmd) {
		throw FabricCassiMessageQueueException("Unable to create getTopo command.");
	}

	pcCmd->setFlags(cxx::io::encodeProtocol::header::ONE_MSG);

	return enqueue(pcCmd);
}

/**
 * processCassiCommand
 * Process the POST request with the CASSI command/payload
 * @param pcReq - The POST request.
 * @return 0 on success
 */
inttype FabricCassiMessageQueue::processCassiCommand(JsonApiRequest* pcReq) {
	if ((false == pcReq->m_pcData->exists("command")) ||
	    (false == pcReq->m_pcData->exists("payload"))) {
		return -1;
	}

	json::value command = pcReq->m_pcData->operator[]("command");
	json::value payload = pcReq->m_pcData->operator[]("payload");

	m_pcDcDb->processJSONCommand(command, payload);

	pcReq->m_pcRetData   = m_pcDcDb->return_json;
	pcReq->m_cErrorTitle = m_pcDcDb->return_error_message;

	return 0;
}

/**
 * signalGetTopoRep
 * Called from the Fabric to send back the topology response.
 * @param iStatus - The return status for this request
 * @param pCtx - the request block context.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::signalGetTopoRep(voidptr pCtx){
	cxx::io::encodeProtocol::renderReader cSignal(FabricCassiMessageQueue::GET_TOPO_REP::MSG_ID);

	cSignal.setVoidPtr(FabricCassiMessageQueue::GET_TOPO_REP::CONTEXT_FIELD, pCtx);

	cxx::io::encodeProtocol::reader* pcReader = cSignal.mutableReader();
	if (0 == pcReader) {
		throw FabricCassiMessageQueueException("Unable to create the Get Topology REPly.");
	}

	return enqueue(pcReader);
}

/**
 * sendBundleQ
 * Sends the packaged bundle to the fabric for forwarding to the peer.
 * @param pBundle - reference to the Bundle being sent to the peer.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::sendBundleQ(uinttype iPeerId, uinttype iMsgId, voidptr pBundle) {
	FabricEnvelope* pcCmd = new FabricEnvelope(BUNDLED_MSG_ID_CMD, pBundle);
	if (0 == pcCmd) {
		throw FabricCassiMessageQueueException("Unable to create getTopo command.");
	}

	pcCmd->setPeerId(iPeerId);
	pcCmd->setMsgId(iMsgId);
	pcCmd->setFlags(cxx::io::encodeProtocol::header::ONE_MSG);

	return enqueue(pcCmd);
}

/**
 * signalBundleReq
 * Signal to the FCQ that we have a bundle list to process.
 * @param pBundleArray - the bundle array reference to pass up.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::signalBundleReq(voidptr pBundleArray) {
	cxx::io::encodeProtocol::renderReader cSignal(FabricCassiMessageQueue::BUNDLE_REQ::MSG_ID);

	cSignal.setVoidPtr(FabricCassiMessageQueue::BUNDLE_REQ::CONTEXT_FIELD, pBundleArray);

	cxx::io::encodeProtocol::reader* pcReader = cSignal.mutableReader();
	if (0 == pcReader) {
		throw FabricCassiMessageQueueException("Unable to create the Get Topology REPly.");
	}

	return enqueue(pcReader);
}

/**
 * processBundleFromFabric
 * Processes the arraylist of messages from the fabric. Where possible
 * the callback is cached to prevent continuous hash lookups.
 * @param pcMsg - message containing the arraylist pointer.
 * @returns 0. All errors throw and exception.
 */
inttype FabricCassiMessageQueue::processBundleFromFabric(cxx::io::encodeProtocol::reader* pcMsg) {
	voidptr pCtx = 0;

	if (0 != pcMsg->getVoidPtr(FabricCassiMessageQueue::BUNDLE_REQ::CONTEXT_FIELD, pCtx)) {
		throw FabricCassiMessageQueueException("BUNDLE_REQ: no context pointer found.");
	}

	cxx::util::ArrayList<cxx::io::encodeProtocol::reader*>* pcBundle =
		reinterpret_cast<cxx::util::ArrayList<cxx::io::encodeProtocol::reader*> *>(pCtx);
	if (0 == pcBundle) {
		throw FabricCassiMessageQueueException("Not a Array Bundle of messages!");
	}

	pcBundle->setDeleteValue(false);

	uinttype             iMsgId  = 0;
	uinttype             iLastId = 0;
	FabricCassiBridgeCb* pcCb    = 0;

	for (inttype i=0; i<pcBundle->size(); i++) {
		cxx::io::encodeProtocol::reader* pcQMsg = pcBundle->get(i);
		if (0 == pcQMsg) {
			throw FabricCassiMessageQueueException("Unexpected value for message reference\n");
		}

		iMsgId = pcQMsg->getMsgId();
		if (0 == iMsgId) {
			pcCb = 0;
		} else 	if (iLastId != iMsgId) {
			pcCb = m_cMsgToBridgeCbs.get(iMsgId);
		}

		iLastId = iMsgId;

		boolean del = true;

		if (true == pcQMsg->isLast()) {
			lastMessage();
		}

		if (0 != pcCb) {
			del = (pcCb->dispatch(pcQMsg) == 0);
		} else {
			del = true;
		}

		if (true == del) {
			delete pcQMsg; pcQMsg = 0;
		}
	}

	return 0;
}

/**
 * lastMessage
 * Tell the Fabric we got the last message in the bundle.
 * @param pcReq = The GET request block.
 * @return 0 on success.
 */
inttype FabricCassiMessageQueue::lastMessage(void) {
	FabricEnvelope* pcCmd = new FabricEnvelope(LAST_MESSAGE_ID_CMD, 0);
	if (0 == pcCmd) {
		throw FabricCassiMessageQueueException("Unable to create LAST_MESSAGE_ID_CMD command.");
	}

	pcCmd->setFlags(cxx::io::encodeProtocol::header::ONE_MSG);

	return enqueue(pcCmd);
}

/**
 * FabricCassiAPIConsumer constructor
 */
FabricCassiAPIConsumer::FabricCassiAPIConsumer(FabricCassiMessageQueue* pcParent) {
	m_pcParent = pcParent;
}

/**
 * run
 * run the thread that consumes and executes API commands
 */
void FabricCassiAPIConsumer::run(void) {
	while (m_pcParent->m_bStopFabricCassiAPIConsumerThread == false) {
		if (true == m_pcParent->m_pcJsonApi->cassHasWorkPoll(10)) {
			JsonApiRequest* pcReq = 0;

			inttype iStatus = m_pcParent->m_pcJsonApi->dequeueREQ(pcReq);
			if ((0 == iStatus) && (0 != pcReq)) {
				DEEP_LOG_DEBUG(OTHER, "FabricCassi REQUEST: ID=%s, %s->%d\n",
					       pcReq->m_cIdStr.c_str(),
					       pcReq->m_cPath.c_str(),
					       pcReq->m_eRequest);

				com::deepis::communication::fabricconnector::ApiCallback* pcCb = 0;
				inttype iCbStatus =
					com::deepis::communication::fabricconnector::ApiCallback::IMMEDIATE_REPLY_RETVAL;

				pcCb = m_pcParent->m_pcApiHandlers->get(&pcReq->m_cPath);
				if (0 == pcCb) {
					pcReq->m_eRetCode    = JsonApi::HANDLER_NOT_FOUND;
					pcReq->m_cErrorTitle = "No handler support just yet.";
				} else {
					iCbStatus = pcCb->callback(pcReq);
				}

				if (ApiCallback::IMMEDIATE_REPLY_RETVAL == iCbStatus) {
					iStatus = m_pcParent->m_pcJsonApi->enqueueREP(pcReq);
					if (0 != iStatus) {
						DEEP_LOG_ERROR(OTHER, "Error enqueuing the REPLY to the API.\n");
						delete pcReq; pcReq = 0;
					}
				} else if (0 > iCbStatus) {
					pcReq->m_eRetCode    = JsonApi::HANDLER_INTERNAL_ERROR;
					pcReq->m_cErrorTitle = "API Handler found an error in the REQUEST.";
				}
			} else {
				DEEP_LOG_ERROR(OTHER, "Error dequeuing REQUEST from the API.\n");
				if (0 != pcReq) {
					delete pcReq; pcReq = 0;
				}
			}
		}
	}
}
