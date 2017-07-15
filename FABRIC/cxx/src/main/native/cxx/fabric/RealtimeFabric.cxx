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
#include "cxx/util/Logger.h"

#include "RealtimeFabric.h"
#include "com/deepis/communication/common/FabricEnvelope.h"

#include <sstream>
#include <json.h>
#include <json_parser.h>

#include "com/deepis/communication/fabricconnector/DummyBridge.h"

#include "cxx/util/Time.h"

using namespace cxx::lang;
using namespace cxx::util;
using namespace com::deepis::communication::common;
using namespace com::deepis::communication::fabricconnector;

#define DEBUG_MESSAGE_FLOW 0
#define DEBUG_MESSAGE_BUNDLE 0
#define DEEP_ARRAY_BUNDLE 0

/**
 * RealtimeFabric
 * Constructor
 */
cxx::fabric::RealtimeFabric::RealtimeFabric(const uinttype          iServerId,
					    const cxx::lang::String cRepAddress,
					    void*                   pcLoaderList,
					    ulongtype               iMemoryCap) :
	m_iServerId(iServerId),
	m_cRepAddress(cRepAddress),
	m_pcMsgMgr(0),
	m_repSock(0),
	m_cJoinReqCb(this),
	m_cJoinRepCb(this),
	m_cTopoReqCb(this),
	m_cTopoRepCb(this),
	m_cShimReqCb(this),
	m_cShimRepCb(this),
	m_cPackRepCb(this),
	m_pcInBoundPeers(0),
	m_pcInBoundPeersByName(0),
	m_pcOutBoundPeers(0),
	m_pcOutBoundPeersByName(0),
	m_pcOutBoundPeersByUUID(0),
	m_cMsgMgrNotifications(this),
	m_cTopology(iServerId),
	m_pcCassiShim(0),
	m_pcCassiThread(0),
	m_pcCassiFabQCb(0),
	m_iBundleCount(0),
	m_iTxBundleTimeStart(0),
	m_iRxBundleTimeStart(0),
	m_pcBatch(0) {

	m_pcMsgMgr = new cxx::io::MessageManager;
	if (0 == m_pcMsgMgr) {
		throw RealtimeFabricException("Unable to create the MessageManager.");
	}

	m_pcInBoundPeers = new cxx::util::HashMap<uinttype, inBound*>(16, false, false, false);
	if (0 == m_pcInBoundPeers) {
		throw RealtimeFabricException("Unable to create the Inbound Peers Map.");
	}

	m_pcInBoundPeersByName = new cxx::util::HashMap<cxx::lang::String*, inBound*>(16, true, true, false);
	if (0 == m_pcInBoundPeersByName) {
		throw RealtimeFabricException("Unable to create the Inbound Peers By Name Map.");
	}

	m_pcOutBoundPeers = new cxx::util::HashMap<uinttype, outBound*>(16, false, false, false);
	if (0 == m_pcOutBoundPeers) {
		throw RealtimeFabricException("Unable to create the Outbound Peers Map.");
	}

	m_pcOutBoundPeersByName = new cxx::util::HashMap<cxx::lang::String*, outBound*>(16, true, true, false);
	if (0 == m_pcOutBoundPeersByName) {
		throw RealtimeFabricException("Unable to create the Outbound Peers By Name Map.");
	}

	m_pcOutBoundPeersByUUID = new cxx::util::HashMap<cxx::util::UUID*, outBound*>(16, true, false, false);
	if (0 == m_pcOutBoundPeersByUUID) {
		throw RealtimeFabricException("Unable to create the Outbound Peers By UUID Map.");
	}

	m_pcCassiShim = new FabricCassiMessageQueue(iServerId, pcLoaderList, iMemoryCap);
	if (0 == m_pcCassiShim) {
		throw RealtimeFabricException("Unable to create the FabricCassi Shim.");
	}

	m_pcCassiThread = new cxx::lang::Thread(m_pcCassiShim);
	if (0 == m_pcCassiThread) {
		throw RealtimeFabricException("Unable to create the FabricCassi Thread.");
	}

	m_pcCassiFabQCb = new cassiFabQCb(this);
	if (0 == m_pcCassiFabQCb) {
		throw RealtimeFabricException("Unable to create a cassiFabQCb.");
	}

	ulongtype status = m_pcMsgMgr->addRepSocket(m_cRepAddress, m_repSock);
	if (0 != status) {
		throw RealtimeFabricException("Unable to create socket.");
	}

	status = m_pcMsgMgr->registerMessageId(JOIN_REQ::MSG_ID,
					       &m_cJoinReqCb);
	if (0 != status) {
		throw RealtimeFabricException("Unable to register callback.");
	}

	status = m_pcMsgMgr->registerMessageId(JOIN_REP::MSG_ID,
					       &m_cJoinRepCb);
	if (0 != status) {
		throw RealtimeFabricException("Unable to register callback.");
	}

	status = m_pcMsgMgr->registerMessageId(TOPO_REQ::MSG_ID,
					       &m_cTopoReqCb);
	if (0 != status) {
		throw RealtimeFabricException("Unable to register callback.");
	}

	status = m_pcMsgMgr->registerMessageId(TOPO_REP::MSG_ID,
					       &m_cTopoRepCb);
	if (0 != status) {
		throw RealtimeFabricException("Unable to register callback.");
	}

	status = m_pcMsgMgr->registerMessageId(SHIM_REQ::MSG_ID,
					       &m_cShimReqCb);
	if (0 != status) {
		throw RealtimeFabricException("Unable to register callback.");
	}

	status = m_pcMsgMgr->registerMessageId(FabricCassiMessageQueue::SHIM_REP::MSG_ID,
					       &m_cShimRepCb);
	if (0 != status) {
		throw RealtimeFabricException("Unable to register callback.");
	}

	status = m_pcMsgMgr->registerMessageId(PACK_BUNDLE_REQ::MSG_ID,
					       &m_cPackRepCb);
	if (0 != status) {
		throw RealtimeFabricException("Unable to register callback.");
	}

	status = m_pcMsgMgr->registerNotifications(&m_cMsgMgrNotifications);
	if (0 != status) {
		throw RealtimeFabricException("Unable to register the notifications callback.");
	}

	status = m_pcMsgMgr->registerNativeCallback(m_pcCassiShim->cassiToFabricEventFd(), m_pcCassiFabQCb);
	if (0 != status) {
		throw RealtimeFabricException("Unable to register to get native FD events on CassiFab Q.");
	}

	DEEP_LOG_DEBUG(OTHER, "RealtimeFabric constructed, listening on %s.\n",
		       m_cRepAddress.c_str());

	m_pcCassiThread->start();
}

