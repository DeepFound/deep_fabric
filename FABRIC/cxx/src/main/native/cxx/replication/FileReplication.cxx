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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

#include "cxx/util/Logger.h"
#include "cxx/util/HashMap.cxx"
#include "com/deepis/communication/common/FabricEnvelope.h"
#include "cxx/lang/System.h"

#include "FileReplication.h"
#include "cxx/util/PrettyPrint.h"

#include "cxx/util/concurrent/atomic/AtomicInteger.h"

namespace {
	cxx::util::concurrent::atomic::AtomicInteger g_iInstances(0);
}

#include "cxx/util/Time.h"

//#define DEEP_DEBUG_PACKET_FLOW 1
#define DEEP_DELUGE_REPLICATE_FILE 0

using namespace cxx::lang;
using namespace cxx::util;
using namespace com::deepis::communication;

/**
 * getFileReplicationBridgeInstance
 * Creates an instance of the FileReplicationBridge.
 * @param pcCommon - reference to the FabricCassi message Q instance
 * @return reference to an instance of FileReplicationBridge.
 */
com::deepis::communication::fabricconnector::FabricCassiBridgeCb*
cxx::replication::getFileReplicationBridgeInstance(com::deepis::communication::fabricconnector::FabricCassiMessageQueue* pcCommon,
						   voidptr pCtx) {
	DEEP_LOG_DEBUG(OTHER, "Creating FileReplicationBridge.\n");

	return new cxx::replication::FileReplicationBridge(pcCommon, pCtx);
}

/**
 * Constructor
 * @param pcCommon - Reference to the MessageQueue
 */
cxx::replication::FileReplicationBridge::
FileReplicationBridge(com::deepis::communication::fabricconnector::FabricCassiMessageQueue* pcCommon,
		      voidptr pCtx) :
	com::deepis::communication::fabricconnector::FabricCassiBridgeCb(pcCommon) {

	if (0 != pCtx) {
		cxx::lang::String* pcPaths = reinterpret_cast<cxx::lang::String* >(pCtx);

		m_cPaths[0] = pcPaths[0];
		m_cPaths[1] = pcPaths[1];
	}
}

/**
 * ~FileReplicationBridge
 * Destructor
 */
cxx::replication::FileReplicationBridge::
~FileReplicationBridge() {
	//shutdown();
}

/**
 * remoteOverloaded
 * The indicated remote endpoint is experiencing overload conditions.
 * @param pcMsg - message containing the overload information.
 */
void cxx::replication::FileReplicationBridge::remoteOverloaded(cxx::io::encodeProtocol::reader* pcMsg) {
	DEEP_LOG_DEBUG(OTHER, "FileReplicationBridge::remoteOverloaded\n");
}

/**
 * shutdown
 * FabricCassi thread is signaling a shutdown of service.
 */
void cxx::replication::FileReplicationBridge::shutdown() {
	DEEP_LOG_DEBUG(OTHER, "FileReplicationBridge::shutdown started.\n");
#if 0
	cxx::util::Iterator<cxx::util::UUID* >* pcIt = m_cTxBlock.keySet()->iterator();
	while (true == pcIt->hasNext()) {
		cxx::util::UUID* pcId  = pcIt->next();
		FileTxBlock*     pcOut = m_cTxBlock.get(pcId);

		if (0 != pcOut) {
			delete pcOut; pcOut = 0;
		}

		pcIt->remove();
		delete pcId; pcId = 0;
	}

	cxx::util::Iterator<cxx::util::UUID* >* pcIt2 = m_cRxBlock.keySet()->iterator();
	while (true == pcIt2->hasNext()) {
		cxx::util::UUID* pcId = pcIt2->next();
		FileRxBlock*     pcIn = m_cRxBlock.get(pcId);

		if (0 != pcIn) {
			delete pcIn; pcIn = 0;
		}

		pcIt->remove();
		delete pcId; pcId = 0;
	}
#endif
	DEEP_LOG_DEBUG(OTHER, "FileReplicationBridge::shutdown complete.\n");
}

/**
 * peerStateChanged
 * The Peer has new state information. All state can be fetched from
 * the message. Do not delete this message. Copy all needed info.
 * @param pcMsg - the state change information for the indicated peer.
 */
