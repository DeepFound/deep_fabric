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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_DATALOGMANAGER_CXX_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_DATALOGMANAGER_CXX_

#include "com/deepis/db/store/relative/util/InvalidException.h"
#include "com/deepis/db/store/relative/core/Properties.h"

#include "com/deepis/db/store/relative/distributed/DataLogManager.h"

using namespace com::deepis::db::store::relative::distributed;

template<typename K>
void DataLogManager<K>::logChangeSet(QuickList<DataLogEntry<K>*>* changeSet) {
	m_log.append(changeSet);
}

template<typename K>
void DataLogManager<K>::logDelete(const K key, uinttype viewpoint, uinttype size, bytearray data) {
	DataLogEntry<K>* logEntry = new DataLogEntry<K>(REMOVE, key, viewpoint, size, data);
	m_log.addEntry(logEntry);
}

template<typename K>
QuickList<DataLogEntry<K>*>* DataLogManager<K>::getChangeSet(uinttype minViewpoint, uinttype maxViewpoint) {
	// if change set changes are all before min requested viewpoint, no changes to send
	if (getMaxViewpoint() < minViewpoint) {
		return null;
	}

	// TODO this is an adaptive piece, instead of sending all previous data, mark remote rebuild, or possibly don't send them
	#if 0
	// if change set changes are all after max requested viewpoint, no changes to send
	if (getMinViewpoint() > maxViewpoint) {
		return null;
	}
	#endif

	QuickListEntry<DataLogEntry<K>*>* node = m_log.getHead();
	QuickList<DataLogEntry<K>*>* changeSet = null;

	while ((node != null) && (node->getEntry()->getViewpoint() <= maxViewpoint)) {
		if (changeSet == null) {
			changeSet = new QuickList<DataLogEntry<K>*>();
		}

		changeSet->addEntry(node->getEntry()->clone());
		node = node->getNext();
	}

	return changeSet;
}

template<typename K>
void DataLogManager<K>::clearLog() {
	m_log.clear();
}

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_DATALOGMANAGER_CXX_ */
#endif