/**
 * getFabricCassiMessageQueue
 * get underlying queue
 */
FabricCassiMessageQueue* cxx::fabric::RealtimeFabric::getFabricCassiMessageQueue() {
	return m_pcCassiShim;
}

/**
 * RealtimeFabric
 * Destructor
 */
cxx::fabric::RealtimeFabric::~RealtimeFabric() {
	stopService();
}

/**
 * stopService
 * This permanently stops the RealtimeFabric service. Lots of cleanup
 * and subsystem shutdown.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::stopService(){

	cxx::lang::String cJson;
	m_cTopology.renderAsJSON(cJson);
	DEEP_LOG_DEBUG(OTHER, "#%u, Topology at shutdown: %s\n", m_iServerId, cJson.c_str());

	if (0 != m_pcCassiThread) {
		m_pcCassiShim->signalExitCassi();
		m_pcCassiThread->join();

		delete m_pcCassiThread; m_pcCassiThread = 0;
	}

	if (0 != m_pcCassiShim) {
		delete m_pcCassiShim; m_pcCassiShim = 0;
	}

	if (0 != m_pcMsgMgr) {
		ulongtype status = m_pcMsgMgr->unRegisterMessageId(JOIN_REQ::MSG_ID);
		if (0 != status) {
			throw RealtimeFabricException("Unable to unregister callback.");
		}
	
		status = m_pcMsgMgr->unRegisterMessageId(JOIN_REP::MSG_ID);
		if (0 != status) {
			throw RealtimeFabricException("Unable to unregister callback.");
		}

		status = m_pcMsgMgr->unRegisterMessageId(TOPO_REQ::MSG_ID);
		if (0 != status) {
			throw RealtimeFabricException("Unable to unregister callback.");
		}
	
		status = m_pcMsgMgr->unRegisterMessageId(TOPO_REP::MSG_ID);
		if (0 != status) {
			throw RealtimeFabricException("Unable to unregister callback.");
		}

		if (0 != m_repSock) {
			m_pcMsgMgr->close(m_repSock);
			m_repSock = 0;
		}

		status = m_pcMsgMgr->unregisterNotifications(&m_cMsgMgrNotifications);
		if (0 != status) {
			throw RealtimeFabricException("Unable to unregister the notifications callback.");
		}

		delete m_pcMsgMgr; m_pcMsgMgr = 0;
	}

	if (0 != m_pcInBoundPeers) {
		delete m_pcInBoundPeers; m_pcInBoundPeers = 0;
	}

	if (0 != m_pcInBoundPeersByName) {
		delete m_pcInBoundPeersByName; m_pcInBoundPeersByName = 0;
	}

	if (0 != m_pcOutBoundPeers) {
		delete m_pcOutBoundPeers; m_pcOutBoundPeers = 0;
	}

	if (0 != m_pcOutBoundPeersByName) {
		delete m_pcOutBoundPeersByName; m_pcOutBoundPeersByName = 0;
	}

	if (0 != m_pcOutBoundPeersByUUID) {
		delete m_pcOutBoundPeersByUUID; m_pcOutBoundPeersByUUID = 0;
	}

	m_iBundleCount       = 0;
	m_iTxBundleTimeStart = 0;
	m_iRxBundleTimeStart = 0;

	if (0 != m_pcBatch) {
		delete m_pcBatch; m_pcBatch = 0;
	}

	return 0;
}

/**
 * addPeer
 * Add/create a connection to the peer RTFabic. Can only create on
 * peer connection. Duplicates raise an error.
 * @param cAddress [in] - the RealtimeFabric peer IP Address.
 * @param pcUUID [in] - UUID for this Peer. Function takes ownership
 * of the UUID reference.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::addPeer(const cxx::lang::String cActual,
					       cxx::util::UUID&        cUUID) {
	if (0 !=  m_pcOutBoundPeersByName->get(const_cast<cxx::lang::String* >(&cActual))) {
		/** Peer adjacency is already created. */
		return 1;
	}

	outBound* pcOut = new outBound;
	if (0 == pcOut) {
		DEEP_LOG_ERROR(OTHER, "Unable to allocate an outbound connection.\n");
		abort();
	}

	pcOut->m_cServerAddr = cActual;

	/** Setup Map for Name to outBound. */
	cxx::lang::String* pcActual = new cxx::lang::String(cActual);
	m_pcOutBoundPeersByName->put(pcActual, pcOut);

	/** Setup Map for UUID to outBound. */
	cxx::util::UUID* pcUUID = new cxx::util::UUID(cUUID.getMostSignificantBits(),
						      cUUID.getLeastSignificantBits());
	m_pcOutBoundPeersByUUID->put(pcUUID, pcOut);

	/** Create a connection to this peer. */
	uinttype status = m_pcMsgMgr->addReqSocket(*pcActual, pcOut->m_reqSocket);
	if (0 != status) {
		return 2;
	}

	DEEP_LOG_INFO(OTHER, "Connecting to %s (UUID: %s)\n",
		      pcActual->c_str(),
		      pcUUID->toString().c_str());

	/** Encode the JOIN_REQ */
	cxx::io::encodeProtocol::writer* pcWriter    = 0;

	status = encodeJoinReq(pcWriter);
	if ((0 != status) || (0 == pcWriter)) {
		return 3;
	}

	errno = 0;

	uinttype iStatus = m_pcMsgMgr->sendREQ(pcOut->m_reqSocket, *pcWriter);

	delete pcWriter; pcWriter = 0;

	if (0 != iStatus) {
		return 4;
	}

	return 0;
}

/**
 * delPeer
 * Delete the Peer by the server address.
 * @param cAddress [in] - value for the address
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::delPeer(const cxx::lang::String cAddress) {
	cxx::lang::String* pcRetKey = 0;
	boolean            bRemoved = false;
	outBound*          pcOut = m_pcOutBoundPeersByName->remove(const_cast<cxx::lang::String* >(&cAddress),
								 &pcRetKey,
								 &bRemoved);
	if ((false == bRemoved) || (0 == pcOut)) {
		/** Peer adjacency does not exist. */
		return 1;
	}
	
	if (0 != pcRetKey) {
		delete pcRetKey; pcRetKey = 0;
	}

	uinttype iServerId = pcOut->m_iServerId;
	if (0 != iServerId) {
		m_pcOutBoundPeers->remove(iServerId);
	}

	cxx::util::UUID* pcUUID = pcOut->m_pcUUID;
	if (0 != pcUUID) {
		cxx::util::UUID* pcRetKey = 0;
		m_pcOutBoundPeersByUUID->remove(pcUUID, &pcRetKey);

		if (0 != pcRetKey) {
			delete pcRetKey; pcRetKey = 0;
		}
	}

	ulongtype iRetVal = 0;

	if (0 != m_pcMsgMgr->close(pcOut->m_reqSocket)) {
		iRetVal = 2;
	}

	delAdj(pcOut->m_iServerId);

	delete pcOut; pcOut = 0;

	return iRetVal;
}