void cxx::replication::FileReplicationBridge::peerStateChanged(cxx::io::encodeProtocol::reader* pcMsg) {
	DEEP_LOG_DEBUG(OTHER, "FileReplicationBridge::peerStateChanged\n");

	if (0 == pcMsg) {
		return;
	}
	inttype  iStatus   = 0;
	uinttype iServerId = 0;

	iStatus = pcMsg->getUint32Field(fabricconnector::FabricCassiMessageQueue::PEER_CHANGE::SERVER_ID_FIELD, iServerId);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "Unable to obtain ServerId from peerStateChange.\n");
		return;
	}

	inttype iState = fabricconnector::FabricCassiMessageQueue::PEER_CHANGE::DOWN;

	iStatus = pcMsg->getInt32Field(fabricconnector::FabricCassiMessageQueue::PEER_CHANGE::STATE_FIELD, iState);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "Unable to obtain State Field from peerStateChange.\n");
		return;
	}

	if (fabricconnector::FabricCassiMessageQueue::PEER_CHANGE::UP == iState) {
		if (0 == m_cPaths[0].size()) {
			if (1 == m_pcCommon->getServerId()) {
				DEEP_LOG_ERROR(OTHER, "No source filename provided.\n");
				exit(0);
			} else {
				return;
			}
		}

		cxx::lang::String cDst;

		if (1 == m_pcCommon->getServerId()) {
			if (0 == m_cPaths[1].compare("<default>")) {
				std::stringstream ss;
				ss << "/tmp/slave-" << iServerId << ".bin";
				cDst = ss.str();
			} else {
				cDst = m_cPaths[1];
			}
		}

		remove(cDst);

		if (0 != requestFileReplication(iServerId, m_cPaths[0], cDst)) {
			DEEP_LOG_ERROR(OTHER, "Failed to make request.\n");
			abort();
		}
	}
}

/**
 * dispatch
 * A bridge defined message has been forwarded to this bridge
 * dispatcher for processing.
 * @param pcMsg - the message to dispatch upon.
 * @return TBD. 0 means success for now.
 */
inttype cxx::replication::FileReplicationBridge::dispatch(cxx::io::encodeProtocol::reader* pcMsg) {
	uinttype iMsgId = pcMsg->getMsgId();

#if DEEP_DEBUG_PACKET_FLOW
	DEEP_LOG_DEBUG(OTHER, "FileReplicationBridge::dispatch MsgId=0x%x\n", iMsgId);
#endif

	inttype iStatus = 0;

	switch (iMsgId) {
		case FILE_TX_REQ::MSG_ID:
			iStatus = processFileTxReq(pcMsg);
			break;
		case FILE_TX_REP::MSG_ID:
			iStatus = processFileTxRep(pcMsg);
			break;
		case FILE_BLOCK_REQ::MSG_ID:
			iStatus = processFileBlockReq(pcMsg);
			break;
		case FILE_BLOCK_REP::MSG_ID:
			iStatus = processFileBlockRep(pcMsg);
			break;
		default:
			break;
	}

	return iStatus;
}

/**
 * allMessageIds
 * Returns a list of all bridge messages that are to be forwarded to
 * this bridge.
 * @return 0 for nothing or a zero terminate uinttype array of all
 * messageIDs to register for forwarding.
 */
const uinttype* cxx::replication::FileReplicationBridge::allMessageIds(void) {
	static uinttype iRetVal[] = {
		cxx::replication::FileReplicationBridge::FILE_TX_REQ::MSG_ID,
		cxx::replication::FileReplicationBridge::FILE_TX_REP::MSG_ID,
		cxx::replication::FileReplicationBridge::FILE_BLOCK_REQ::MSG_ID,
		cxx::replication::FileReplicationBridge::FILE_BLOCK_REP::MSG_ID,
		0
	};

	return iRetVal;
}

/**
 * requestFileReplication
 * Calculates the filesize, sha1sum and renders the request message.
 * We add the transaction to the UUID transaction list awaiting for
 * the reply.
 * @param iPeerId - destination peer ID for the request.
 * @param cSrc - path name for the file to transfer
 * @param cDst - path name to call it at the destination.
 * @return 0 on success.
 */
