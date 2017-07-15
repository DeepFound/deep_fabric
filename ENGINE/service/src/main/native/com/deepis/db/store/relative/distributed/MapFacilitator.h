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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_MAPFACILITATOR_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_MAPFACILITATOR_H_

#include "cxx/util/ArrayList.h" 
#include "com/deepis/db/store/relative/core/RealTimeMap.h"

#include "com/deepis/db/store/relative/distributed/Facilitator.h"
#include "com/deepis/db/store/relative/distributed/IMessageService.h"
#include "com/deepis/db/store/relative/distributed/ObjectMessageService.h"
#include "com/deepis/db/store/relative/distributed/MapBehavior.h"
#include "com/deepis/db/store/relative/distributed/VirtualInfo.h"
#include "com/deepis/db/store/relative/distributed/DataRequest.h"
#include "com/deepis/db/store/relative/distributed/StatusRequest.h"
#include "com/deepis/db/store/relative/distributed/LogEntry.h"
#include "com/deepis/db/store/relative/distributed/VirtualKeySpace.h"
#include "com/deepis/db/store/relative/distributed/SegmentDataManager.h"
#include "com/deepis/db/store/relative/distributed/DataLogManager.h"
#include "com/deepis/db/store/relative/distributed/QuickList.h"
#include "com/deepis/db/store/relative/distributed/PeerInfo.h"

using namespace cxx::util;              
using namespace com::deepis::db::store::relative::core;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

template<typename K>
class CommitWorkspace {
	private:
		const KeyBuilder<K>* m_keyBuilder;
		QuickList<VirtualLogEntry<K>*>* m_vlog;
		QuickList<DataLogEntry<K>*>* m_log;

		FORCE_INLINE void init() {
			if (m_log == null) {
				m_log = new QuickList<DataLogEntry<K>*>();
			}
		}

	public:
		CommitWorkspace(const KeyBuilder<K>* keyBuilder):
			m_keyBuilder(keyBuilder),
			m_vlog(null),
			m_log(null) {
		}

		virtual ~CommitWorkspace() {
			if (m_log != null) {
				delete m_log;
			}
	
			if (m_vlog != null) {
				delete m_vlog;
			}
		}

		QuickList<VirtualLogEntry<K>*>* getVirtualLog() const {
			return m_vlog;
		}

		QuickList<DataLogEntry<K>*>* getLog() const {
			return m_log;
		}

		boolean updateSegmentStats(Segment<K>* segment, longtype commitViewpoint) const {
			boolean changed = false;

			if (commitViewpoint > segment->getMaxRealViewpoint()) {
				segment->setMaxRealViewpoint(commitViewpoint);
				changed = true;
			}

			if (VirtualKeySpace<K>::versionKeySpace(segment, 0, segment->vsize(), commitViewpoint, commitViewpoint) == true) {
				changed = true;
			}

			return changed;
		}

		void logVirtualUpdate(const K key, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint) {
			if (m_vlog == null) {
				m_vlog = new QuickList<VirtualLogEntry<K>*>();
			}
			
			VirtualLogEntry<K>* virtualLogEntry = new VirtualLogEntry<K>(UPDATE, key, vsize, minViewpoint, maxViewpoint);
			m_vlog->addEntry(virtualLogEntry);
		}

		void logInsert(const K key, uinttype viewpoint, uinttype size, bytearray data) {
			init();

			DataLogEntry<K>* logEntry = new DataLogEntry<K>(ADD, m_keyBuilder->cloneKey(key), viewpoint, size, data);
			m_log->addEntry(logEntry);
		}

		void logUpdate(const K key, uinttype viewpoint, uinttype size, bytearray data) {
			init();

			DataLogEntry<K>* logEntry = new DataLogEntry<K>(UPDATE, m_keyBuilder->cloneKey(key), viewpoint, size, data);
			m_log->addEntry(logEntry);
		}

		void logDelete(const K key, uinttype viewpoint, uinttype size, bytearray data) {
			init();

			DataLogEntry<K>* logEntry = new DataLogEntry<K>(REMOVE, m_keyBuilder->cloneKey(key), viewpoint, size, data);
			m_log->addEntry(logEntry);
		}
};

template<typename K>
class MapFacilitator : public Facilitator<K> {

