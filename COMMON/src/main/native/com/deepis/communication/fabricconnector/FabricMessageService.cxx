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
#ifndef COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_FABRICMESSAGESERVICE_CXX_
#define COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_FABRICMESSAGESERVICE_CXX_

#include "com/deepis/communication/common/MsgIds.h"
#include "com/deepis/communication/common/ThreadPool.h"
#include "com/deepis/communication/fabricconnector/FabricMessageService.h"
#include "com/deepis/communication/fabricconnector/CassiServiceBridge.h"
#include "com/deepis/communication/fabricconnector/DataTransaction.h"
#include "cxx/io/EncodeProtocol.h"

#include "com/deepis/db/store/relative/util/InvalidException.h"
#include "com/deepis/db/store/relative/distributed/Serializer.h"

using namespace com::deepis::communication::common;
using namespace cxx::io;
using namespace com::deepis::communication::fabricconnector;
using namespace com::deepis::db::store::relative::util;
using namespace com::deepis::db::store::relative::distributed;

template<typename K>
const Comparator<longtype> FabricMessageService<K>::LONG_CMP;

template<typename K>
void FabricMessageService<K>::run() {
	while(m_lookForMessages == true) {
		if (m_messages.isEmpty() == false) {
			encodeProtocol::reader* msg = null;
			voidptr pMsg = null;

			m_messages.dequeue(pMsg);
			msg = (encodeProtocol::reader*) pMsg;
			dispatchMessage(msg);
		}

		if (m_messages.isEmpty() == true) {
			Thread::sleep(10);
		}
	}
}

template<typename K>
void FabricMessageService<K>::dispatchMessage(encodeProtocol::reader* message) {
	uinttype messageId = message->getMsgId();

	longtype threadId = 0;
	message->getInt64Field(Serializer<K>::THREAD_ID, threadId);

	uinttype senderId = 0;
	message->getUint32Field(Serializer<K>::SERVER_ID, senderId);

	boolean respond = false;

	// XXX: SUCCESS REPONSE
	if (messageId == getMessageId(Serializer<K>::MSG_ID_CODE_SUCCESS)) {
		if (threadId != 0) {
			// XXX: release waiting thread
			m_receiver->setContextCompleted(threadId);
		}
		
	// XXX: FAIL RESPONSE
	} else if (messageId == getMessageId(Serializer<K>::MSG_ID_CODE_FAIL)) {
		DEEP_LOG_ERROR(OTHER, "GOT FAILURE RESPONSE THREAD %lld \n", threadId);

	// XXX: VIRTUAL REPRESENTATION REQUEST (slave came online)
	} else if (messageId == getMessageId(Serializer<K>::MSG_ID_VIRTUAL_REPRESENTATION_REQUEST)) {
		m_receiver->processVirtualRepresentationRequest();

	// XXX: VIRTUAL UPDATE (virtual state change)
	} else if (messageId == getMessageId(Serializer<K>::MSG_ID_VIRTUAL_LOG_ENTRY)) {
		VirtualLogEntry<K> virtualLogEntry;
		Serializer<K>::deserialize(message, &virtualLogEntry);

		m_receiver->processVirtualUpdate(&virtualLogEntry);

	// XXX: VIRTUAL COMMIT (commit previos virtual state changes)
	} else if (messageId == getMessageId(Serializer<K>::MSG_ID_VIRTUAL_COMMIT)) {
		m_receiver->processVirtualCommit();

	// XXX: DATA LOG ENTRY (committed data)
	#if 0
	} else if (messageId == getMessageId(Serializer<K>::MSG_ID_DATA_LOG_ENTRY)) {
		Thread* thread = m_transactions.get(threadId);

		if ((message->isFirst()) == true) {
			if (thread != null) {
				ThreadPool::getInstance()->freeThread(thread);
			}

			DataTransaction<K>* dataTransaction = new DataTransaction<K>(m_receiver);
			dataTransaction->addDataLogEntry(message);

			thread = ThreadPool::getInstance()->allocThread(dataTransaction);

			m_transactions.put(threadId, thread);

		} else {
			DataTransaction<K>* threadState = (DataTransaction<K>*) thread->getRunnable();
			threadState->addDataLogEntry(message);
		}

		if (message->isLast()) {
			thread->start();	
			// in HA, we need to wait for this thread to finish before sending a response
			thread->join();
		}

		respond = true;
	#else
	} else if (messageId == getMessageId(Serializer<K>::MSG_ID_DATA_LOG_ENTRY_BLOCK)) {
		QuickList<DataLogEntry<K>*>* data = new QuickList<DataLogEntry<K>*>();
		Serializer<K>::deserialize(message, data);

		DataTransaction<K>* dataTransaction = new DataTransaction<K>(m_receiver);
		dataTransaction->setData(data);

		Thread* thread = ThreadPool::getInstance()->allocThread(dataTransaction);

		thread->start();	
		thread->join();

		ThreadPool::getInstance()->freeThread(thread);
		delete dataTransaction; 

		respond = true;
	#endif

	// XXX: VIRTUAL KEY SPACE VERSION REQUEST
	} else if (messageId == getMessageId(Serializer<K>::MSG_ID_VIRTUAL_KEY_SPACE_VERSION_REQUEST)) {
		sendVirtualKeySpaceVersionResponse(senderId, threadId);

	// XXX: VIRTUAL KEY SPACE VERSION RESPONSE
	} else if (messageId == getMessageId(Serializer<K>::MSG_ID_VIRTUAL_KEY_SPACE_VERSION_RESPONSE)) {
		longtype version = 0;
		Serializer<K>::deserialize(message, &version);

		m_receiver->setContextResult(version, threadId);
		m_receiver->setContextCompleted(threadId);	
	}

	// respond to all non-status response messages
	if ((respond == true) && ((message->isLast()) == true)) {
		sendResponseCode(Serializer<K>::MSG_ID_CODE_SUCCESS, senderId, 0 /* tableId */, threadId);
	}

	delete message;
}