inttype cxx::replication::FileReplicationBridge::
requestFileReplication(uinttype           iPeerId,
		       cxx::lang::String& cSrc,
		       cxx::lang::String& cDst) {
	struct stat cBuf;
	const char* pcName = cSrc.c_str();
	if (0 != stat(pcName, &cBuf)) {
		DEEP_LOG_WARN(OTHER, "Error obtaining file info for %s.\n", pcName);
		exit(-1);
	}

	cxx::util::UUID* pcUUID = cxx::util::UUID::randomUUID();
	uinttype iSize =
		cxx::io::encodeProtocol::writer::sizeOfUint32(m_pcCommon->getServerId()) + \
		cxx::io::encodeProtocol::writer::sizeOfString(cDst) + \
		cxx::io::encodeProtocol::writer::sizeOfUint64(cBuf.st_size) + \
		cxx::io::encodeProtocol::writer::sizeOfInt64(pcUUID->getLeastSignificantBits()) + \
		cxx::io::encodeProtocol::writer::sizeOfInt64(pcUUID->getMostSignificantBits());

	com::deepis::communication::common::FabricEnvelope* pcReq =
		new com::deepis::communication::common::FabricEnvelope(iSize, FILE_TX_REQ::MSG_ID, iPeerId);

	pcReq->setUint32Field(FILE_TX_REQ::SERVER_ID_FIELD, m_pcCommon->getServerId());
	pcReq->setStringField(FILE_TX_REQ::FILE_PATH_FIELD, cDst);
	pcReq->setUint64Field(FILE_TX_REQ::FILE_SIZE_FIELD, cBuf.st_size);
	pcReq->setInt64Field(FILE_TX_REQ::FILE_UUIDLO_FIELD, pcUUID->getLeastSignificantBits());
	pcReq->setInt64Field(FILE_TX_REQ::FILE_UUIDHI_FIELD, pcUUID->getMostSignificantBits());

	pcReq->setFlags(cxx::io::encodeProtocol::header::ONE_MSG);

#if DEEP_DEBUG_PACKET_FLOW
	DEEP_LOG_DEBUG(OTHER, "To #%u, File: %s->%s (%s), Bytes: %ld, Blocks: %ld, BlockSize: %ld.\n",
		       iPeerId,
		       pcName,
		       cDst.c_str(),
		       pcUUID->toString().c_str(),
		       cBuf.st_size,
		       cBuf.st_blocks,
		       cBuf.st_blksize);
#endif

	if (0 != send(pcReq)) {
		DEEP_LOG_WARN(OTHER, "Unable to send FILE_TX_REQ message.\n");
		delete pcReq; pcReq = 0;
	}

	FileTxBlock* pcOut = new FileTxBlock;
	pcOut->m_iPeerId     = iPeerId;
	pcOut->m_cFileName   = cSrc;

	m_cTxBlock.put(pcUUID, pcOut);

	return 0;
}

/**
 * processFileTxReq
 * Process the request to send a file to this Fabric. If allowed,
 * create a FileRxBlock to track and send a reply message back to ack
 * this request.
 * @param pcMsg - The REQuest message
 * @return 0 on success.
 */
inttype cxx::replication::FileReplicationBridge::
processFileTxReq(cxx::io::encodeProtocol::reader* pcMsg) {
	FileRxBlock* pcIn = new FileRxBlock;

	pcMsg->getUint32Field(FILE_TX_REQ::SERVER_ID_FIELD, pcIn->m_iPeerId);

	pcMsg->getStringField(FILE_TX_REQ::FILE_PATH_FIELD, pcIn->m_cFileName);

	longtype iLo = 0;
	longtype iHi = 0;

	pcMsg->getInt64Field(FILE_TX_REQ::FILE_UUIDLO_FIELD, iLo);
	pcMsg->getInt64Field(FILE_TX_REQ::FILE_UUIDHI_FIELD, iHi);

	cxx::util::UUID* pcUUID = new cxx::util::UUID(iHi, iLo);
	m_cRxBlock.put(pcUUID, pcIn);

#if DEEP_DEBUG_PACKET_FLOW
	DEEP_LOG_DEBUG(OTHER, "Handling Tx Request from #%u, for: %s (%s)\n",
		       pcIn->m_iPeerId,
		       pcIn->m_cFileName.c_str(),
		       pcUUID->toString().c_str());
#endif
	uinttype iSize =
		cxx::io::encodeProtocol::writer::sizeOfUint32(m_pcCommon->getServerId()) + \
		cxx::io::encodeProtocol::writer::sizeOfInt64(iLo) + \
		cxx::io::encodeProtocol::writer::sizeOfInt64(iHi) + \
		cxx::io::encodeProtocol::writer::sizeOfInt32((inttype)FILE_TX_REP::OK_TO_SEND);

	com::deepis::communication::common::FabricEnvelope* pcRep =
		new com::deepis::communication::common::FabricEnvelope(iSize, FILE_TX_REP::MSG_ID, pcIn->m_iPeerId);

	pcRep->setUint32Field(FILE_TX_REP::SERVER_ID_FIELD, m_pcCommon->getServerId());
	pcRep->setInt64Field(FILE_TX_REP::FILE_UUIDLO_FIELD, iLo);
	pcRep->setInt64Field(FILE_TX_REP::FILE_UUIDHI_FIELD, iHi);
	pcRep->setInt32Field(FILE_TX_REP::FILE_REPLY_FIELD, (inttype)FILE_TX_REP::OK_TO_SEND);

	pcRep->setFlags(cxx::io::encodeProtocol::header::ONE_MSG);

	if (0 != send(pcRep)) {
		DEEP_LOG_WARN(OTHER, "Unable to send FILE_TX_REP message.\n");
		delete pcRep; pcRep = 0;
	}

	return 0;
}

