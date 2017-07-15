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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_OBJECTMESSAGESERVICE_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_OBJECTMESSAGESERVICE_H_

#include "com/deepis/db/store/relative/distributed/IMessageService.h"

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

template<typename K>
class ObjectMessageService : public IMessageService {
	private:
		Facilitator<K>* m_receiver;

	public:
		ObjectMessageService() : 
			m_receiver(null) {
		}

		virtual void setCassiServiceBridge(void* cassiServiceBridge);

		virtual void setFabricQueue(void* fabricQueue);

		// XXX: states

                virtual void peerUpEvent();

                virtual void peerDownEvent();

		virtual uinttype getPeers() const;

		// XXX: incoming message handling

		virtual boolean initialize(void* receiver);
		boolean initialize(Facilitator<K>* facilitator);

		virtual boolean handleIncomingMessage(void* message);

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

		// XXX: etc

		virtual ~ObjectMessageService() { }
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_OBJECTMESSAGESERVICE_H_ */
