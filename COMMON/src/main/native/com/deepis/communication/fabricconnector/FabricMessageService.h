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
#ifndef COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_FABRICMESSAGESERVICE_H_
#define COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_FABRICMESSAGESERVICE_H_

#include "com/deepis/db/store/relative/distributed/IMessageService.h"
#include "com/deepis/db/store/relative/distributed/BaseMessageData.h"
#include "com/deepis/db/store/relative/distributed/Facilitator.h"
#include "com/deepis/db/store/relative/distributed/LogEntry.h"
#include "com/deepis/communication/fabricconnector/FabricCassiMessageQueue.h"
#include "cxx/io/EncodeProtocol.h"
#include "cxx/lang/Runnable.h"
#include "cxx/lang/Thread.h"
#include "cxx/io/MsgQueue.h"
#include "cxx/util/TreeMap.cxx"

using namespace com::deepis::db::store::relative::distributed;
using namespace cxx::io;
using namespace cxx::lang;
using namespace cxx::util;

namespace com { namespace deepis { namespace communication { namespace fabricconnector {

class CassiServiceBridge;

template<typename K>
class FabricMessageService : public Runnable, public IMessageService {
	private:
		static const uinttype MESSAGE_QUEUE_SIZE = (1024 << 4);
		static const Comparator<longtype> LONG_CMP;

		CassiServiceBridge* m_cassiServiceBridge;
		FabricCassiMessageQueue* m_fabricQueue;
		MsgQueue m_messages;

		// owner thread id -> local state / thread data
		TreeMap<longtype, Thread*> m_transactions;

		Facilitator<K>* m_receiver;
		Thread* m_receiverThread;

		boolean m_lookForMessages;
		uinttype m_peers;

	public:
		FabricMessageService() :
			m_cassiServiceBridge(null),
			m_fabricQueue(null),
			m_messages(MESSAGE_QUEUE_SIZE, false /* bEnableEventFd */),
			m_transactions(&LONG_CMP),
			m_receiver(null),
			m_receiverThread(null),
			m_lookForMessages(true),
			m_peers(0) {
		
			m_transactions.setDeleteKey(false);
			m_transactions.setDeleteValue(false);
		}

		virtual ~FabricMessageService() {
			//CassiServiceBridge::getInstance()->shutdown();

			m_lookForMessages = false;
			if (m_receiverThread != null) {
				m_receiverThread->join();
				delete m_receiverThread;
			}
		}

		virtual void setCassiServiceBridge(void* cassiServiceBridge) {
			setCassiServiceBridge((CassiServiceBridge*)cassiServiceBridge);
		}

		void setCassiServiceBridge(CassiServiceBridge* cassiServiceBridge) {
			m_cassiServiceBridge = cassiServiceBridge;
		}

		virtual void setFabricQueue(void* fabricQueue) {
			setFabricQueue((FabricCassiMessageQueue*)fabricQueue);
		}

		void setFabricQueue(FabricCassiMessageQueue* fabricQueue) {
			m_fabricQueue = fabricQueue;
		}
		
		// XXX: states

		virtual void peerUpEvent();

		virtual void peerDownEvent();

		virtual uinttype getPeers() const;

		// XXX: incoming message handling

		virtual void run();

		virtual void dispatchMessage(encodeProtocol::reader* message);

		virtual boolean initialize(void* receiver);
		boolean initialize(Facilitator<K>* facilitator);

		virtual boolean handleIncomingMessage(void* message);

		void sendResponseCode(uinttype code, uinttype peerId, uinttype tableId, longtype threadId);

		// XXX: message setup

		virtual uinttype getMessageId(uinttype msgId);

		virtual boolean registerMessageId(uinttype msgId);

		virtual encodeProtocol::writer* createWriter(uinttype size, uinttype messageId, BaseMessageData* baseMessageData);

		virtual uinttype getBaseMessageSize(BaseMessageData* baseMessageData);

		virtual void encodeBaseMessage(encodeProtocol::writer* writer, BaseMessageData* baseMessageData);

                // XXX: subscriber methods

		virtual PeerInfo* getReceiverPeerInfo() const;

		virtual void addSubscriber(PeerInfo* peerInfo);

		virtual void subscribeTo(PeerInfo* peerInfo);

		// XXX: virtual key space methods

		virtual longtype sendVirtualKeySpaceVersionRequest(PeerInfo* peerInfo);

		void sendVirtualKeySpaceVersionResponse(uinttype serverId, longtype threadId);

		virtual void sendVirtualRepresentationRequest(PeerInfo* peerInfo);

		virtual void publishVirtualUpdate(ArrayList<PeerInfo*>* peerInfos, const void* logEntry);
		void publishVirtualUpdate(ArrayList<PeerInfo*>* peerInfos, const VirtualLogEntry<K>* logEntry);

		virtual void publishVirtualCommit(ArrayList<PeerInfo*>* peerInfos);

		// XXX: row data methods

		virtual void publishDataLog(ArrayList<PeerInfo*>* peerInfos, void* data);
		void publishDataLog(ArrayList<PeerInfo*>* peerInfos, QuickList<DataLogEntry<K>*>* data);

		virtual void sendDataRequest(PeerInfo* peerInfo, const void* dataRequest);
		void sendDataRequest(PeerInfo* peerInfo, const DataRequest<K>* dataRequest);

		// XXX: status methods
		virtual void sendStatusRequest(PeerInfo* peerInfo, const void* statusRequest);
		void sendStatusRequest(PeerInfo* peerInfo, const StatusRequest<K>* statusRequest);
};

} } } } // namespace

#endif /* COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_FABRICMESSAGESERVICE_H_ */