/**
 * processFileTxRep
 * Process the request to send a file to this Fabric. If allowed,
 * create a FileRxBlock to track and send a reply message back to ack
 * this request.
 * @param pcMsg - The REQuest message
 * @return 0 on success.
 */
inttype cxx::replication::FileReplicationBridge::
processFileTxRep(cxx::io::encodeProtocol::reader* pcMsg) {

	uinttype iPeerId = 0;

	inttype iStatus = pcMsg->getUint32Field(FILE_TX_REP::SERVER_ID_FIELD, iPeerId);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "No Server ID found in the REPly mesage\n");
		return -1;
	}

	longtype iLo = 0;
	longtype iHi = 0;

	iStatus = pcMsg->getInt64Field(FILE_TX_REP::FILE_UUIDLO_FIELD, iLo);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "No UUID LSB found from REPLY #%u\n", iPeerId);
		return -2;
	}

	iStatus = pcMsg->getInt64Field(FILE_TX_REP::FILE_UUIDHI_FIELD, iHi);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "No UUID MSB found from REPLY #%u\n", iPeerId);
		return -3;
	}

	cxx::util::UUID cUUID(iHi, iLo);

	inttype iReply = -1;

	iStatus = pcMsg->getInt32Field(FILE_TX_REP::FILE_REPLY_FIELD, iReply);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "No REPLY indication found from REPLY #%u (%s)\n",
			      iPeerId,
			      cUUID.toString().c_str());
		return -4;
	}

	FileTxBlock* pcOut = m_cTxBlock.get(&cUUID);
	if (0 == pcOut) {
		DEEP_LOG_WARN(OTHER, "UUID (%s) could not be located.\n", cUUID.toString().c_str());
		return -1;
	}

#if DEEP_DEBUG_PACKET_FLOW
	DEEP_LOG_DEBUG(OTHER, "Handling Tx Response from #%u, for: %s (%s): REPLY=%d\n",
		       iPeerId,
		       pcOut->m_cFileName.c_str(),
		       cUUID.toString().c_str(),
		       iReply);
#endif

	struct stat cBuf;
	const char* pcName = pcOut->m_cFileName.c_str();

	errno = 0;

	if (0 != stat(pcName, &cBuf)) {
		DEEP_LOG_WARN(OTHER, "Error obtaining file info for %s, errno=%d.\n", pcName, errno);
		return -2;
	}

#if STAT_SIZES
	pcOut->m_iBlockSize  = cBuf.st_blksize;
	pcOut->m_iBlocksLeft = cBuf.st_blocks;
	pcOut->m_iBundleSize = FILE_BUNDLE_SIZE / pcOut->m_iBlockSize;
#else
	pcOut->m_iBlockSize  = FILE_BLOCK_SIZE;
	pcOut->m_iBlocksLeft = cBuf.st_blocks;
	pcOut->m_iBundleSize = FILE_BUNDLE_SIZE;
#endif

	pcOut->m_iFd         = open(pcOut->m_cFileName, O_RDONLY, 0);

	if (-1 == pcOut->m_iFd) {
		DEEP_LOG_WARN(OTHER, "Unable to open file: %s\n", cUUID.toString().c_str());
		exit(-1);
	}

	inttype iInst = g_iInstances.incrementAndGet();

	pcOut->m_iStart = System::currentTimeMillis();

	DEEP_LOG_INFO(OTHER, "File TX Started from #%u -> #%u, for: %s (%s) [Open #%d]\n",
		      m_pcCommon->getServerId(),
		      pcOut->m_iPeerId,
		      pcOut->m_cFileName.c_str(),
		      cUUID.toString().c_str(),
		      iInst);

	boolean bFinished = false;
