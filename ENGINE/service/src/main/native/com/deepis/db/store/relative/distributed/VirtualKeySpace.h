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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_VIRTUALKEYSPACE_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_VIRTUALKEYSPACE_H_

#include "cxx/util/MapEntry.h"  
#include "cxx/util/concurrent/atomic/AtomicLong.h"

// cxx included due to some invalid inlining without definition in TreeMap which generates linker error
#include "cxx/util/TreeMap.h"
#include "cxx/util/TreeMap.cxx"

#include "com/deepis/db/store/relative/core/RealTimeMap.h"
#include "com/deepis/db/store/relative/core/Information.h"
#include "com/deepis/db/store/relative/core/Segment.h"

#include "com/deepis/db/store/relative/distributed/VirtualInfo.h"
#include "com/deepis/db/store/relative/distributed/QuickList.h"

using namespace cxx::util;
using namespace cxx::util::concurrent::locks;
using namespace cxx::util::concurrent::atomic;
using namespace com::deepis::db::store::relative::core;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

template<typename K>
class MapFacilitator;

template<typename K>
class VirtualKeySpace {

	private:
		MapFacilitator<K>* m_mapFacilitator;
		RealTimeMap<K>* m_map;

		QuickList<VirtualLogEntry<K>*> m_virtualLog;

		const Comparator<K>* m_keyComparator;
		TreeMap<K,VirtualInfo*>* m_compiledKeySpace;

		AtomicLong m_keySpaceVersion;

		void addLogEntry(LogEntryAction action, K key, inttype vsize = 0, uinttype minViewpoint = 0, uinttype maxViewpoint = 0);
		void publishVirtualUpdate(const VirtualLogEntry<K>* logNode);
		void publishVirtualCommit();

		Segment<K>* createSegment(const K key, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint) const;

	public:
		VirtualKeySpace(MapFacilitator<K>* mapFacilitator, RealTimeMap<K>* map, const Comparator<K>* keyComparator):
			m_mapFacilitator(mapFacilitator),
			m_map(map),
			m_virtualLog(),
			m_keyComparator(keyComparator),
			m_compiledKeySpace(null),
			m_keySpaceVersion(0) {
		}

		virtual ~VirtualKeySpace() {
			if (m_compiledKeySpace != null) {
				delete m_compiledKeySpace;
			}
		}

		FORCE_INLINE void addKey(K key, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint) {
			addLogEntry(ADD, key, vsize, minViewpoint, maxViewpoint);
		}

		FORCE_INLINE void updateKey(K key, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint) {
			addLogEntry(UPDATE, key, vsize, minViewpoint, maxViewpoint);
		}

		FORCE_INLINE void removeKey(K key) {
			addLogEntry(REMOVE, key);
		}

		FORCE_INLINE void appendVirtualLog(QuickList<VirtualLogEntry<K>*>* virtualLog) {
			m_virtualLog.append(virtualLog);
		}

		FORCE_INLINE void commitVirtualChanges() {
			m_keySpaceVersion.incrementAndGet();
		}

		FORCE_INLINE longtype getKeySpaceVersion() const {
			return m_keySpaceVersion.get();
		}

		static uinttype isolateMinViewpoint(const Segment<K>* segment, longtype version);
		static uinttype isolateMaxVirtualViewpoint(const Segment<K>* segment, longtype version);
		static inttype isolateVirtualSize(const Segment<K>* segment, longtype version);
		static boolean versionKeySpace(Segment<K>* segment, longtype version, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint);
		static const Information* isolateInformation(const Information* orginfo, uinttype minViewpoint, uinttype maxViewpoint);

		void sendVirtualRepresentation();
		void processVirtualUpdate(const VirtualLogEntry<K>* logEntry);

		void compileKeySpace(boolean publish = true);
		void printKeySpace () const;
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_VIRTUALKEYSPACE_H_ */