/**
 * delPeer
 * Delete the peer by the UUID.
 * @param cUUID [in/ref] - the UUID for the peer.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::delPeer(cxx::util::UUID& cUUID) {
	cxx::util::UUID* pcRetKey = 0;
	boolean          bRemoved = false;
	outBound*        pcOut = m_pcOutBoundPeersByUUID->remove(&cUUID,
								 &pcRetKey,
								 &bRemoved);
	if ((false == bRemoved) || (0 == pcOut)) {
		/** Peer adjacency does not exist. */
		return 1;
	}
	
	if (0 != pcRetKey) {
		delete pcRetKey; pcRetKey = 0;
	}

	uinttype iServerId = pcOut->m_iServerId;
	if (0 != iServerId) {
		m_pcOutBoundPeers->remove(iServerId);
	}

	cxx::lang::String* pcRetAdd = 0;
	m_pcOutBoundPeersByName->remove(&pcOut->m_cServerAddr, &pcRetAdd);

	if (0 != pcRetAdd) {
		delete pcRetAdd; pcRetAdd = 0;
	}

	ulongtype iRetVal = 0;

	if (0 != m_pcMsgMgr->close(pcOut->m_reqSocket)) {
		iRetVal = 2;
	}

	delAdj(pcOut->m_iServerId);

	delete pcOut; pcOut = 0;

	return iRetVal;
}

/**
 * poll
 * Poll for fabric events.
 * @param iTimeout [in] - milliseconds to block waiting for an event.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::poll(ulongtype iTimeout) {

	if (0 != m_pcMsgMgr->poll(iTimeout)) {
		return 1;
	}

	return 0;
}

/**
 * processJoinReq
 * Process a Join Request from a peer FABRIC. On success will send a
 * Join Reply message back to the sender.
 * @param pcRxMsg [in] - reference to the destreamed join message.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::
processJoinReq(cxx::io::encodeProtocol::reader* pcRxMsg) {
	inBound* pcIn      = 0;
	uinttype iServerId = 0;

	/**
	 * Extract the server ID and see if it matches a
	 * known IN Block.
	 */
	inttype status = pcRxMsg->getUint32Field(JOIN_REQ::SERVER_ID_FIELD,
						 iServerId);
	if ((0 != status) || (0 == iServerId)) {
		DEEP_LOG_ERROR(OTHER, "Could not obtain the server ID\n");
		return 1;
	}

	pcIn = m_pcInBoundPeers->get(iServerId);
	if (0 == pcIn) {
		/**
		 * This is new to us.
		 */
		cxx::lang::String* pcName = new cxx::lang::String;
		inttype status = pcRxMsg->getStringField(JOIN_REQ::SERVER_FIELD, *pcName);
		if (0 != status) {
			delete pcName; pcName = 0;
			return 2;
		}

		pcIn = m_pcInBoundPeersByName->get(pcName);
		if (0 == pcIn) {
			/** Not found, so we create the ADJ */
			pcIn = new inBound;
			if (0 == pcIn) {
				DEEP_LOG_ERROR(OTHER, "Unable to allocate the inBound join block.\n");
				delete pcName; pcName = 0;
				return 3;
			}

			pcIn->m_cServerAddr = *pcName;

			m_pcInBoundPeersByName->put(pcName, pcIn);
		}

		m_pcInBoundPeers->put(iServerId, pcIn);
	}

	pcIn->m_iServerId = iServerId;

	/** ACK Back with the JOIN_REP to the sender. */
	uinttype iSize =
		cxx::io::encodeProtocol::writer::sizeOfUint32(m_iServerId) + \
		cxx::io::encodeProtocol::writer::sizeOfString(m_cRepAddress);

	cxx::io::encodeProtocol::writer cWriter(iSize, JOIN_REP::MSG_ID);

	status = cWriter.setUint32Field(JOIN_REP::SERVER_ID_FIELD, m_iServerId);
	if (0 != status) {
		return 1;
	}

	status = cWriter.setStringField(JOIN_REP::SERVER_FIELD, m_cRepAddress);
	if (0 != status) {
		return 2;
	}

	ulongtype iStatus = m_pcMsgMgr->sendREP(*pcRxMsg, cWriter);
	if (0 != iStatus) {
		DEEP_LOG_ERROR(OTHER, "Failed to Reply to the JoinREQ\n");
		return 5;
	}

	DEEP_LOG_INFO(OTHER, "JOIN_REQ for Server: #%u (%s) - Channel UP.\n",
		      pcIn->m_iServerId,
		      pcIn->m_cServerAddr.c_str());

	return 0;
}

/**
 * processJoinRep
 * Process a Join Reply from the pending UP state peer. On success
 * will make the state UP.
 * @param pcRxMsg [in] - reference to the destreamed message.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::
processJoinRep(cxx::io::encodeProtocol::reader* pcRxMsg) {
	outBound*          pcOut    = 0;
	cxx::lang::String cName;

	/**
	 * Extract the server name and see if it matches a
	 * known IN Block.
	 */
	inttype status = pcRxMsg->getStringField(JOIN_REQ::SERVER_FIELD, cName);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Could not obtain the server name.\n");
		return 1;
	}

	pcOut = m_pcOutBoundPeersByName->get(&cName);
	if (0 == pcOut) {
		DEEP_LOG_ERROR(OTHER, "JOIN_REP for unknown server: %s\n",
			       cName.c_str());
		return 2;
	}

	status = pcRxMsg->getUint32Field(JOIN_REQ::SERVER_ID_FIELD,
					 pcOut->m_iServerId);
	if ((0 != status) || (0 == pcOut->m_iServerId)) {
		DEEP_LOG_ERROR(OTHER, "Could not obtain the server ID\n");
		return 3;
	}

	m_pcOutBoundPeers->put(pcOut->m_iServerId, pcOut);

	DEEP_LOG_INFO(OTHER, "JOIN_REP for Server: #%u (%s) - Channel UP.\n",
		      pcOut->m_iServerId,
		      cName.c_str());

	addAdj(pcOut->m_iServerId);

	return 0;
}

/**
 * encodeJoinReq
 * Encode the JOIN_REQ from the supplied ADJ
 * @param pcWriter [out] - the encoded fields are returned in the
 * writer block.
 * @return 0 in success.
 */