#if DEEP_DELUGE_REPLICATE_FILE
	while (false == bFinished) {
#endif
		iStatus = renderFileBlockBundle(iLo, iHi, pcOut, bFinished);
		if (0 != iStatus) {
			return iStatus;
		}
#if DEEP_DELUGE_REPLICATE_FILE
	}
#endif
	return iStatus;
}

/**
 * renderFileBlockBundle
 * This creates a bundle of file blocks to created and sent to the
 * target peer.
 * @param iLo - UUID LSB
 * @param iHi - UUID MSB
 * @param pcOut - FileTxBlock for the target peer.
 * @param bFinished [out] - if true we finished reading the file.
 * @return 0
 */
inttype cxx::replication::FileReplicationBridge::
renderFileBlockBundle(longtype iLo, longtype iHi, FileTxBlock* pcOut, boolean& bFinished) {
	if (0 == pcOut->m_iFd) {
		bFinished = true;
		return 0;
	}

	uinttype iFlags = cxx::io::encodeProtocol::header::FIRST_FLAG;

	for (longtype i=pcOut->m_iBundleSize; i>=1; i--) {
		ssize_t iReadSize = 0;

		while (true) {
			errno = 0;

			iReadSize = read(pcOut->m_iFd, pcOut->m_iBuffer, pcOut->m_iBlockSize);
			if (-1 == iReadSize) {
				if (EAGAIN != errno) {
					DEEP_LOG_WARN(OTHER, "Error reading the file (%s) errno=%d\n",
						      pcOut->m_cFileName.c_str(),
						      errno);
					abort();
				} else {
#if DEEP_DEBUG_PACKET_FLOW
					DEEP_LOG_DEBUG(OTHER, "(%lld) EAGAIN on file (%s)\n",
						       i,
						       pcOut->m_cFileName.c_str());
#endif
				}

			} else {
				break;
			}
		}

		pcOut->m_iBytesTx += iReadSize;
		ulongtype iTxTime = time::GetNanos<time::SteadyClock>();

		uinttype iSize =
			cxx::io::encodeProtocol::writer::sizeOfUint32(m_pcCommon->getServerId()) + \
			cxx::io::encodeProtocol::writer::sizeOfInt64(iLo) + \
			cxx::io::encodeProtocol::writer::sizeOfInt64(iHi) + \
			cxx::io::encodeProtocol::writer::sizeOfRaw(iReadSize) + \
			cxx::io::encodeProtocol::writer::sizeOfUint64(iTxTime);

		if ((iReadSize == 0) || (1 == i)) {
			iFlags |= cxx::io::encodeProtocol::header::LAST_FLAG;
			i = 1;

			if (iReadSize == 0) {
				close(pcOut->m_iFd);

				pcOut->m_iFd = 0;

				inttype iInst = g_iInstances.decrementAndGet();

				DEEP_LOG_INFO(OTHER, "File TX Complete from #%u -> #%u, for: %s [Open #%d]\n",
					      m_pcCommon->getServerId(),
					      pcOut->m_iPeerId,
					      pcOut->m_cFileName.c_str(),
					      iInst);

				long long iEnd = System::currentTimeMillis();

				float iDuration = (iEnd - pcOut->m_iStart);
				float iRate     = (pcOut->m_iBytesTx / iDuration) * 1000;

				cxx::lang::String cPrettyRate;
				prettyprint::Bytes<float>(iRate, cPrettyRate);

				cxx::lang::String cPrettySize;
				prettyprint::Bytes<ulongtype>(pcOut->m_iBytesTx, cPrettySize);

				DEEP_LOG_INFO(OTHER, "TX Rate: %s/sec : File Size=%s, Total Time=%f sec\n",
					      cPrettyRate.c_str(),
					      cPrettySize.c_str(),
					      iDuration/1000);
			}
		}

		com::deepis::communication::common::FabricEnvelope* pcMsg =
			new com::deepis::communication::common::FabricEnvelope(iSize, FILE_BLOCK_REQ::MSG_ID, pcOut->m_iPeerId);

		pcMsg->setUint32Field(FILE_BLOCK_REQ::SERVER_ID_FIELD, m_pcCommon->getServerId());
		pcMsg->setInt64Field(FILE_BLOCK_REQ::FILE_UUIDLO_FIELD, iLo);
		pcMsg->setInt64Field(FILE_BLOCK_REQ::FILE_UUIDHI_FIELD, iHi);
		pcMsg->setRawField(FILE_BLOCK_REQ::BLOCK_FIELD, pcOut->m_iBuffer, iReadSize);
		pcMsg->setUint64Field(FILE_BLOCK_REQ::TIME_STAMP_FIELD, iTxTime);
		pcMsg->setFlags(iFlags);

		if (0 != send(pcMsg)) {
			DEEP_LOG_WARN(OTHER, "Unable to send FILE_BLOCK_REQ message.\n");
			delete pcMsg; pcMsg = 0;
			return -1;
		}

		iFlags = 0;
	}

	return 0;
}

