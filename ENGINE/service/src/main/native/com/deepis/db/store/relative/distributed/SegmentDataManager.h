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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_SEGMENTDATAMANAGER_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_SEGMENTDATAMANAGER_H_

#include "com/deepis/db/store/relative/core/RealTimeMap.h"
#include "com/deepis/db/store/relative/core/RealTimeBuilder.h"

#include "com/deepis/db/store/relative/distributed/DataRequest.h"
#include "com/deepis/db/store/relative/distributed/LogEntry.h"
#include "com/deepis/db/store/relative/distributed/QuickList.h"

using namespace com::deepis::db::store::relative::core;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

template<typename K>
struct SegmentDataManager {
	private:
		RealTimeMap<K>* m_map;
		const KeyBuilder<K>* m_keyBuilder;
	
		ulongtype m_operationCount;

	public:
		SegmentDataManager(RealTimeMap<K>* map) :
			m_map(map),
			m_keyBuilder(map->m_keyBuilder),
			m_operationCount(0) {

			}

		const DataRequest<K>* createRequestForSegment(const Segment<K>* segment, longtype version) const;

		ulongtype getOperationCount() const;
		void resetOperationCount();
		void processDataLog(QuickList<DataLogEntry<K>*>* dataLog, Segment<K>* segment, boolean destroy);

		QuickList<DataLogEntry<K>*>* processRequest(const DataRequest<K>* dataRequest);
		Segment<K>* getSegment(K key);

	private:

		Transaction* manageTransaction(ThreadContext<K>* ctxt, Transaction* tx, uinttype* currentViewpoint, DataLogEntry<K>* logEntry);
		void processDataLogEntry(DataLogEntry<K>* logEntry, Segment<K>* segment, ThreadContext<K>* ctxt, Transaction* tx);
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_SEGMENTDATAMANAGER_H_ */