ulongtype cxx::fabric::RealtimeFabric::
encodeJoinReq(cxx::io::encodeProtocol::writer*& pcWriter) {

	uinttype iSize =
		cxx::io::encodeProtocol::writer::sizeOfUint32(m_iServerId)   + \
		cxx::io::encodeProtocol::writer::sizeOfString(m_cRepAddress);

	pcWriter = new cxx::io::encodeProtocol::writer(iSize, JOIN_REQ::MSG_ID);
	if (0 == pcWriter) {
		DEEP_LOG_ERROR(OTHER, "Unable to create a writer!\n");
		abort();
	}

	inttype status = pcWriter->setUint32Field(JOIN_REQ::SERVER_ID_FIELD, m_iServerId);
	if (0 != status) {
		return 1;
	}

	status = pcWriter->setStringField(JOIN_REQ::SERVER_FIELD, m_cRepAddress);
	if (0 != status) {
		return 2;
	}

	return 0;
}

/**
 * rejoinReq
 * Sends a REQuest to join the peer. This is done usually after a
 * prior disconnect with the peer.
 * @param cAddress - the remote address for the peer.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::
rejoinReq(outBound*          pcOut,
	  cxx::lang::String& cAddress) {
	DEEP_LOG_INFO(OTHER, "Reconnecting to %s\n", cAddress.c_str());

	/** Encode the JOIN_REQ */
	cxx::io::encodeProtocol::writer* pcWriter = 0;

	ulongtype status = encodeJoinReq(pcWriter);
	if ((0 != status) || (0 == pcWriter)) {
		return 1;
	}

	errno = 0;

	uinttype iStatus = m_pcMsgMgr->sendREQ(pcOut->m_reqSocket, *pcWriter);
	
	delete pcWriter; pcWriter = 0;

	if (0 != iStatus) {
		return 2;
	}

	return 0;
}

/**
 * disconnect
 * Called when we need to mitigate the lose of the address
 * that was connected on the outgoing socket.
 * @param cAddress [in] - the outgoing IP Address.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::
disconnect(cxx::lang::String& cAddress) {
	ulongtype status = 0;

	outBound* pcOut = m_pcOutBoundPeersByName->get(&cAddress);
	if (0 == pcOut) {
		return 1;
	}

	/** Flush this addresses server ID and mapping. */
	delAdj(pcOut->m_iServerId);

	m_pcOutBoundPeers->remove(pcOut->m_iServerId);

	DEEP_LOG_DEBUG(OTHER, "Dropped server: #%u (%s) from the topology.\n",
		       pcOut->m_iServerId,
		       cAddress.c_str());

	pcOut->m_iServerId = 0;

	status = rejoinReq(pcOut, cAddress);
	if (0 != status) {
		return 2;
	}

	return 0;
}

/**
 * sendOutTopoReq
 * Encode and send the TOPO_REQ to all UP peers.
 * @return 0 in success.
 */
ulongtype cxx::fabric::RealtimeFabric::
sendOutTopoReq() {
	if (true == m_pcOutBoundPeers->isEmpty()) {
		/** Nothing to do with no peers. */
		return 0;
	}

	cxx::lang::String cJson;
	int status = m_cTopology.renderAsJSON(cJson);
	if (0 != status) {
		return 1;
	}

	uinttype iSize =
		cxx::io::encodeProtocol::writer::sizeOfUint32(m_iServerId) + \
		cxx::io::encodeProtocol::writer::sizeOfString(cJson);

	cxx::io::encodeProtocol::writer cWriter(iSize, TOPO_REQ::MSG_ID);

	status = cWriter.setUint32Field(TOPO_REQ::SERVER_ID_FIELD, m_iServerId);
	if (0 != status) {
		return 2;
	}

	status = cWriter.setStringField(TOPO_REQ::MATRIX_FIELD, cJson);
	if (0 != status) {
		return 3;
	}

	/** Buffer is now encoded. Now send to all outBound Peers. */
	cxx::util::Set<uinttype>*      pcSet = m_pcOutBoundPeers->keySet();
	cxx::util::Iterator<uinttype>* pcIt  = pcSet->iterator();

	while (true == pcIt->hasNext()) {
		uinttype  iPeerId = pcIt->next();

		DEEP_LOG_DEBUG(OTHER, "[TX] TOPO_REQ -> #%u: %s\n", iPeerId, cJson.c_str());

		outBound* pcOut   = m_pcOutBoundPeers->get(iPeerId);

		cxx::io::encodeProtocol::writer cTopoReq(cWriter);

		if (0 != m_pcMsgMgr->sendREQ(pcOut->m_reqSocket, cTopoReq)) {
			DEEP_LOG_ERROR(OTHER, "Failed to send TOPO_REQ!\n");
			return 4;
		}

		DEEP_LOG_DEBUG(OTHER, "Updated TOPO sent to #%u\n", pcOut->m_iServerId);
	}

	delete pcIt;  pcIt  = 0;
	delete pcSet; pcSet = 0;

	return 0;
}

/**
 * processTopoReq
 * Processes a topology request message from a connected peer.
 * @param pcRxMsg[in] - the decoded message fields.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::
processTopoReq(cxx::io::encodeProtocol::reader* pcRxMsg) {

	uinttype  iServerId = 0;
	boolean   bChanged  = false;

	/**
	 * Extract the server ID
	 */
	inttype status = pcRxMsg->getUint32Field(TOPO_REQ::SERVER_ID_FIELD,
						 iServerId);
	if ((0 != status) || (0 == iServerId)) {
		DEEP_LOG_ERROR(OTHER, "Could not obtain the server ID\n");
		return 1;
	}

	cxx::lang::String cJson;

	status = pcRxMsg->getStringField(TOPO_REQ::MATRIX_FIELD, cJson);
	if (0 != status) {
		return 2;
	}

	DEEP_LOG_DEBUG(OTHER, "[RX] #%u -> TOPO_REQ: %s\n", iServerId, cJson.c_str());

	status = m_cTopology.processJSON(cJson, bChanged);
	if (0 != status) {
		return 3;
	}

	DEEP_LOG_DEBUG(OTHER, "[TX] TOPO_REP -> #%u, with changes=%s\n",
		       iServerId,
		       ((true == bChanged) ? "yes" : "no"));

	if (true == bChanged) {
		int status = m_cTopology.renderAsJSON(cJson);
		if (0 != status) {
			return 4;
		}
	}

	uinttype iSize =
		cxx::io::encodeProtocol::writer::sizeOfUint32(m_iServerId) + \
		cxx::io::encodeProtocol::writer::sizeOfString(cJson);

	cxx::io::encodeProtocol::writer cWriter(iSize, TOPO_REP::MSG_ID);

	status = cWriter.setUint32Field(TOPO_REP::SERVER_ID_FIELD, m_iServerId);
	if (0 != status) {
		return 5;
	}

	status = cWriter.setStringField(TOPO_REP::MATRIX_FIELD, cJson);
	if (0 != status) {
		return 6;
	}

	status = m_pcMsgMgr->sendREP(*pcRxMsg, cWriter);
	if (0 != status) {
		return 7;
	}

	return 0;
}