/**
 * processFileBlockReq
 * Process a file block and write any data to the specified file
 * handle via the UUID.
 * @param pcMsg - the File Block.
 * @return 0 on success.
 */
inttype cxx::replication::FileReplicationBridge::
processFileBlockReq(cxx::io::encodeProtocol::reader* pcMsg) {
	longtype iLo     = 0;
	longtype iHi     = 0;

	inttype  iStatus = pcMsg->getInt64Field(FILE_BLOCK_REQ::FILE_UUIDLO_FIELD, iLo);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "Unable to extract UUID LO from the message\n");
		return -1;
	}

	iStatus = pcMsg->getInt64Field(FILE_BLOCK_REQ::FILE_UUIDHI_FIELD, iHi);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "Unable to extract UUID HI from the message\n");
		return -2;
	}

	cxx::util::UUID cUUID(iHi, iLo);

	FileRxBlock* pcIn = m_cRxBlock.get(&cUUID);
	if (0 == pcIn) {
		DEEP_LOG_WARN(OTHER, "UUID (%s) could not be located.\n", cUUID.toString().c_str());
		return -3;
	}

	if (true == pcMsg->isFirst()) {
		errno = 0;

		if (-1 == pcIn->m_iFd) {
			pcIn->m_iFd = open(pcIn->m_cFileName, O_WRONLY | O_CREAT, 0644);

			if (-1 == pcIn->m_iFd) {
				DEEP_LOG_ERROR(OTHER, "Unable to open the file for writing. errno=%d\n", errno);
				exit(-2);
			}

			inttype iInst = g_iInstances.incrementAndGet();

			DEEP_LOG_INFO(OTHER, "File RX Started from #%u -> #%u, for: %s (%s) [Open #%d]\n",
				      pcIn->m_iPeerId,
				      m_pcCommon->getServerId(),
				      pcIn->m_cFileName.c_str(),
				      cUUID.toString().c_str(),
				      iInst);

			pcIn->m_iStart = System::currentTimeMillis();

		} else {
			ulongtype iTxTime = 0;
			iStatus = pcMsg->getUint64Field(FILE_BLOCK_REQ::TIME_STAMP_FIELD, iTxTime);

			ulongtype iRxTime = time::GetNanos<time::SteadyClock>();
			DEEP_LOG_INFO(OTHER, "[RX] Bundle first: %llu, %llu latency %f ms\n",
				      iTxTime,
				      iRxTime,
				      (doubletype)(iRxTime-iTxTime)/time::MILLISECONDS);

		}

	} else if  (pcMsg->isLast()) {
		/**
		 * Now send the REP block message to get more file
		 * blocks.
		 */
		uinttype iSize =
			cxx::io::encodeProtocol::writer::sizeOfUint32(m_pcCommon->getServerId()) + \
			cxx::io::encodeProtocol::writer::sizeOfInt64(iLo) + \
			cxx::io::encodeProtocol::writer::sizeOfInt64(iHi);

		com::deepis::communication::common::FabricEnvelope* pcMsg =
			new com::deepis::communication::common::FabricEnvelope(iSize, FILE_BLOCK_REP::MSG_ID, pcIn->m_iPeerId);

		pcMsg->setUint32Field(FILE_BLOCK_REP::SERVER_ID_FIELD, m_pcCommon->getServerId());
		pcMsg->setInt64Field(FILE_BLOCK_REP::FILE_UUIDLO_FIELD, iLo);
		pcMsg->setInt64Field(FILE_BLOCK_REP::FILE_UUIDHI_FIELD, iHi);

		pcMsg->setFlags(cxx::io::encodeProtocol::header::ONE_MSG);

		if (0 != send(pcMsg)) {
			DEEP_LOG_WARN(OTHER, "Unable to send FILE_TX_REQ message.\n");
			delete pcMsg; pcMsg = 0;
		}
	}

	const ubytetype* pcRaw = 0;
	uinttype         iSize = 0;
	
	iStatus = pcMsg->getRawFieldRef(FILE_BLOCK_REQ::BLOCK_FIELD, pcRaw, iSize);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "Missing Raw field in the File Block.\n");
		return -4;
	}