	public:
		FORCE_INLINE virtual boolean allowRemoteOwner() const {
			if (getMapBehavior() == DISTRIBUTED_STANDALONE) {
				return false;
			}
			if (getMapBehavior() == DISTRIBUTED_MASTER) {
				return false;
			}

			return true;
		}

		FORCE_INLINE virtual boolean allowRemoteConsumer() const {
			if (getMapBehavior() == DISTRIBUTED_STANDALONE) {
				return false;
			}
			if (getMapBehavior() == DISTRIBUTED_LAZY_SLAVE) {
				return false;
			}
			if (getMapBehavior() == DISTRIBUTED_HA_SYNC_SLAVE) {
				return false;
			}
			if (getMapBehavior() == DISTRIBUTED_HA_ASYNC_SLAVE) {
				return false;
			}

			return true;
		}

		FORCE_INLINE MapBehavior getMapBehavior() const {
			return m_myInfo->getMapBehavior();
		}

	private:
		RealTimeMap<K>* m_map;
		IMessageService* m_messageService;

		boolean m_synchronizeTransactions;

		ArrayList<PeerInfo*>* m_subscribers;

		// TODO this will be a collection of owners of various maps / keyspaces
		PeerInfo* m_owner;

		VirtualKeySpace<K>* m_virtualKeySpace;
		SegmentDataManager<K>* m_segmentDataManager;
		DataLogManager<K>* m_dataLogManager;
		PeerInfo* m_myInfo;

	public: 
		MapFacilitator(RealTimeMap<K>* map, MapBehavior mapBehavior, IMessageService* messageService, PeerInfo* myInfo = null) :
			m_map(map),
			m_messageService(messageService),
			m_synchronizeTransactions(false),
			m_subscribers(null),
			m_owner(null),
			m_dataLogManager(null),
			m_myInfo(myInfo) {

			m_virtualKeySpace = new VirtualKeySpace<K>(this, map, map->m_comparator);
			m_segmentDataManager = new SegmentDataManager<K>(m_map);

			if (mapBehavior == DISTRIBUTED_MASTER) {
				m_dataLogManager = new DataLogManager<K>();
			}

			if (m_myInfo == null) {
				m_myInfo = new PeerInfo(0 /* serverId */, "" /* address */, mapBehavior, this);
			}
		}

		FORCE_INLINE IMessageService* getMessageService() {
			return m_messageService;
		}

		// XXX: capabilities of this map

		FORCE_INLINE boolean maintainLiveSummaries() const {
			return (getMapBehavior() == DISTRIBUTED_MASTER);
		}

		FORCE_INLINE boolean useLiveViewpoint() const {
			return (getMapBehavior() == DISTRIBUTED_MASTER);
		}

		FORCE_INLINE boolean logChanges() const {
			return (getMapBehavior() == DISTRIBUTED_MASTER);
		}

		FORCE_INLINE boolean synchronizeTransactions() const {
			return m_synchronizeTransactions;
		} 

		// XXX: virtual key space methods

		FORCE_INLINE void addVirtualFirstKey(K key, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint) {
			m_virtualKeySpace->addKey(key, vsize, minViewpoint, maxViewpoint);
		}
		
		FORCE_INLINE void updateVirtualKey(K key, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint) {
			m_virtualKeySpace->updateKey(key, vsize, minViewpoint, maxViewpoint);
		}
	
		FORCE_INLINE void removeVirtualFirstKey(K key) {
			m_virtualKeySpace->removeKey(key);
		}

		virtual longtype getVirtualKeySpaceVersion() const {
			return m_virtualKeySpace->getKeySpaceVersion();
		}	

		FORCE_INLINE static uinttype isolateMinViewpoint(Segment<K>* segment, longtype version = 0) {
			return VirtualKeySpace<K>::isolateMinViewpoint(segment, version);
		}

		FORCE_INLINE static uinttype isolateMaxVirtualViewpoint(Segment<K>* segment, longtype version = 0) {
			return VirtualKeySpace<K>::isolateMaxVirtualViewpoint(segment, version);
		}

		FORCE_INLINE static uinttype isolateVirtualSize(Segment<K>* segment, longtype version = 0) {
			return VirtualKeySpace<K>::isolateVirtualSize(segment, version);
		}