/**
 * processTopoRep
 * Processes a topology response from the remote PEER.
 * @param pcRxMsg[in] - the decoded message fields.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::
processTopoRep(cxx::io::encodeProtocol::reader* pcRxMsg) {
	uinttype  iServerId = 0;

	/**
	 * Extract the server ID
	 */
	inttype status = pcRxMsg->getUint32Field(TOPO_REP::SERVER_ID_FIELD,
						 iServerId);
	if ((0 != status) || (0 == iServerId)) {
		DEEP_LOG_ERROR(OTHER, "Could not obtain the server ID\n");
		return 1;
	}

	cxx::lang::String cJson;
	boolean           bChanged = false;

	status = pcRxMsg->getStringField(TOPO_REP::MATRIX_FIELD, cJson);
	if (0 != status) {
		return 2;
	}

	DEEP_LOG_DEBUG(OTHER, "[RX] #%u -> TOPO_REP: %s\n", iServerId, cJson.c_str());

	status = m_cTopology.processJSON(cJson, bChanged);
	if (0 != status) {
		return 3;
	}

	if (true == bChanged) {
		DEEP_LOG_DEBUG(OTHER, "TOPO REP from #%u triggered an update.\n", iServerId);
		sendOutTopoReq();
	}

	outBound* pcOut = m_pcOutBoundPeers->get(iServerId);
	if (0 == pcOut) {
		DEEP_LOG_WARN(OTHER, "No Outbound connection for #%u.\n",
			      iServerId);
		return 2;
	}

	return 0;
}

/**
 * addAdj
 * Add a peer adjacency and send out the updated matrix.
 * @param iPeerId - the peer ID to add.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::
addAdj(uinttype iPeerId) {
	int status = m_cTopology.addAdj(iPeerId);
	if (0 != status) {
		return 1;
	}

	if (0 != m_pcCassiShim->signalPeerChange(iPeerId, FabricCassiMessageQueue::PEER_CHANGE::UP)) {
		return 2;
	}

	if (0 != sendOutTopoReq()) {
		return 3;
	}

	return 0;
}

/**
 * delAdj
 * Remove the specified PeerID from the topology and send out an
 * update.
 * @param iPeerId - the peer ID to remove
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::
delAdj(uinttype iPeerId) {
	int status = m_cTopology.delAdj(iPeerId);
	if (0 != status) {
		return 1;
	}

	if (0 != m_pcCassiShim->signalPeerChange(iPeerId, FabricCassiMessageQueue::PEER_CHANGE::DOWN)) {
		return 2;
	}

	if (0 != sendOutTopoReq()) {
		return 3;
	}

	return 0;
}

/**
 * processFabricCommands
 * Handles any command requests from the FabricCassi thread.
 * @param pcMsg - reference to a fabric envelope message
 * @return 0 on success.
 */
FORCE_INLINE ulongtype cxx::fabric::RealtimeFabric::processFabricCommands(FabricEnvelope* pcMsg) {
	switch (pcMsg->getCmd()) {
	case REGISTER_MSG_ID_CMD:
		{
			uinttype iMsgId = reinterpret_cast<ulongtype>(pcMsg->getCmdValue());
			inttype  status = m_pcMsgMgr->registerMessageId(iMsgId, &m_cShimReqCb);
			if (0 != status) {
				throw RealtimeFabricException("Unable to register callback for shim command.");
			}
		}
		break;

	case ADD_PEER_MSG_ID_CMD:
		{
			peerCmdArgs* pcArgs = reinterpret_cast<peerCmdArgs*>(pcMsg->getCmdValue());
			if (0 != pcArgs) {
				inttype status = addPeer(pcArgs->m_cAddress, *(pcArgs->m_pcUUID));
				if (0 != m_pcCassiShim->signalAddPeerRep(status, pcArgs)) {
					throw RealtimeFabricException("Unable to REP to addPeer");
				}
			} else {
				throw RealtimeFabricException("Invalid command argument for addPeer.");
			}
		}
		break;

	case DEL_PEER_MSG_ID_CMD:
		{
			peerCmdArgs* pcArgs = reinterpret_cast<peerCmdArgs*>(pcMsg->getCmdValue());
			if (0 != pcArgs) {
				inttype status = delPeer(*(pcArgs->m_pcUUID));
				if (0 != m_pcCassiShim->signalDelPeerRep(status, pcArgs)) {
					throw RealtimeFabricException("Unable to delPeer");
				}
			} else {
				throw RealtimeFabricException("Invalid command argument for delPeer.");
			}
		}
		break;

	case GET_TOPO_MSG_ID_CMD:
		{
			JsonApiRequest* pcReq = reinterpret_cast<JsonApiRequest*>(pcMsg->getCmdValue());
			if (0 != pcReq) {
				json_object* pcJson = 0;
				if (0 != m_cTopology.renderAsJSON(pcJson)) {
					throw RealtimeFabricException("Unable to cTopology.renderAsJSON");
				}

				pcReq->m_pcRetData = pcJson;

				if (0 != m_pcCassiShim->signalGetTopoRep(pcReq)) {
					throw RealtimeFabricException("Unable to signal topology back.");
				}

			} else {
				throw RealtimeFabricException("Invalid command argument for getTopo.");
			}
		}
		break;

	case BUNDLED_MSG_ID_CMD:
		processBundleCommand(pcMsg);
		break;

	case LAST_MESSAGE_ID_CMD:
		sendLastRep();
		break;

	default:
		DEEP_LOG_WARN(OTHER, "CASSI: Unknown command envelope: %u\n", pcMsg->getCmd());
		break;
	}

	return 0;
}