#if DEEP_DEBUG_PACKET_FLOW
	DEEP_LOG_DEBUG(OTHER, "Handling File Block %d Bytes from #%u, for: %s (%s)\n",
		       iSize,
		       pcIn->m_iPeerId,
		       pcIn->m_cFileName.c_str(),
		       cUUID.toString().c_str());
#endif
	if (0 == iSize) {
		/** This is the end of file block. */
		close(pcIn->m_iFd);

		pcIn->m_iFd = 0;

		inttype iInst = g_iInstances.decrementAndGet();

		DEEP_LOG_INFO(OTHER, "File RX Complete from #%u -> #%u, for: %s (%s) [Open #%d]\n",
			      pcIn->m_iPeerId,
			      m_pcCommon->getServerId(),
			      pcIn->m_cFileName.c_str(),
			      cUUID.toString().c_str(),
			      iInst);

		long long iEnd = System::currentTimeMillis();

		float iDuration = (iEnd - pcIn->m_iStart);
		float iRate     = (pcIn->m_iBytesRx / iDuration) * 1000;

		cxx::lang::String cPrettyRate;
		prettyprint::Bytes<float>(iRate, cPrettyRate);

		cxx::lang::String cPrettySize;
		prettyprint::Bytes<ulongtype>(pcIn->m_iBytesRx, cPrettySize);

		DEEP_LOG_INFO(OTHER, "RX Rate: %s/sec : File Size=%s, Total Time=%f sec\n",
			      cPrettyRate.c_str(),
			      cPrettySize.c_str(),
			      iDuration/1000);

		if (0 >= iInst) {
			exit(0);
		}

		return 0;

	} else {
		pcIn->m_iBytesRx += iSize;
	}

	errno = 0;

	ssize_t iWritten = write(pcIn->m_iFd, pcRaw, iSize);
	if (-1 == iWritten) {
		DEEP_LOG_ERROR(OTHER, "Failed to write the raw data to file. errno=%d\n", errno);
		abort();
	}

	return 0;
}

/**
 * processFileBlockRep
 * Process a file block and write any data to the specified file
 * handle via the UUID.
 * @param pcMsg - the File Block.
 * @return 0 on success.
 */
inttype cxx::replication::FileReplicationBridge::
processFileBlockRep(cxx::io::encodeProtocol::reader* pcMsg) {
#if !DEEP_DELUGE_REPLICATE_FILE
	longtype iLo = 0;
	longtype iHi = 0;
	inttype iStatus = pcMsg->getInt64Field(FILE_BLOCK_REP::FILE_UUIDLO_FIELD, iLo);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "Unable to extract UUID LO from the message\n");
		return -1;
	}

	iStatus = pcMsg->getInt64Field(FILE_BLOCK_REP::FILE_UUIDHI_FIELD, iHi);
	if (0 != iStatus) {
		DEEP_LOG_WARN(OTHER, "Unable to extract UUID HI from the message\n");
		return -2;
	}

	cxx::util::UUID cUUID(iHi, iLo);

	FileTxBlock* pcOut = m_cTxBlock.get(&cUUID);
	if (0 == pcOut) {
		DEEP_LOG_WARN(OTHER, "UUID (%s) could not be located.\n", cUUID.toString().c_str());
		return -1;
	}

#if DEEP_DEBUG_PACKET_FLOW
	DEEP_LOG_DEBUG(OTHER, "Handling FILE BLOCK REPly from #%u, for: %s (%s): REPLY=%d\n",
		       pcOut->m_iPeerId,
		       pcOut->m_cFileName.c_str(),
		       cUUID.toString().c_str(),
		       0);
#endif

	boolean bFinished = false;

	iStatus = renderFileBlockBundle(iLo, iHi, pcOut, bFinished);

	return iStatus;
#endif
	return 0;
}