		FORCE_INLINE void versionVirtualKeySpace(Segment<K>* segment, uinttype vsize, uinttype minViewpoint, uinttype maxVirtualViewpoint) {
			VirtualKeySpace<K>::versionKeySpace(segment, getVirtualKeySpaceVersion(), vsize, minViewpoint, maxVirtualViewpoint);
		}

		// XXX: subscriber methods

		virtual PeerInfo* getPeerInfo() const {
			return m_myInfo;
		}

		ArrayList<PeerInfo*>* getSubscribers() const {
			return m_subscribers;
		}

		PeerInfo* getPeerInfo(uinttype x) const {
			return m_subscribers->ArrayList<PeerInfo*>::get(x);
		}

		virtual uinttype getServerId() const {
			return m_myInfo->getServerId();
		}

		virtual void addSubscriber(PeerInfo* subscriber);
		virtual void subscribeTo(PeerInfo* owner);

		boolean subscriberSyncHold();

		// XXX: virtual keyspace methods

			// XXX: message passing methods
			virtual void publishVirtualUpdate(const VirtualLogEntry<K>* logEntry);
			virtual void publishVirtualCommit();
			virtual void sendVirtualRepresentationRequest();

		void requestVirtualRepresentation();
		virtual void processVirtualRepresentationRequest();

		virtual void processVirtualUpdate(const VirtualLogEntry<K>* logEntry);
		virtual void processVirtualCommit();

		// XXX: row data methods

			// XXX: message passing methods
			virtual void publishDataLog(QuickList<DataLogEntry<K>*>* data);
			virtual void sendDataRequest(PeerInfo* peerInfo, const DataRequest<K>* dataRequest);

		void requestDataForSegment(Segment<K>* segment, longtype version);
		void processDataRequest(MapFacilitator<K>* requester, const DataRequest<K>* dataRequest);
		virtual void processDataTransaction(QuickList<DataLogEntry<K>*>* data);
		void processDataReceived(QuickList<DataLogEntry<K>*>* data);

		FORCE_INLINE void commitLog(CommitWorkspace<K>* workspace) {
			if (workspace->getVirtualLog() != null) {
				m_virtualKeySpace->appendVirtualLog(workspace->getVirtualLog());
				workspace->getVirtualLog()->disown();
			}

			// TODO: keep this log around if we have a mix of HA and non-HA peers
			if ((workspace->getLog() != null) && (synchronizeTransactions() == false)) {
				m_dataLogManager->logChangeSet(workspace->getLog());
				workspace->getLog()->disown();

			} else if (synchronizeTransactions() == true) {
				m_virtualKeySpace->compileKeySpace(true /* publish */);
				m_messageService->publishDataLog(m_subscribers, workspace->getLog());	
			}
		}

		// XXX: status methods

			// XXX: message passing methods
			virtual void sendStatusRequest(PeerInfo* peerInfo, const StatusRequest<K>* statusRequest) const;

		boolean checkOwnerForUpdates(const Segment<K>* segment = null) const;
		void processStatusRequestReceived(const StatusResponse* statusResponse) const;

		void processStatusRequest(const MapFacilitator<K>* requester, const StatusRequest<K>* statusRequest) const;
		uinttype getMaxViewpoint(const K key = (K) Converter<K>::NULL_VALUE, const boolean maxVirtual = true) const;

		const StatusRequest<K>* createRequestForStatus(const Segment<K>* segment = null) const;

		// XXX: utility methods

		void setContextSegment(const Segment<K>* segment) const;
		Segment<K>* getContextSegment() const;

		virtual void setContextResult(longtype result, longtype threadId = 0) const;
		virtual longtype getContextResult() const;

		virtual void setContextCompleted(longtype threadId) const;
		virtual void setContextCompleted(boolean completed) const;
		virtual void waitForResult() const;

		ulongtype getSegmentOperationCount() const;
		void resetSegmentOperationCount();

		void printVirtualRepresentation();
		void printMap();

		virtual ~MapFacilitator() {
			delete m_myInfo;
			delete m_virtualKeySpace;
			delete m_subscribers;
			delete m_segmentDataManager;
			delete m_dataLogManager;
		}
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_MAPFACILITATOR_H_ */