/**
 * processBundleCommand
 * Process the bundle command, which will have a reference to a
 * ArrayList of messages to send to the destination fabric.
 * If messages contain sizes less than 1KiB then we need to bundle
 * them into larger containers for shipping.
 * @param pcMsg - the Bundle Message.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::processBundleCommand(FabricEnvelope* pcMsg) {
	FabricCassiMessageQueue::BundleList* pcBundle =
		reinterpret_cast<FabricCassiMessageQueue::BundleList*>(pcMsg->getCmdValue());

	if (0 == pcBundle) {
		throw RealtimeFabricException("Invalid command argument for BUNDLED.");
	}

	inttype iSize      = pcBundle->m_cBundle.size();
	ulongtype iAveSize = pcBundle->m_iTotalByteSize / iSize;

	uinttype  iPeerId = pcMsg->getPeerId();
	outBound* pcOut   = m_pcOutBoundPeers->get(iPeerId);
	if (0 == pcOut) {
		DEEP_LOG_WARN(OTHER, "Unknown Outbound Peer ID #%u. Message dropped.\n", iPeerId);
		delete pcBundle; pcBundle = 0;
		return -1;
	}

#if DEBUG_MESSAGE_BUNDLE
	DEEP_LOG_DEBUG(OTHER, "[#%d] TX: Fabric->Fabric #%d, First in Bundle. BundleSize=%d, AveBytes=%llu.\n",
		       m_iServerId, iPeerId, iSize, iAveSize);

	m_iTxBundleTimeStart = time::GetNanos<time::SteadyClock>();
#endif
	uinttype iPackFlags = cxx::io::encodeProtocol::header::FIRST_FLAG;
	cxx::io::MsgBundle::writer cPack;

	pcBundle->m_cBundle.setDeleteValue(false);

	for (inttype i=0; i<iSize; i++) {
		FabricEnvelope*                  pcQMsg   = pcBundle->m_cBundle.get(i);
		cxx::io::encodeProtocol::writer* pcWriter = pcQMsg;

		if (0 == pcQMsg) {
			DEEP_LOG_ERROR(OTHER, "Unexpected value for message reference\n");
			abort();
		}

		uinttype iFlags  = 0;

		if (0 != pcQMsg->getFlags(iFlags)) {
			throw RealtimeFabricException("Unable to get envelope flags.");
		}

		boolean   bLast = (iFlags & cxx::io::encodeProtocol::header::LAST_FLAG) ? true : false;
#if DEBUG_MESSAGE_BUNDLE
		if (true == bLast) {
			ulongtype iTime = time::GetNanos<time::SteadyClock>();
			iTime = iTime - m_iTxBundleTimeStart;
			m_iTxBundleTimeStart = 0;

			DEEP_LOG_DEBUG(OTHER, "[#%d] TX: Fabric->Fabric #%d, Last in Bundle. %f ms\n",
				       m_iServerId, iPeerId, (doubletype)iTime/time::MILLISECONDS);
		}
#endif

		if (MIN_MESSAGE_SIZE <= iAveSize) {
			if (0 != sendMsg(pcOut, pcQMsg, bLast)) {
				pcBundle->m_cBundle.setDeleteValue(true);
				delete pcBundle; pcBundle = 0;
				return -2;
			}
		} else {
			if (cPack.remainingSpace() >= pcQMsg->getSize()) {
				cPack.append(pcWriter);
				pcQMsg = 0;

			} else {
				cxx::io::encodeProtocol::writer* pcBundleMsg = 0;
				cPack.close(pcBundleMsg, PACK_BUNDLE_REQ::MSG_ID);

				pcBundleMsg->setFlags(iPackFlags);
				if (0 != sendMsg(pcOut, pcBundleMsg, bLast)) {
					pcBundle->m_cBundle.setDeleteValue(true);
					delete pcBundle; pcBundle = 0;
					return -3;
				}

				iPackFlags = 0;

				delete pcBundleMsg; pcBundleMsg = 0;

				cPack.open();
				cPack.append(pcWriter);
				pcQMsg = 0;
#if DEBUG_MESSAGE_BUNDLE
				DEEP_LOG_DEBUG(OTHER, "[#%d] TX: Fabric->Fabric #%d, Pack Sent.\n",
					       m_iServerId, iPeerId);
#endif
			}

			if (true == bLast) {
				cxx::io::encodeProtocol::writer* pcBundleMsg = 0;
				cPack.close(pcBundleMsg, PACK_BUNDLE_REQ::MSG_ID);

				pcBundleMsg->setFlags(iPackFlags & cxx::io::encodeProtocol::header::LAST_FLAG);

				if (0 != sendMsg(pcOut, pcBundleMsg, bLast)) {
					pcBundle->m_cBundle.setDeleteValue(true);
					delete pcBundle; pcBundle = 0;
					return -4;
				}
#if DEBUG_MESSAGE_BUNDLE
				DEEP_LOG_DEBUG(OTHER, "[#%d] TX: Fabric->Fabric #%d, Last Pack Sent.\n",
					       m_iServerId, iPeerId);
#endif
				delete pcBundleMsg; pcBundleMsg = 0;
			}
		}

		if (0 != pcQMsg) {
			delete pcQMsg; pcQMsg = 0;
		}
	}

	delete pcBundle; pcBundle = 0;

	return 0;
}

/**
 * sendMsg
 * Send the writer message to the destination fabric.
 * @param pcOut - Peer Context
 * @param pcMsg - the Message to send.
 * @param bLast - true if this is the last message in the bundle.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::sendMsg(outBound*                        pcOut,
					       cxx::io::encodeProtocol::writer* pcMsg,
					       boolean                          bLast) {
	ulongtype iStatus = m_pcMsgMgr->sendREQ(pcOut->m_reqSocket,
						*pcMsg,
						bLast);
	switch  (iStatus) {
		case 1:
		case 2:
			DEEP_LOG_WARN(OTHER, "Peer ID not UP yet. #%u. Message dropped.\n", pcOut->m_iServerId);
			break;
		case 0:
			break;
		default:
			throw RealtimeFabricException("Failed to send/queue envelope to peer.");
	}

	return iStatus;
}

/**
 * consumeWorkItems
 * Consumes all available work items in the Fabric's queue.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::consumeWorkItems() {
	while (true == m_pcCassiShim->fabricHasWork()) {
		FabricEnvelope* pcMsg = 0;
		boolean         bMore = false;
		
		inttype iStatus = m_pcCassiShim->dequeue(pcMsg, bMore);
		if (0 != iStatus) {
			throw RealtimeFabricException("Failed to consume work item.");
		}

		if (0 == pcMsg) {
			throw RealtimeFabricException("No message posted on the work queue.");
		}

		uinttype iFlags  = 0;

		if (0 != pcMsg->getFlags(iFlags)) {
			throw RealtimeFabricException("Unable to get envelope flags.");
		}

		uinttype iPeerId = pcMsg->getPeerId();

#if DEBUG_MESSAGE_FLOW
		uinttype iMsgId  = pcMsg->getMsgId();

		DEEP_LOG_DEBUG(OTHER, "[#%d] FabCassi->Fabric Msg: %x, PeerId: %u, Flags: %x\n",
			       m_iServerId, iMsgId, iPeerId, iFlags);
#endif
		iFlags = iFlags ^ cxx::io::encodeProtocol::header::VERSION;

		if ((iFlags & cxx::io::encodeProtocol::header::ONE_MSG) ==
		    cxx::io::encodeProtocol::header::ONE_MSG) {
			if (0 != pcMsg->getCmd()) {
				/** This is a command envelope */
				if (0 != processFabricCommands(pcMsg)) {
					throw RealtimeFabricException("Error processing a command message.\n");
				}

			} else {

				/** Bundling is handled as a command. We do one shot stuff here. */
				outBound* pcOut = m_pcOutBoundPeers->get(iPeerId);
				if (0 != pcOut) {
					ulongtype iStatus = m_pcMsgMgr->sendREQ(pcOut->m_reqSocket,
										*pcMsg,
										true);
					switch  (iStatus) {
					case 1:
					case 2:
						DEEP_LOG_WARN(OTHER, "Peer ID not UP yet. #%u. Message dropped.\n", iPeerId);
						break;
					case 0:
						break;
					default:
						throw RealtimeFabricException("Failed to send/queue envelope to peer.");
					}
				} else {
					DEEP_LOG_WARN(OTHER, "Unknown Outbound Peer ID #%u. Message dropped.\n", iPeerId);
				}
			}

		} else {
			throw RealtimeFabricException("Received a bundle message but not as a command!\n");
		}

		delete pcMsg; pcMsg = 0;
	}

	return 0;
}

