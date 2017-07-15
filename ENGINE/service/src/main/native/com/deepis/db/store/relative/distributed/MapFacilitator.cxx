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
#ifdef DEEP_DISTRIBUTED
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_MAPFACILITATOR_CXX_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_MAPFACILITATOR_CXX_

#include "com/deepis/db/store/relative/util/InvalidException.h"

#include "com/deepis/db/store/relative/core/Properties.h"
#include "com/deepis/db/store/relative/core/RealTimeMap.cxx"
#include "com/deepis/db/store/relative/util/Versions.h"

#include "com/deepis/db/store/relative/distributed/VirtualInfo.h"
#include "com/deepis/db/store/relative/distributed/LogEntry.h"
#include "com/deepis/db/store/relative/distributed/MapFacilitator.h"
#include "com/deepis/db/store/relative/distributed/DataRequest.h"
#include "com/deepis/db/store/relative/distributed/StatusRequest.h"
#include "com/deepis/db/store/relative/distributed/PeerInfo.h"

using namespace com::deepis::db::store::relative::util;
using namespace com::deepis::db::store::relative::core;
using namespace com::deepis::db::store::relative::distributed;

template<typename K>
void MapFacilitator<K>::addSubscriber(PeerInfo* subscriber) {
	#ifdef DEEP_DEBUG
	if (allowRemoteConsumer() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::addsubscriber: consumer not allowed");
	}
	#endif

	if (m_subscribers == null) {
		m_subscribers = new ArrayList<PeerInfo*>(5 /* cap */, false /* delete value */);
	}
	m_subscribers->ArrayList<PeerInfo*>::add(subscriber);

	if ((subscriber->getMapBehavior() == DISTRIBUTED_HA_SYNC_SLAVE) || (subscriber->getMapBehavior() == DISTRIBUTED_HA_ASYNC_SLAVE)) {
		m_synchronizeTransactions = true;
	}
}

template<typename K>
void MapFacilitator<K>::subscribeTo(PeerInfo* owner) {
	#ifdef DEEP_DEBUG
	if (allowRemoteOwner() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::subscribeTo: owner not allowed");
	}
	#endif

	// TODO this will be a collection of owners of various maps / keyspaces
	m_owner = owner;
	requestVirtualRepresentation();
}

template<typename K>
boolean MapFacilitator<K>::subscriberSyncHold() {
	if (allowRemoteConsumer() == false) {
		return false;
	}
	
	if (m_subscribers != null) {
		for (inttype x = 0; x < m_subscribers->ArrayList<PeerInfo*>::size(); x++) {
			PeerInfo* peerInfo = m_subscribers->ArrayList<PeerInfo*>::get(x);

			if ((peerInfo->getMapBehavior() == DISTRIBUTED_HA_SYNC_SLAVE) || (peerInfo->getMapBehavior() == DISTRIBUTED_HA_ASYNC_SLAVE)) {
				longtype version = peerInfo->getVirtualKeySpaceVersion();
				if (version == 0) {
					version = m_messageService->sendVirtualKeySpaceVersionRequest(peerInfo);
					if (version == 0) {
						return true;
					} else {
						peerInfo->setVirtualKeySpaceVersion(version);
					}
				}
			}
		}
	}

	return false;
}

template<typename K>
void MapFacilitator<K>::requestVirtualRepresentation() {
	#ifdef DEEP_DEBUG
	if (allowRemoteOwner() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::requestVirtualRepresentation: owner not allowed");
	}
	#endif

	#if 0
	sendVirtualRepresentationRequest();
	#else
	m_messageService->sendVirtualRepresentationRequest(m_owner);
	#endif
}

template<typename K>
void MapFacilitator<K>::sendVirtualRepresentationRequest() {
	#ifdef DEEP_DEBUG
	if (allowRemoteOwner() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::sendVirtualRepresentationRequest: owner not allowed");
	}
	#endif

	((Facilitator<K>*)(m_owner->getFacilitator()))->processVirtualRepresentationRequest();
}

template<typename K>
void MapFacilitator<K>::processVirtualRepresentationRequest() {
	#ifdef DEEP_DEBUG
	if (allowRemoteConsumer() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::processVirtualRepresentationRequest: consumer not allowed");
	}
	#endif

	m_virtualKeySpace->compileKeySpace(false /*publish*/);
	m_virtualKeySpace->sendVirtualRepresentation();
}

template<typename K>
void MapFacilitator<K>::publishVirtualUpdate(const VirtualLogEntry<K>* logEntry) {
	#ifdef DEEP_DEBUG
	if (allowRemoteConsumer() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::publishVirtualUpdate: consumer not allowed");
	}
	#endif

	if (m_subscribers != null) {
		for (inttype x = 0; x < m_subscribers->ArrayList<PeerInfo*>::size(); x++) {
			MapFacilitator<K>* mapFacilitator = (MapFacilitator<K>*) m_subscribers->ArrayList<PeerInfo*>::get(x)->getFacilitator();
			if (mapFacilitator != null) {
				mapFacilitator->processVirtualUpdate(logEntry);
			}
		}
	}
}

