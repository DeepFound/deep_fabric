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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_SEGMENTDATAMANAGER_CXX_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_SEGMENTDATAMANAGER_CXX_

#include "com/deepis/db/store/relative/util/InvalidException.h"

#include "com/deepis/db/store/relative/core/Properties.h"
#include "com/deepis/db/store/relative/core/RealTimeTypes.h"
#include "com/deepis/db/store/relative/core/RealTimeMap.cxx"
#include "com/deepis/db/store/relative/core/RealTimeBuilder.h"

#include "com/deepis/db/store/relative/distributed/VirtualKeySpace.h"
#include "com/deepis/db/store/relative/distributed/SegmentDataManager.h"
#include "com/deepis/db/store/relative/distributed/DataRequest.h"
#include "com/deepis/db/store/relative/distributed/LogEntry.h"

using namespace com::deepis::db::store::relative::util;
using namespace com::deepis::db::store::relative::core;
using namespace com::deepis::db::store::relative::distributed;

template<typename K>
const DataRequest<K>* SegmentDataManager<K>::createRequestForSegment(const Segment<K>* segment, longtype version) const {
	DataRequest<K>* dataRequest = new DataRequest<K>();

	if (segment->getVirtual() == true) {
		dataRequest->setRequestType(FULL);
	} else {
		dataRequest->setRequestType(PARTIAL);
	}

	dataRequest->setKey(m_keyBuilder->cloneKey(segment->firstKey()));
	dataRequest->setVirtualSize(VirtualKeySpace<K>::isolateVirtualSize(segment, version));
	dataRequest->setMaxVirtualViewpoint(VirtualKeySpace<K>::isolateMaxVirtualViewpoint(segment, version));
	dataRequest->setMaxRealViewpoint(segment->getMaxRealViewpoint());

	return dataRequest;
}

template<typename K>
ulongtype SegmentDataManager<K>::getOperationCount() const {
	return m_operationCount;
}

template<typename K>
void SegmentDataManager<K>::resetOperationCount() {
	m_operationCount = 0;
}

template<typename K>
void SegmentDataManager<K>::processDataLog(QuickList<DataLogEntry<K>*>* dataLog, Segment<K>* segment, boolean destroy) {
	if ((dataLog != null) && (dataLog->getHead() != null)) {
		uinttype currentViewpoint = 0;

		ThreadContext<K>* ctxt = m_map->m_threadContext.getContext();
		Transaction* tx = null;

		// XXX: setCardinalityEnabled / setVirtualSizeEnabled
		if (segment != null) {
			segment->RealTimeTypes<K>::SegTreeMap::setStatisticsEnabled(false);
		}

		QuickListEntry<DataLogEntry<K>*>* node = dataLog->getHead();
		while (node != null) {
			DataLogEntry<K>* logEntry = node->getEntry();

			tx = manageTransaction(ctxt, tx, &currentViewpoint, logEntry);
			processDataLogEntry(logEntry, segment, ctxt, tx);

			node = node->getNext();
		}

		// XXX: note if GET (lazy), this is committing at a level above the related get, if there is one (get will do final commit)
		tx->commit(tx->getLevel());
		tx->setCheckRemoteOwner(true);

		// XXX: commit if not a GET transaction (put is coming in sync from master)
		if (tx->getLevel() == -1) {
			Transaction::destroy(tx);
			ctxt->setTransaction(null);
		}

		if (segment != null) {
			segment->RealTimeTypes<K>::SegTreeMap::setStatisticsEnabled(true);
		}
	}

	if ((destroy == true) && (dataLog != null)) {
		delete dataLog;
	}
}

template<typename K>
Transaction* SegmentDataManager<K>::manageTransaction(ThreadContext<K>* ctxt, Transaction* tx, uinttype* currentViewpoint, DataLogEntry<K>* logEntry) {
	if ((tx == null) || (logEntry->getViewpoint() != (*currentViewpoint))) {
		if (tx != null) {
			tx->commit(tx->getLevel());
		}

		tx = ctxt->getTransaction(); 
		if (tx == null) {
			tx = Transaction::create();
			#ifdef CXX_LANG_MEMORY_DEBUG
			ctxt->setTransaction(tx, true);
			#else
			ctxt->setTransaction(tx);
			#endif
		}
		
		tx->setCheckRemoteOwner(false);
		tx->setIsolation(Transaction::COMMITTED);
		tx->begin();
		m_map->associate(tx);
	}
	
	(*currentViewpoint) = logEntry->getViewpoint();
	tx->setCommitViewpoint((*currentViewpoint));

	return tx;
}