/**
 * processShimReq
 * Post the reader object onto the shim queue.
 * @param pcRxMsg - the reader object message to post to Cassi
 * @return 0 on success, anything else is a error.
 */
ulongtype cxx::fabric::RealtimeFabric::
processShimReq(cxx::io::encodeProtocol::reader* pcRxMsg) {
	/** The FabricCassi Thread now owns this reader object. */
	boolean bLast  = pcRxMsg->isLast();
#if DEEP_ARRAY_BUNDLE
	boolean bFirst = pcRxMsg->isFirst();

	if ((true == bFirst) && (false == bLast)) {
		/** First message in the bundle. */
		m_iBundleCount = 1;
		m_iRxBundleTimeStart = time::GetNanos<time::SteadyClock>();

#if DEBUG_MESSAGE_BUNDLE
		DEEP_LOG_DEBUG(OTHER, "[#%d] RX: Fabric->Fabric, First in Bundle.\n",
			       m_iServerId);
#endif
		if (0 != m_pcBatch) {
			throw RealtimeFabricException("Batch already created!");
		}

		m_pcBatch = new cxx::util::ArrayList<cxx::io::encodeProtocol::reader*>(BUNDLE_ARRAY_INIT_CAP,
										       true);
		if (0 == m_pcBatch) {
			throw RealtimeFabricException("Unable to allocate a new batch list.");
		}

		m_pcBatch->add(pcRxMsg);

	} else if ((false == bFirst) && (true == bLast)) {
		/** Last Message in the bundle. */
		m_iBundleCount++;

#if DEBUG_MESSAGE_BUNDLE
		ulongtype iTime = time::GetNanos<time::SteadyClock>() - m_iRxBundleTimeStart;
		DEEP_LOG_DEBUG(OTHER, "[#%d] RX: Fabric->Fabric, Last in Bundle. Size=%d. %f ms\n",
			       m_iServerId, m_iBundleCount, (doubletype)iTime/time::MILLISECONDS);
#endif
		m_iRxBundleTimeStart = 0;

		m_pcBatch->add(pcRxMsg);
		m_pcCassiShim->signalBundleReq(m_pcBatch);
		m_pcBatch = 0;

	} else if ((false == bFirst) && (false == bLast)) {
		/** Middle of the bundle. */
		m_iBundleCount++;
		m_pcBatch->add(pcRxMsg);

	} else if ((true == bFirst) && (true == bLast)) {
		/** One Message, not a bundle. Just queue it. */
		if (0 != m_pcCassiShim->enqueue(pcRxMsg)) {
			DEEP_LOG_WARN(OTHER, "Issue enqueuing message to CASSI.\n");
			/** XXX: fix this to return NACK */
			return -1;
		}
	}
#else
	if (0 != m_pcCassiShim->enqueue(pcRxMsg)) {
		DEEP_LOG_WARN(OTHER, "Issue enqueuing message to CASSI.\n");
		/** XXX: fix this to return NACK */
		return -1;
	}
#endif

	if (true == bLast) {
		/** ACK Back with the SHIM_REP to the sender. */
		cxx::io::encodeProtocol::writer cWriter(0, FabricCassiMessageQueue::SHIM_REP::MSG_ID);
		if (0 != m_pcMsgMgr->sendREP(cWriter)) {
			DEEP_LOG_ERROR(OTHER, "Unable to send SHIM_REPly.\n");
			return -2;
		}
	}

	return 0;
}

/**
 * processShimRep
 * Post the reader object onto the shim queue. This is an ACK for the
 * previous REQuest messages sent to the server. We will just post
 * negative ACKS to the queue as this will be the back pressure
 * request is the remote server is overloaded.
 * @param pcRxMsg - the reader object message to post to Cassi
 * @return 0 on success, anything else is a error.
 */
ulongtype cxx::fabric::RealtimeFabric::
processShimRep(cxx::io::encodeProtocol::reader* pcRxMsg) {
	inttype iReply = 0;

	inttype iStatus = pcRxMsg->getInt32Field(FabricCassiMessageQueue::SHIM_REP::REPLY_ID_FIELD, iReply);
	if ((0 != iStatus) || (FabricCassiMessageQueue::SHIM_REP::REPLY_OK == iReply)) {
		delete pcRxMsg; pcRxMsg = 0;
		return 0;
	}

	/**
	 * We have a Negative SHIM REPly so forward to the CASSI
	 * Thread. The FabricCassi thread now owns this reader object.
	 */
	if (0 != m_pcCassiShim->enqueue(pcRxMsg)) {
		DEEP_LOG_WARN(OTHER, "Issue enqueuing message to CASSI.\n");
		return -1;
	}

	return 0;
}

/**
 * processPackReq
 * Process the packed bundle.
 * @param pcRxMsg - the reader object message to post to Cassi
 * @return 0 on success, anything else is a error.
 */
ulongtype cxx::fabric::RealtimeFabric::
processPackReq(cxx::io::encodeProtocol::reader* pcRxMsg) {
	const ubytetype* pBuffer = 0;
	uinttype         iSize   = 0;

	if (0 != pcRxMsg->getBundleBuffer(pBuffer, iSize)) {
		return -1;
	}

	cxx::io::MsgBundle::reader cPack(pBuffer, iSize);

	cxx::io::encodeProtocol::reader* pcMsg = 0;

	for (pcMsg = cPack.getNextMsg();
	       0 != pcMsg;
	       pcMsg = cPack.getNextMsg()) {

		processShimReq(pcMsg);
	}

	return 0;
}