template<typename K>
void MapFacilitator<K>::publishVirtualCommit() {
	#ifdef DEEP_DEBUG
	if (allowRemoteConsumer() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::publishVirtualCommit: nconsumer not allowed");
	}
	#endif

	if (m_subscribers != null) {
		for (inttype x = 0; x < m_subscribers->ArrayList<PeerInfo*>::size(); x++) {
			MapFacilitator<K>* mapFacilitator = (MapFacilitator<K>*) m_subscribers->ArrayList<PeerInfo*>::get(x)->getFacilitator();
			if (mapFacilitator != null) {
				mapFacilitator->processVirtualCommit();
			}
		}
	}
}

template<typename K>
void MapFacilitator<K>::processVirtualUpdate(const VirtualLogEntry<K>* logEntry) {
	#ifdef DEEP_DEBUG
	if (allowRemoteOwner() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::processVirtualUpdate: owner not allowed");
	}
	#endif

	m_virtualKeySpace->processVirtualUpdate(logEntry);
}

template<typename K>
void MapFacilitator<K>::processVirtualCommit() {
	#ifdef DEEP_DEBUG
	if (allowRemoteOwner() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::processVirtualCommit: owner not allowed");
	}
	#endif

	m_virtualKeySpace->commitVirtualChanges();
}

template<typename K>
void MapFacilitator<K>::publishDataLog(QuickList<DataLogEntry<K>*>* dataLog) {
	#ifdef DEEP_DEBUG
	if (allowRemoteConsumer() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::publishDataLog: consumer not allowed");
	}
	#endif

	if (m_subscribers != null) {
		for (inttype x = 0; x < m_subscribers->ArrayList<PeerInfo*>::size(); x++) {
			MapFacilitator<K>* mapFacilitator = (MapFacilitator<K>*) m_subscribers->ArrayList<PeerInfo*>::get(x)->getFacilitator();
			if (mapFacilitator != null) {
				mapFacilitator->processDataTransaction(dataLog);
			}
		}
	}
}

template<typename K>
void MapFacilitator<K>::requestDataForSegment(Segment<K>* segment, longtype version) {
	#ifdef DEEP_DEBUG
	if (allowRemoteOwner() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::requestDataForSegment: owner not allowed");
	}
	#endif

	// XXX: if virtual there may not be any new data, first check if there is (cheaper operation)
	if (segment->getVirtual() == false) {
		if (checkOwnerForUpdates(segment) == false) {
			return;
		}
	}

	const DataRequest<K>* dataRequest = m_segmentDataManager->createRequestForSegment(segment, version);

	// XXX: this thread needs to block / continue to retry or something and hold the segment in context
	setContextSegment(segment);

	m_messageService->sendDataRequest(m_owner, dataRequest);

	Converter<K>::destroy(dataRequest->getKey());
	delete dataRequest;
}

template<typename K>
void MapFacilitator<K>::sendDataRequest(PeerInfo* peerInfo, const DataRequest<K>* dataRequest) {
	// TODO: remove these casts - add methods to parent Facilitator
	return ((MapFacilitator<K>*)(peerInfo->getFacilitator()))->processDataRequest(this, dataRequest);
}

template<typename K>
void MapFacilitator<K>::processDataRequest(MapFacilitator<K>* requester, const DataRequest<K>* dataRequest) {
	// process change specific data request
	QuickList<DataLogEntry<K>*>* logData = m_dataLogManager->getChangeSet(dataRequest->getMaxRealViewpoint(), dataRequest->getMaxVirtualViewpoint());	

	// process segment specific data request
	QuickList<DataLogEntry<K>*>* segmentData = m_segmentDataManager->processRequest(dataRequest);

	if (logData != null) {
		logData->append(segmentData);
		segmentData->disown();
		delete segmentData;
	
		// TODO only if all peers have retrieved this data
		m_dataLogManager->clearLog();

		requester->processDataReceived(logData);
		return;
	}

	requester->processDataReceived(segmentData);
}

template<typename K>
void MapFacilitator<K>::processDataTransaction(QuickList<DataLogEntry<K>*>* data) {
	m_segmentDataManager->processDataLog(data, null /* segment */ , false /* destroy */);
}

template<typename K>
void MapFacilitator<K>::processDataReceived(QuickList<DataLogEntry<K>*>* data) {
	m_segmentDataManager->processDataLog(data, getContextSegment(), true /* destroy */);
}

template<typename K>
boolean MapFacilitator<K>::checkOwnerForUpdates(const Segment<K>* segment) const {
	#ifdef DEEP_DEBUG
	if (allowRemoteOwner() == false) {
		throw InvalidException("Invalid MapFacilitator<K>::checkOwnerForUpdates: owner not allowed");
	}
	#endif

	setContextSegment(segment);
	const StatusRequest<K>* statusRequest = createRequestForStatus(segment);

	m_messageService->sendStatusRequest(m_owner, statusRequest);

	delete statusRequest;

	return getContextResult();
}

