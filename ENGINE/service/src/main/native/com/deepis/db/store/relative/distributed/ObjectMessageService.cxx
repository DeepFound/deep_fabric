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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_OBJECTMESSAGESERVICE_CXX_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_OBJECTMESSAGESERVICE_CXX_

#include "com/deepis/db/store/relative/distributed/ObjectMessageService.h"
#include "com/deepis/db/store/relative/distributed/PeerInfo.h"
#include "com/deepis/db/store/relative/util/InvalidException.h"

using namespace com::deepis::db::store::relative::distributed;
using namespace com::deepis::db::store::relative::util;

template<typename K>
void ObjectMessageService<K>::setCassiServiceBridge(void* cassiServiceBridge) {
	return;
}

template<typename K>
void ObjectMessageService<K>::setFabricQueue(void* fabricQueue) {
	return;
}

template<typename K>
void ObjectMessageService<K>::peerUpEvent() {
	return;
}

template<typename K>
void ObjectMessageService<K>::peerDownEvent() {
	return;
}

template<typename K>
uinttype ObjectMessageService<K>::getPeers() const {
	return 0;
}

template<typename K>
boolean ObjectMessageService<K>::initialize(void* receiver) {
	return initialize((Facilitator<K>*)receiver);
}

template<typename K>
boolean ObjectMessageService<K>::initialize(Facilitator<K>* receiver) {
	m_receiver = receiver;
	return true;
}

template<typename K>
boolean ObjectMessageService<K>::handleIncomingMessage(void* message) {
	throw InvalidException("Invalid ObjectMessageService::handleIncomingMessage: invalid message");
}

template<typename K>
uinttype ObjectMessageService<K>::getMessageId(uinttype msgId) {
	return msgId;
}

template<typename K>
boolean ObjectMessageService<K>::registerMessageId(uinttype msgId) {
	return true;
}

template<typename K>
encodeProtocol::writer* ObjectMessageService<K>::createWriter(uinttype size, uinttype messageId, BaseMessageData* baseMessageData) {
	return new encodeProtocol::writer(size, messageId);
}

template<typename K>
uinttype ObjectMessageService<K>::getBaseMessageSize(BaseMessageData* baseMessageData) {
	return 0;
}

template<typename K>
void ObjectMessageService<K>::encodeBaseMessage(encodeProtocol::writer* writer, BaseMessageData* baseMessageData) {
	return;
}

template<typename K>
PeerInfo* ObjectMessageService<K>::getReceiverPeerInfo() const {
	return m_receiver->getPeerInfo();
}

template<typename K>
void ObjectMessageService<K>::addSubscriber(PeerInfo* peerInfo) {
	m_receiver->addSubscriber(peerInfo);
}

template<typename K>
void ObjectMessageService<K>::subscribeTo(PeerInfo* peerInfo) {
	m_receiver->subscribeTo(peerInfo);
}

template<typename K>
longtype ObjectMessageService<K>::sendVirtualKeySpaceVersionRequest(PeerInfo* peerInfo) {
	return ((Facilitator<K>*)(peerInfo->getFacilitator()))->getVirtualKeySpaceVersion();
}

template<typename K>
void ObjectMessageService<K>::sendVirtualRepresentationRequest(PeerInfo* peerInfo) {
	return m_receiver->sendVirtualRepresentationRequest();
}

template<typename K>
void ObjectMessageService<K>::publishVirtualUpdate(ArrayList<PeerInfo*>* peerInfos, const void* logEntry) {
	publishVirtualUpdate(peerInfos, (const VirtualLogEntry<K>*)logEntry);
}

template<typename K>
void ObjectMessageService<K>::publishVirtualUpdate(ArrayList<PeerInfo*>* peerInfos, const VirtualLogEntry<K>* logEntry) {
	m_receiver->publishVirtualUpdate(logEntry);
}

template<typename K>
void ObjectMessageService<K>::publishVirtualCommit(ArrayList<PeerInfo*>* peerInfos) {
	m_receiver->publishVirtualCommit();
}

template<typename K>
void ObjectMessageService<K>::publishDataLog(ArrayList<PeerInfo*>* peerInfos, void* data) {
	publishDataLog(peerInfos, (QuickList<DataLogEntry<K>*>*)data);
}

template<typename K>
void ObjectMessageService<K>::publishDataLog(ArrayList<PeerInfo*>* peerInfos, QuickList<DataLogEntry<K>*>* data) {
	m_receiver->publishDataLog(data);
}

template<typename K>
void ObjectMessageService<K>::sendDataRequest(PeerInfo* peerInfo, const void* dataRequest) {
	sendDataRequest(peerInfo, (const DataRequest<K>*) dataRequest);
}

template<typename K>
void ObjectMessageService<K>::sendDataRequest(PeerInfo* peerInfo, const DataRequest<K>* dataRequest) {
	m_receiver->sendDataRequest(peerInfo, dataRequest);
}

template<typename K>
void ObjectMessageService<K>::sendStatusRequest(PeerInfo* peerInfo, const void* statusRequest) {
	sendStatusRequest(peerInfo, (StatusRequest<K>*)statusRequest);
}

template<typename K>
void ObjectMessageService<K>::sendStatusRequest(PeerInfo* peerInfo, const StatusRequest<K>* statusRequest) {
	m_receiver->sendStatusRequest(peerInfo, statusRequest);
}

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_OBJECTMESSAGESERVICE_CXX_ */