template<typename K>
void FabricMessageService<K>::peerUpEvent() {
        m_peers++;
}

template<typename K>
void FabricMessageService<K>::peerDownEvent() {
        m_peers--;
}

template<typename K>
uinttype FabricMessageService<K>::getPeers() const {
	return m_peers;
}

template<typename K>
boolean FabricMessageService<K>::initialize(void* receiver) {
	return initialize((Facilitator<K>*)receiver);
}

template<typename K>
boolean FabricMessageService<K>::initialize(Facilitator<K>* receiver) {
	boolean success = true;
	m_receiver = receiver;

	// XXX: start service queue thread - grabs messages from this service and executes them on receiver

	m_receiverThread = new Thread(this);
	m_receiverThread->start();

	if (success == false) {
		throw InvalidException("Invalid FabricMessageService<K>::initialize: failed to start CassiServiceBridge");
	}

	return true;
}

template<typename K>
boolean FabricMessageService<K>::handleIncomingMessage(void* message) {
	cxx::io::encodeProtocol::reader* pcMsg = (encodeProtocol::reader*) message;
	m_messages.enqueue(pcMsg);

	return true;
}

template<typename K>
void FabricMessageService<K>::sendResponseCode(uinttype code, uinttype serverId, uinttype tableId, longtype threadId) {
	BaseMessageData baseMessageData(m_receiver->getServerId(), tableId, threadId);

	encodeProtocol::writer* encodeWriter = createWriter(getBaseMessageSize(&baseMessageData), getMessageId(code), &baseMessageData);
	encodeBaseMessage(encodeWriter, &baseMessageData);

	FabricEnvelope* fe = (FabricEnvelope*) encodeWriter;
	fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);

	fe->setPeerId(serverId);

	boolean success = (m_cassiServiceBridge->send(fe) == 0);
	if (success == false) {
		throw InvalidException("Invalid FabricMessageService<K>::sendResponseCode: send failed");
	}
}

template<typename K>
uinttype FabricMessageService<K>::getMessageId(uinttype msgId) {
	return MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI,msgId);
}

template<typename K>
boolean FabricMessageService<K>::registerMessageId(uinttype msgId) {
	return (m_fabricQueue->registerMessageId(msgId) == 0);
}

template<typename K>
encodeProtocol::writer* FabricMessageService<K>::createWriter(uinttype size, uinttype messageId, BaseMessageData* baseMessageData) {
        return new FabricEnvelope(size, messageId, 0 /* peerId */);
}

template<typename K>
uinttype FabricMessageService<K>::getBaseMessageSize(BaseMessageData* baseMessageData) {
	return encodeProtocol::writer::sizeOfUint32(baseMessageData->getServerId()) + 
	encodeProtocol::writer::sizeOfInt64(baseMessageData->getThreadId()) +
	encodeProtocol::writer::sizeOfUint32(baseMessageData->getTableId());
}

template<typename K>
void FabricMessageService<K>::encodeBaseMessage(encodeProtocol::writer* writer, BaseMessageData* baseMessageData) {
	writer->setUint32Field(Serializer<K>::SERVER_ID, baseMessageData->getServerId());
	writer->setInt64Field(Serializer<K>::THREAD_ID, baseMessageData->getThreadId());
	writer->setUint32Field(Serializer<K>::TABLE_ID, baseMessageData->getTableId());
}

