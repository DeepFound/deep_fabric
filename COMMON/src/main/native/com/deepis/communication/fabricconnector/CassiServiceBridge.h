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
#ifndef COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_CASSISERVICEBRIDGE_H_
#define COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_CASSISERVICEBRIDGE_H_

#include "cxx/lang/Thread.h"
#include "cxx/lang/Runnable.h"
#include "cxx/io/EncodeProtocol.h"

#include "com/deepis/communication/common/MsgIds.h"
#include "com/deepis/communication/fabricconnector/FabricCassiMessageQueue.h"
#include "com/deepis/db/store/relative/distributed/IMessageService.h"
#include "com/deepis/db/store/relative/distributed/Serializer.h"

using namespace cxx::lang;
using namespace cxx::io;
using namespace com::deepis::communication::common;
using namespace com::deepis::db::store::relative::distributed;

namespace com { namespace deepis { namespace communication { namespace fabricconnector {

class CassiServiceBridge : public FabricCassiBridgeCb {
	private:

		// TODO: this will be collection of tableId -> messageService
		IMessageService* m_messageService;
	
		CassiServiceBridge(FabricCassiMessageQueue* pcCommon) :
			FabricCassiBridgeCb(pcCommon),
			m_messageService(null) {
		}

	public:
		static CassiServiceBridge* getInstance(FabricCassiMessageQueue* pcCommon) {
			return new CassiServiceBridge(pcCommon);
		}

		virtual const uinttype* allMessageIds(void) {
			static uinttype messageIds[15];

			messageIds[0] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_STATUS_REQUEST);
			messageIds[1] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_STATUS_RESPONSE);
			messageIds[2] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_DATA_REQUEST);
			messageIds[3] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_VIRTUAL_INFO);
			messageIds[4] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_DATA_LOG_ENTRY);
			messageIds[5] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_DATA_LOG_ENTRY_BLOCK);
			messageIds[6] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_VIRTUAL_LOG_ENTRY);
			messageIds[7] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_VIRTUAL_REPRESENTATION_REQUEST);
			messageIds[8] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_VIRTUAL_COMMIT);
			messageIds[9] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_PEER_INFO);
			messageIds[10] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_VIRTUAL_KEY_SPACE_VERSION_REQUEST);
			messageIds[11] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_VIRTUAL_KEY_SPACE_VERSION_RESPONSE);

			messageIds[12] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_CODE_SUCCESS);
			messageIds[13] = MSG_ID(msgPrefix::REALTIME_DISTRIBUTED_CASSI, Serializer<void*>::MSG_ID_CODE_FAIL);
			messageIds[14] = 0;

			return messageIds;
		}

		virtual void setMessageService(IMessageService* messageService) {
			m_messageService = messageService;
			m_messageService->setFabricQueue(m_pcCommon);
			m_messageService->setCassiServiceBridge(this);
		}

		virtual void remoteOverloaded(cxx::io::encodeProtocol::reader* pcMsg) {
			
		}

		virtual void shutdown() {

		}

		virtual void peerStateChanged(cxx::io::encodeProtocol::reader* pcMsg) {
			if (m_messageService != null) {
				inttype state = 0;
				pcMsg->getInt32Field(com::deepis::communication::fabricconnector::FabricCassiMessageQueue::PEER_CHANGE::STATE_FIELD, state);
				if (state == com::deepis::communication::fabricconnector::FabricCassiMessageQueue::PEER_CHANGE::UP) {
					m_messageService->peerUpEvent();

				} else {
					m_messageService->peerDownEvent();
				}
			}
		}

		virtual inttype dispatch(cxx::io::encodeProtocol::reader* pcMsg) {
			m_messageService->handleIncomingMessage(pcMsg) == true;
			// indicate not to delete message
			return 1;
		}
};

} } } } // namespace

com::deepis::communication::fabricconnector::FabricCassiBridgeCb*
singletonCassiServiceBridge(com::deepis::communication::fabricconnector::FabricCassiMessageQueue* pcCommon, voidptr pCtx);

#endif /* COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_CASSISERVICEBRIDGE_H_ */
