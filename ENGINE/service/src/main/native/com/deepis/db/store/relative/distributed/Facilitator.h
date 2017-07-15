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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_FACILITATOR_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_FACILITATOR_H_

#include "com/deepis/db/store/relative/distributed/LogEntry.h"
#include "com/deepis/db/store/relative/distributed/QuickList.h"
#include "com/deepis/db/store/relative/distributed/DataRequest.h"
#include "com/deepis/db/store/relative/distributed/StatusRequest.h"

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

class PeerInfo;

template<typename K>
class Facilitator {
	public:
		virtual boolean allowRemoteOwner() const = 0;
		virtual boolean allowRemoteConsumer() const = 0;

		virtual PeerInfo* getPeerInfo() const = 0;
		virtual uinttype getServerId() const = 0;
		virtual void addSubscriber(PeerInfo* subscriber) = 0;
		virtual void subscribeTo(PeerInfo* owner) = 0;

		virtual void sendVirtualRepresentationRequest() = 0;
		virtual void publishVirtualUpdate(const VirtualLogEntry<K>* logEntry) = 0;
		virtual void publishVirtualCommit() = 0;

		virtual longtype getVirtualKeySpaceVersion() const = 0;
		virtual void processVirtualRepresentationRequest() = 0;
		virtual void processVirtualUpdate(const VirtualLogEntry<K>* logEntry) = 0;
		virtual void processVirtualCommit() = 0;

		virtual void publishDataLog(QuickList<DataLogEntry<K>*>* data) = 0;
		virtual void sendDataRequest(PeerInfo* peerInfo, const DataRequest<K>* dataRequest) = 0;
		virtual void processDataTransaction(QuickList<DataLogEntry<K>*>* data) = 0;
		
		virtual void sendStatusRequest(PeerInfo* peerInfo, const StatusRequest<K>* statusRequest) const = 0;

		virtual void setContextResult(longtype result, longtype threadId = 0) const = 0;
		virtual longtype getContextResult() const = 0;
		virtual void setContextCompleted(longtype threadId) const = 0;
		virtual void setContextCompleted(boolean completed) const = 0;
		virtual void waitForResult() const = 0;
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_FACILITATOR_H_ */