template<typename K>
PeerInfo* FabricMessageService<K>::getReceiverPeerInfo() const {
	return m_receiver->getPeerInfo();
}

template<typename K>
void FabricMessageService<K>::addSubscriber(PeerInfo* peerInfo) {
	m_receiver->addSubscriber(peerInfo);
}

template<typename K>
void FabricMessageService<K>::subscribeTo(PeerInfo* peerInfo) {
	m_receiver->subscribeTo(peerInfo);
}

template<typename K>
longtype FabricMessageService<K>::sendVirtualKeySpaceVersionRequest(PeerInfo* peerInfo) {
	// XXX: mark this thread as incomplete, since this call requires a response before continuing (synchronous)
	m_receiver->setContextCompleted(false);

	BaseMessageData baseMessageData(m_receiver->getServerId(), 0 /* tableId */);

	FabricEnvelope* fe = (FabricEnvelope*) Serializer<K>::serializeBaseMessage(this, Serializer<K>::MSG_ID_VIRTUAL_KEY_SPACE_VERSION_REQUEST, &baseMessageData);
	fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);

	fe->setPeerId(peerInfo->getServerId());

	boolean success = (m_cassiServiceBridge->send(fe) == 0);
	if (success == false) {
		throw InvalidException("Invalid FabricMessageService<K>::sendVirtualKeySpaceVersionRequest: send failed");
	}

	m_receiver->waitForResult();

	return (m_receiver->getContextResult());
}

template<typename K>
void FabricMessageService<K>::sendVirtualKeySpaceVersionResponse(uinttype serverId, longtype threadId) {
	BaseMessageData baseMessageData(m_receiver->getServerId(), 0 /* tableId */, threadId);

	longtype version = m_receiver->getVirtualKeySpaceVersion();

	FabricEnvelope* fe = (FabricEnvelope*) Serializer<K>::serialize(this, version, &baseMessageData);
	fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);

	fe->setPeerId(serverId);

	boolean success = (m_cassiServiceBridge->send(fe) == 0);
	if (success == false) {
		throw InvalidException("Invalid FabricMessageService<K>::sendVirtualKeySpaceVersionResponse: send failed");
	}
}

template<typename K>
void FabricMessageService<K>::sendVirtualRepresentationRequest(PeerInfo* peerInfo) {
	FabricEnvelope* fe = new FabricEnvelope(0 /* bufferLen */, getMessageId(Serializer<K>::MSG_ID_VIRTUAL_REPRESENTATION_REQUEST), peerInfo->getServerId());
	fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);

	boolean success = (m_cassiServiceBridge->send(fe) == 0);
	if (success == false) {
		throw InvalidException("Invalid FabricMessageService<K>::sendVirtualRepresentationRequest: send failed");
	}
}

template<typename K>
void FabricMessageService<K>::publishVirtualUpdate(ArrayList<PeerInfo*>* peerInfos, const void* logEntry) {
	publishVirtualUpdate(peerInfos, (const VirtualLogEntry<K>*) logEntry);
}

template<typename K>
void FabricMessageService<K>::publishVirtualUpdate(ArrayList<PeerInfo*>* peerInfos, const VirtualLogEntry<K>* logEntry) {
	for (int x = 0; x < peerInfos->ArrayList<PeerInfo*>::size(); x++) {
		PeerInfo* peerInfo = peerInfos->ArrayList<PeerInfo*>::get(x);

		BaseMessageData baseMessageData(m_receiver->getServerId(), 0 /* tableId */);

		FabricEnvelope* fe = (FabricEnvelope*) Serializer<K>::serialize(this, (VirtualLogEntry<K>*)logEntry, &baseMessageData);
		fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);

		fe->setPeerId(peerInfo->getServerId());

		boolean success = (m_cassiServiceBridge->send(fe) == 0);
		if (success == false) {
			throw InvalidException("Invalid FabricMessageService<K>::publishVirtualUpdate: send failed");
		}
	}
}

template<typename K>
void FabricMessageService<K>::publishVirtualCommit(ArrayList<PeerInfo*>* peerInfos) {
	for (int x = 0; x < peerInfos->ArrayList<PeerInfo*>::size(); x++) {
		PeerInfo* peerInfo = peerInfos->ArrayList<PeerInfo*>::get(x);

		FabricEnvelope* fe = new FabricEnvelope(0 /* bufferLen */, getMessageId(Serializer<K>::MSG_ID_VIRTUAL_COMMIT), peerInfo->getServerId());
		fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);

		boolean success = (m_cassiServiceBridge->send(fe) == 0);
		if (success == false) {
			throw InvalidException("Invalid FabricMessageService<K>::publishVirtualCommit: send failed");
		}
	}
}