/**
 * sendLastRep
 * Signal back to the peer fabric we received the last message.
 * @return 0 on success.
 */
ulongtype cxx::fabric::RealtimeFabric::
sendLastRep(void) {
	return 0;
}

/** JOIN_REQ_CB */
FORCE_INLINE uinttype cxx::fabric::RealtimeFabric::
JOIN_REQ_CB::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
		      voidptr                          sendId) {
	m_pcParent->processJoinReq(pcRxMsg);
	delete pcRxMsg; pcRxMsg = 0;
	return 0;
}

/** JOIN_REP_CB */
FORCE_INLINE uinttype cxx::fabric::RealtimeFabric::
JOIN_REP_CB::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
		      voidptr                          sendId) {
	m_pcParent->processJoinRep(pcRxMsg);
	delete pcRxMsg; pcRxMsg = 0;
	return 0;
}

/** TOPO_REQ_CB */
FORCE_INLINE uinttype cxx::fabric::RealtimeFabric::
TOPO_REQ_CB::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
		      voidptr                          sendId) {
	m_pcParent->processTopoReq(pcRxMsg);
	delete pcRxMsg; pcRxMsg = 0;
	return 0;
}

/** TOPO_REP_CB */
FORCE_INLINE uinttype cxx::fabric::RealtimeFabric::
TOPO_REP_CB::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
		      voidptr                          sendId) {
	m_pcParent->processTopoRep(pcRxMsg);
	delete pcRxMsg; pcRxMsg = 0;
	return 0;
}

/** SHIM_REQ_CB */
FORCE_INLINE uinttype cxx::fabric::RealtimeFabric::
SHIM_REQ_CB::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
		      voidptr                          sendId) {
	m_pcParent->processShimReq(pcRxMsg);
	return 1;
}

/** SHIM_REP_CB */
FORCE_INLINE uinttype cxx::fabric::RealtimeFabric::
SHIM_REP_CB::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
		      voidptr                          sendId) {
	m_pcParent->processShimRep(pcRxMsg);
	return 1;
}

/** PACK_BUNDLE_CB */
FORCE_INLINE uinttype cxx::fabric::RealtimeFabric::
PACK_BUNDLE_CB::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
			 voidptr                          sendId) {
	m_pcParent->processPackReq(pcRxMsg);
	return 1;
}

/** msgMgrNotifications */

/**
 * msgMgrNotifications
 * Constructor
 * @param pcParent - the RealtimeFabric instance
 */
FORCE_INLINE cxx::fabric::RealtimeFabric::
msgMgrNotifications::msgMgrNotifications(RealtimeFabric* pcParent) :
	m_pcParent(pcParent) {
}

/**
 * ~msgMgrNotifications
 * Destructor
 */
FORCE_INLINE cxx::fabric::RealtimeFabric::
msgMgrNotifications::~msgMgrNotifications() {
	m_pcParent = 0;
}

/**
 * socketAccept
 * The indicated server/REP socket accepted a connection
 * from the supplied IP Address.
 * @param socket - The MsgMgr socket instance
 * @param cPeerAddress - the peers IP address tcp://x.x.x.x:y
 */
FORCE_INLINE void cxx::fabric::RealtimeFabric::
msgMgrNotifications::socketAccept(voidptr            socket,
				  cxx::lang::String& cPeerAddress) {
	DEEP_LOG_DEBUG(OTHER, "socketAccept(%p) <- %s\n", socket, cPeerAddress.c_str());
}

/**
 * socketConnected
 * The indicated socket has connected to its peer.
 * @param socket - The MsgMgr socket instance
 * @param cPeerAddress - the peers IP address tcp://x.x.x.x:y
 */
FORCE_INLINE void cxx::fabric::RealtimeFabric::
msgMgrNotifications::socketConnected(voidptr            socket,
				     cxx::lang::String& cPeerAddress) {
	DEEP_LOG_DEBUG(OTHER, "socketConnected(%p) -> %s\n", socket, cPeerAddress.c_str());
}

/**
 * socketDisconnected
 * The indicated socket has disconnected from its peer.
 * @param socket - The MsgMgr socket instance
 * @param cPeerAddress - the peers IP address tcp://x.x.x.x:y
 */
FORCE_INLINE void cxx::fabric::RealtimeFabric::
msgMgrNotifications::socketDisconnected(voidptr            socket,
					cxx::lang::String& cPeerAddress) {
	DEEP_LOG_DEBUG(OTHER, "socketDisconnected(%p) -> %s\n", socket, cPeerAddress.c_str());

	m_pcParent->disconnect(cPeerAddress);
}

/**
 * overloadWarningRecv
 * Called to handle message receive overload.
 * @param iTimeLimit - number of millis blocked receiving messages.
 */
FORCE_INLINE void cxx::fabric::RealtimeFabric::
msgMgrNotifications::overloadWarningRecv(ulongtype iTimeLimit) {
	DEEP_LOG_WARN(OTHER, "overload on MSG RX: Time:%llu.\n",
		      iTimeLimit);
}

/**
 * overloadWarningSend
 * Called to handle message receive overload.
 * @param iTimeLimit - number of millis blocked sending messages.
 */
FORCE_INLINE void cxx::fabric::RealtimeFabric::
msgMgrNotifications::overloadWarningSend(ulongtype iTimeLimit) {
	DEEP_LOG_WARN(OTHER, "overload on MSG TX: Time:%llu.\n",
		      iTimeLimit);
}

/** cassiFabQCb */

cxx::fabric::RealtimeFabric::
cassiFabQCb::cassiFabQCb(RealtimeFabric* pcParent) : m_pcParent(pcParent) {
}

cxx::fabric::RealtimeFabric::
cassiFabQCb::~cassiFabQCb() {
	m_pcParent = 0;
}

/**
 * callback
 * Handle events on the CassiToFabric Queue.
 * @param iFd - the FD
 * @param iEvents - events on the FD.
 * @return 0 on success.
 */
inttype cxx::fabric::RealtimeFabric::
cassiFabQCb::callback(inttype iFd, shorttype iEvents) {
	if (0 != (ZMQ_POLLIN & iEvents)) {
		//DEEP_LOG_DEBUG(OTHER, "cassiFabQCb::callback(inttype iFd=%d, shorttype iEvents=%d)\n",
		//	       iFd,
		//	       iEvents);
		m_pcParent->consumeWorkItems();
	}

	return 0;
}