template<typename K>
void SegmentDataManager<K>::processDataLogEntry(DataLogEntry<K>* logEntry, Segment<K>* segment, ThreadContext<K>* ctxt, Transaction* tx) {

	#ifdef DEEP_DEBUG
	if (segment != null) {
		if (logEntry->getViewpoint() > VirtualKeySpace<K>::isolateMaxVirtualViewpoint(segment, tx->getVirtualKeySpaceVersion())) {
			throw InvalidException("Invalid SegmentDataManager<K>::processDataLogEntry: data viewpoint ahead of virtual viewpoint");
		}
	}
	#endif

	switch (logEntry->getAction()) {
		case ADD:
			{
				nbyte tmpValue(logEntry->getData(), logEntry->getSize());
				if (m_map->putTransaction(logEntry->getKey(), &tmpValue, RealTimeMap<K>::UNIQUE, 
					tx, RealTimeMap<K>::LOCK_WRITE, 0 /* position */, 0 /* index */, Information::OFFSET_NONE, segment) == false) { 
						throw InvalidException("Invalid SegmentDataManager<K>::processDataLogEntry: ADD putTransaction failed");
				}

				break;
			}

		case UPDATE:
			{
				if ((segment != null) && (segment->containsKey(logEntry->getKey()) == false)) {
					segment = null;
				}

				nbyte tmpValue(logEntry->getData(), logEntry->getSize());
				if (m_map->putTransaction(logEntry->getKey(), &tmpValue, RealTimeMap<K>::EXISTING, 
					tx, RealTimeMap<K>::LOCK_WRITE, 0 /* position */, 0 /* index */, Information::OFFSET_NONE, segment) == false) { 
						throw InvalidException("Invalid SegmentDataManager<K>::processDataLogEntry: UPDATE putTransaction failed");
				}

				break;
			}

		case REMOVE:
			{
				if ((segment != null) && (segment->containsKey(logEntry->getKey()) == false)) {
					segment = null;
				}

				nbyte tmpValue(logEntry->getData(), logEntry->getSize());
				if (m_map->removeTransaction(logEntry->getKey(), &tmpValue, RealTimeMap<K>::DELETE_POPULATED, 
					tx, RealTimeMap<K>::LOCK_WRITE, false /* compressedUpdate */, segment) == false) { 
						throw InvalidException("Invalid SegmentDataManager<K>::processDataLogEntry: REMOVE removeTransaction failed");
				}

				break;
			}
		default:
			throw InvalidException("Invalid SegmentDataManager<K>::processDataLogEntry: invalid operation");
			break;
	}

	if (segment == null) {
		segment = ctxt->getSegment();
		ctxt->setSegment(null);
	}

	if (logEntry->getViewpoint() > segment->getMaxRealViewpoint()) {
		segment->setMaxRealViewpoint(logEntry->getViewpoint());
	}

	m_operationCount++;
}

template<typename K>
QuickList<DataLogEntry<K>*>* SegmentDataManager<K>::processRequest(const DataRequest<K>* dataRequest) {
	Segment<K>* segment = getSegment(dataRequest->getKey());

	if (dataRequest->getRequestType() == PARTIAL) {
		// XXX: client data is already up to date
		if (dataRequest->getMaxRealViewpoint() == segment->getMaxRealViewpoint()) {
			segment->unlock();
			return null;
		}
	}

	QuickList<DataLogEntry<K>*>* data = new QuickList<DataLogEntry<K>*>();

	typedef typename RealTimeTypes<K>::SegTreeMap::TreeMapEntrySet::EntrySetIterator MapInformationEntrySetIterator;
	typename RealTimeTypes<K>::SegTreeMap::TreeMapEntrySet informationSet(true);

	segment->RealTimeTypes<K>::SegTreeMap::entrySet(&informationSet);
	InformationIterator<K> infoIter((MapInformationEntrySetIterator*) informationSet.iterator());

	const typename RealTimeTypes<K>::SegMapEntry* infoEntry = infoIter.next();

	while (infoEntry != null) {
		const Information* info = VirtualKeySpace<K>::isolateInformation(infoEntry->getValue(), 0 /* min viewpoint */, dataRequest->getMaxVirtualViewpoint());

		// XXX: can't take anything beyond the client's virtual representation
		if (dataRequest->getMaxVirtualViewpoint() >= info->getViewpoint()) {
		
			// XXX: only take what the client doesn't already have
			if ((dataRequest->getRequestType() == FULL) || (dataRequest->getMaxRealViewpoint() < info->getViewpoint())) {
				const K key = infoEntry->getKey();

				DataLogEntry<K>* logEntry = new DataLogEntry<K>(ADD, m_keyBuilder->cloneKey(key), info->getViewpoint(), info->getSize(), info->getData());
				data->addEntry(logEntry);
			}
		}

		infoEntry = infoIter.next();
	}

	segment->unlock();

	return data;
}

template<typename K>
Segment<K>* SegmentDataManager<K>::getSegment(K key) {
	ThreadContext<K>* ctxt = m_map->m_threadContext.getContext();
	Segment<K>* segment = m_map->getSegment(ctxt, key, false /* create */, true /* fill */);

	return segment;
}

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_SEGMENTDATAMANAGER_CXX_ */
#endif