template<typename K>
void FabricMessageService<K>::publishDataLog(ArrayList<PeerInfo*>* peerInfos, void* data) {
	publishDataLog(peerInfos, (QuickList<DataLogEntry<K>*>*)data);
}

template<typename K>
void FabricMessageService<K>::publishDataLog(ArrayList<PeerInfo*>* peerInfos, QuickList<DataLogEntry<K>*>* data) {
	if (data != null) {
		// XXX: mark this thread as incomplete, since this call requires a response before continuing (synchronous)
		m_receiver->setContextCompleted(false);

		BaseMessageData baseMessageData(m_receiver->getServerId(), 0 /* tableId */);

		#if 0
		uinttype size = data->getSize();
		for (uinttype x = 0; x < size; x++) {

			DataLogEntry<K>* entry = data->getHead()->getEntry();

			FabricEnvelope* fe = (FabricEnvelope*) Serializer<K>::serialize(this, entry, &baseMessageData);

			if (size == 1) {
				fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);
			} else if (x == 0) {
				fe->setFlags(encodeProtocol::header::FIRST_FLAG);
			} else if (x == size - 1) {
				fe->setFlags(encodeProtocol::header::LAST_FLAG);
			}
			fe->setPeerId(peerInfo->getServerId());

			boolean success = (m_cassiServiceBridge->send(fe) == 0);
			if (success == false) {
				throw InvalidException("Invalid FabricMessageService<K>::publishDataLog: send failed");
			}

			data->advanceHead();
		}
		#else
		for (int x = 0; x < peerInfos->ArrayList<PeerInfo*>::size(); x++) {
			PeerInfo* peerInfo = peerInfos->ArrayList<PeerInfo*>::get(x);

			// TODO: generating this more than once is expensive
			FabricEnvelope* fe = (FabricEnvelope*) Serializer<K>::serialize(this, data, &baseMessageData);

			fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);

			fe->setPeerId(peerInfo->getServerId());

			boolean success = (m_cassiServiceBridge->send(fe) == 0);
			if (success == false) {
				throw InvalidException("Invalid FabricMessageService<K>::publishDataLog: send failed");
			}

			// XXX: thread must wait for response from peer before returning (synchronous)
			if (peerInfo->getMapBehavior() == DISTRIBUTED_HA_SYNC_SLAVE) {
				m_receiver->waitForResult();
			}
		}
		#endif
	}
}

template<typename K>
void FabricMessageService<K>::sendDataRequest(PeerInfo* peerInfo, const void* dataRequest) {
	sendDataRequest(peerInfo, (const DataRequest<K>*) dataRequest);
}

template<typename K>
void FabricMessageService<K>::sendDataRequest(PeerInfo* peerInfo, const DataRequest<K>* dataRequest) {
	BaseMessageData baseMessageData(m_receiver->getServerId(), 0 /* tableId */);

	FabricEnvelope* fe = (FabricEnvelope*) Serializer<K>::serialize(this, (DataRequest<K>*)dataRequest, &baseMessageData);
	fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);

	fe->setPeerId(peerInfo->getServerId());

	boolean success = (m_cassiServiceBridge->send(fe) == 0);
	if (success == false) {
		throw InvalidException("Invalid FabricMessageService<K>::sendDataRequest: send failed");
	}
}


template<typename K>
void FabricMessageService<K>::sendStatusRequest(PeerInfo* peerInfo, const void* statusRequest) {
	sendStatusRequest(peerInfo, (StatusRequest<K>*)statusRequest);
}

template<typename K>
void FabricMessageService<K>::sendStatusRequest(PeerInfo* peerInfo, const StatusRequest<K>* statusRequest) {
	BaseMessageData baseMessageData(m_receiver->getServerId(), 0 /* tableId */);

	FabricEnvelope* fe = (FabricEnvelope*) Serializer<K>::serialize(this, (StatusRequest<K>*)statusRequest, &baseMessageData);
	fe->setFlags(encodeProtocol::header::FIRST_FLAG | encodeProtocol::header::LAST_FLAG);

	fe->setPeerId(peerInfo->getServerId());

	boolean success = (m_cassiServiceBridge->send(fe) == 0);
	if (success == false) {
		throw InvalidException("Invalid FabricMessageService<K>::sendDataRequest: send failed");
	}
}

#endif /* COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_FABRICMESSAGESERVICE_CXX_ */
