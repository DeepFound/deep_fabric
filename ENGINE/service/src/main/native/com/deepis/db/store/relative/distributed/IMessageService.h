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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_IMESSAGESERVICE_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_IMESSAGESERVICE_H_

#include "cxx/lang/types.h"
#include "cxx/util/ArrayList.h"
#include "com/deepis/db/store/relative/distributed/Facilitator.h"
#include "com/deepis/db/store/relative/distributed/BaseMessageData.h"
#include "cxx/io/EncodeProtocol.h"

using namespace cxx::io;
using namespace cxx::util;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

class IMessageService {
	public: 
		virtual void setCassiServiceBridge(void* cassieServiceBridge) = 0;

		virtual void setFabricQueue(void* fabricQueue) = 0;

		// XXX: states

		virtual void peerUpEvent() = 0;

		virtual void peerDownEvent() = 0;

		virtual uinttype getPeers() const = 0;

		// XXX: incoming message handling

		virtual boolean initialize(void* receiver) = 0;

		virtual boolean handleIncomingMessage(void* message) = 0;

		// XXX: message setup

		virtual uinttype getMessageId(uinttype msgId) = 0;

		virtual boolean registerMessageId(uinttype msgId) = 0;

		virtual encodeProtocol::writer* createWriter(uinttype size, uinttype messageId, BaseMessageData* baseMessageData) = 0;

		virtual uinttype getBaseMessageSize(BaseMessageData* baseMessageData) = 0;

		virtual void encodeBaseMessage(encodeProtocol::writer* writer, BaseMessageData* baseMessageData) = 0;

		// XXX: subscriber methods

		virtual PeerInfo* getReceiverPeerInfo() const = 0;

		virtual void addSubscriber(PeerInfo* peerInfo) = 0;

		virtual void subscribeTo(PeerInfo* peerInfo) = 0;

		// XXX: virtual key space methods

		virtual longtype sendVirtualKeySpaceVersionRequest(PeerInfo* peerInfo) = 0;
		
		virtual void sendVirtualRepresentationRequest(PeerInfo* peerInfo) = 0;

		virtual void publishVirtualUpdate(ArrayList<PeerInfo*>* peerInfos, const void* logEntry) = 0;

		virtual void publishVirtualCommit(ArrayList<PeerInfo*>* peerInfos) = 0;

		// XXX: row data methods

		virtual void publishDataLog(ArrayList<PeerInfo*>* peerInfos, void* data) = 0;

		virtual void sendDataRequest(PeerInfo* peerInfo, const void* dataRequest) = 0;

		// XXX: status methods

		virtual void sendStatusRequest(PeerInfo* peerInfo, const void* statusRequest) = 0;
		

		IMessageService() { }
		virtual ~IMessageService() { }
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_IMESSAGESERVICE_H_ */