template<typename K>
void MapFacilitator<K>::processStatusRequestReceived(const StatusResponse* statusResponse) const {
	boolean updates = false;
	Segment<K>* segment = getContextSegment();

	if (segment == null) {
		updates = (statusResponse->getMaxViewpoint() > getMaxViewpoint());

	} else {
		updates = (statusResponse->getMaxViewpoint() > segment->getMaxRealViewpoint());
	}

	delete statusResponse;

	setContextResult(updates ? (longtype)1 : (longtype)0);
}

template<typename K>
void MapFacilitator<K>::sendStatusRequest(PeerInfo* peerInfo, const StatusRequest<K>* statusRequest) const {
	// TODO: remove these casts - add methods to parent Facilitator
	((MapFacilitator<K>*)(peerInfo->getFacilitator()))->processStatusRequest(this, statusRequest);
}

template<typename K>
void MapFacilitator<K>::processStatusRequest(const MapFacilitator<K>* requester, const StatusRequest<K>* statusRequest) const {
	StatusResponse* statusResponse = new StatusResponse();

	statusResponse->setMaxViewpoint(getMaxViewpoint(statusRequest->getKey()));
	requester->processStatusRequestReceived(statusResponse);
}

template<typename K>
const StatusRequest<K>* MapFacilitator<K>::createRequestForStatus(const Segment<K>* segment) const {
	StatusRequest<K>* statusRequest = new StatusRequest<K>();

	if (segment == null) {
		statusRequest->setKey((K) Converter<K>::NULL_VALUE);

	} else {
		statusRequest->setKey(segment->RealTimeTypes<K>::SegTreeMap::firstKey());
	}

	return statusRequest;
}

template<typename K>
uinttype MapFacilitator<K>::getMaxViewpoint(const K key, const boolean virtualMax) const {
	if (key == Converter<K>::NULL_VALUE) {
		return m_map->getMaxViewpoint();

	} else {
		Segment<K>* segment = m_segmentDataManager->getSegment(key);
		segment->unlock();
		if (virtualMax == true) {
			// TODO: do we want this or non-committed max?
			return VirtualKeySpace<K>::isolateMaxVirtualViewpoint(segment, getVirtualKeySpaceVersion());
		} else {
			return segment->getMaxRealViewpoint();
		}
	}
}

template<typename K>
void MapFacilitator<K>::setContextSegment(const Segment<K>* segment) const {
	ThreadContext<K>* ctxt = m_map->m_threadContext.getContext();
	ctxt->setDistributedSegment(segment);
}

template<typename K>
Segment<K>* MapFacilitator<K>::getContextSegment() const {
	ThreadContext<K>* ctxt = m_map->m_threadContext.getContext();
	return ctxt->getDistributedSegment();
}

template<typename K>
void MapFacilitator<K>::setContextResult(longtype result, longtype threadId) const {
	ThreadContext<K>* ctxt = null;
	if (threadId == 0) {
		ctxt = m_map->m_threadContext.getContext();

	} else {
		ctxt = m_map->m_threadContext.getContext(threadId);
	}
	ctxt->setDistributedResult(result);
}

template<typename K>
longtype MapFacilitator<K>::getContextResult() const {
	ThreadContext<K>* ctxt = m_map->m_threadContext.getContext();
	return ctxt->getDistributedResult();
}

template<typename K>
void MapFacilitator<K>::setContextCompleted(longtype threadId) const {
	ThreadContext<K>* ctxt = m_map->m_threadContext.getContext(threadId);
	ctxt->setDistributedCompleted(true);
}

template<typename K>
void MapFacilitator<K>::setContextCompleted(boolean completed) const {
	ThreadContext<K>* ctxt = m_map->m_threadContext.getContext();
	ctxt->setDistributedCompleted(completed);
}

template<typename K>
void MapFacilitator<K>::waitForResult() const {
	ThreadContext<K>* ctxt = m_map->m_threadContext.getContext();
	while (ctxt->getDistributedCompleted() == false) {
		// TODO this should not be a constant spin wait
	}
	// TODO : check for error codes / result in response
}

template<typename K>
void MapFacilitator<K>::resetSegmentOperationCount() {
	m_segmentDataManager->resetOperationCount();
}

template<typename K>
ulongtype MapFacilitator<K>::getSegmentOperationCount() const {
	return m_segmentDataManager->getOperationCount();
}

template<typename K>
void MapFacilitator<K>::printVirtualRepresentation() {
	m_virtualKeySpace->compileKeySpace();
	m_virtualKeySpace->printKeySpace();
}

template<typename K>
void MapFacilitator<K>::printMap() {
	m_map->printSegments();
}

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_MAPFACILITATOR_CXX_ */
#endif
