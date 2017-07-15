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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_VIRTUALKEYSPACE_CXX_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_VIRTUALKEYSPACE_CXX_

#include "com/deepis/db/store/relative/util/InvalidException.h"

#include "com/deepis/db/store/relative/core/RealTimeMap.h"
#include "com/deepis/db/store/relative/distributed/MapFacilitator.h"
#include "com/deepis/db/store/relative/distributed/VirtualKeySpace.h"

using namespace cxx::util;
using namespace com::deepis::db::store::relative::core;
using namespace com::deepis::db::store::relative::util;
using namespace com::deepis::db::store::relative::distributed;

template<typename K>
void VirtualKeySpace<K>::addLogEntry(LogEntryAction action, K key, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint) {
	VirtualLogEntry<K>* entry = new VirtualLogEntry<K>(action, key, vsize, minViewpoint, maxViewpoint);
	m_virtualLog.addEntry(entry);
}

template<typename K>
void VirtualKeySpace<K>::publishVirtualUpdate(const VirtualLogEntry<K>* logEntry) {
	m_mapFacilitator->getMessageService()->publishVirtualUpdate(m_mapFacilitator->getSubscribers(), logEntry);
}

template<typename K>
void VirtualKeySpace<K>::publishVirtualCommit() {
	m_mapFacilitator->getMessageService()->publishVirtualCommit(m_mapFacilitator->getSubscribers());
}

template<typename K>
void VirtualKeySpace<K>::compileKeySpace(boolean publish) {
	if (m_compiledKeySpace == null) {
		m_compiledKeySpace = new TreeMap<K,VirtualInfo*>(m_keyComparator, 
				TreeMap<K,VirtualInfo*>::INITIAL_ORDER /* order */, true /* del key */, true /* del value */);
	}

	QuickListEntry<VirtualLogEntry<K>*>* head = m_virtualLog.getHead();
	VirtualLogEntry<K>* entry = null;
	boolean updateSent = false;

	while ((head != null) && ((head->getEntry()->getProcessed() == false) || (head->getNext() != null))) {
		entry = head->getEntry();

		if (entry->getProcessed() == false) {
			if (publish == true) {
				publishVirtualUpdate(entry);
				updateSent = true;
			}

			switch (entry->getAction()) {
				case ADD:
					m_compiledKeySpace->put(entry->getKey(), entry->createVirtualInfo());	
					break;
				case UPDATE:
					{
						VirtualInfo* virtualInfo = (VirtualInfo*) m_compiledKeySpace->floorEntry(entry->getKey())->getValue();
						virtualInfo->setVirtualSize(entry->getVirtualSize());
						virtualInfo->setMinViewpoint(entry->getMinViewpoint());
						virtualInfo->setMaxViewpoint(entry->getMaxViewpoint());
						
						Converter<K>::destroy(entry->getKey());
						break;
					}
				case REMOVE:
					{
						VirtualInfo* virtualInfo = m_compiledKeySpace->remove(entry->getKey());
						Converter<K>::destroy(entry->getKey());
						delete virtualInfo;

						break;
					}
				default:
					break;
			}
		}
		entry->setProcessed(true);
		m_virtualLog.advanceHead();

		head = m_virtualLog.getHead();
	}

	if (updateSent == true) {
		publishVirtualCommit();
	}
}

template<typename K>
void VirtualKeySpace<K>::sendVirtualRepresentation() {
	typename TreeMap<K,VirtualInfo*>::TreeMapEntrySet entrySet(true);
        typedef typename TreeMap<K,VirtualInfo*>::TreeMapEntrySet::EntrySetIterator KeySpaceEntrySetIterator;

        m_compiledKeySpace->entrySet(&entrySet);
        KeySpaceEntrySetIterator* iter = (KeySpaceEntrySetIterator*) entrySet.reset();

        while (iter->hasNext()) {
                MapEntry<K,VirtualInfo*>* entry = iter->next();
		
		K key = m_map->m_keyBuilder->cloneKey(entry->getKey());
                VirtualInfo* value = entry->getValue();
		
		VirtualLogEntry<K>* logEntry = new VirtualLogEntry<K>(ADD, key, value);
		publishVirtualUpdate(logEntry);

		delete logEntry;
        }

	publishVirtualCommit();
}

template<typename K>
void VirtualKeySpace<K>::processVirtualUpdate(const VirtualLogEntry<K>* logEntry) {
        switch (logEntry->getAction()) {
                case ADD:
                        {
                                Segment<K>* segment = createSegment(logEntry->getKey(), logEntry->getVirtualSize(), logEntry->getMinViewpoint(), logEntry->getMaxViewpoint());
                                m_map->addVirtualSegment(segment);
                                break;
                        }      
                case UPDATE:   
                        {
                                K key = logEntry->getKey();
                                inttype size = logEntry->getVirtualSize();
                                uinttype minvp = logEntry->getMinViewpoint();
                                uinttype maxvp = logEntry->getMaxViewpoint();
                               
                                m_map->updateVirtualSegment(key, size, minvp, maxvp);
                                break;
                        }      
                case REMOVE:   
                        break; 
                default:
                        break;
        }              
}

template<typename K>
uinttype VirtualKeySpace<K>::isolateMinViewpoint(const Segment<K>* segment, longtype version) {
	if ((version == 0) && (segment->getRemoteOwner() == true)) {
		throw InvalidException("Invalid VirtualKeySpace<K>::isolateMinViewpoint: remote owned with no version");
	}

	QuickList<VirtualInfo*>* storyLine = segment->getVirtualStoryLine();
	if (storyLine == null) {
		return 0;
	}

	uinttype minViewpoint = 0;
	QuickListEntry<VirtualInfo*>* head = storyLine->getHead();

	while (head != null) {
		if (head->getEntry()->getVersion() <= version) {
			minViewpoint = head->getEntry()->getMinViewpoint();
		} else {
			break;
		}
		head = head->getNext();
	}

	return minViewpoint;
}

template<typename K>
uinttype VirtualKeySpace<K>::isolateMaxVirtualViewpoint(const Segment<K>* segment, longtype version) {
	if ((version == 0) && (segment->getRemoteOwner() == true)) {
		throw InvalidException("Invalid VirtualKeySpace<K>::isolateMaxVirtualViewpoint: remote owned with no version");
	}

	QuickList<VirtualInfo*>* storyLine = segment->getVirtualStoryLine();
	if (storyLine == null) {
		return 0;
	}

	uinttype maxViewpoint = 0;
	QuickListEntry<VirtualInfo*>* head = storyLine->getHead();

	while (head != null) {
		if (head->getEntry()->getVersion() <= version) {
			maxViewpoint = head->getEntry()->getMaxViewpoint();
		} else {
			break;
		}
		head = head->getNext();
	}

	return maxViewpoint;
}

template<typename K>
inttype VirtualKeySpace<K>::isolateVirtualSize(const Segment<K>* segment, longtype version) {
	if ((version == 0) && (segment->getRemoteOwner() == true)) {
		throw InvalidException("Invalid VirtualKeySpace<K>::isolateVirtualSize: remote owned with no version");
	}

	QuickList<VirtualInfo*>* storyLine = segment->getVirtualStoryLine();
	if (storyLine == null) {
		return 0;
	}

	inttype vsize = 0;
	QuickListEntry<VirtualInfo*>* head = storyLine->getHead();

	while (head != null) {
		if (head->getEntry()->getVersion() <= version) {
			vsize = head->getEntry()->getVirtualSize();
		} else {
			break;
		}
		head = head->getNext();
	}

	return vsize;
}

template<typename K>
boolean VirtualKeySpace<K>::versionKeySpace(Segment<K>* segment, longtype version, inttype vsize, uinttype minViewpoint, uinttype maxVirtualViewpoint) {
	if ((version == 0) && (segment->getRemoteOwner() == true)) {
		throw InvalidException("Invalid VirtualKeySpace<K>::versionKeySpace: remote owned with no version");
	}
	if (segment->getVirtualStoryLine() == null) {
		segment->initVirtualStoryLine();
	}

	QuickList<VirtualInfo*>* storyLine = segment->getVirtualStoryLine();
	QuickListEntry<VirtualInfo*>* head = storyLine->getHead();

	while ((head != null) && (head->getEntry()->getVersion() != version)) {
		head = head->getNext();
	}

	if (head == null) {
		VirtualInfo* virtualInfo = new VirtualInfo(vsize, minViewpoint, maxVirtualViewpoint, version);
		storyLine->addEntry(virtualInfo);
		return true;

	} else {
		VirtualInfo* entry = head->getEntry();

		boolean changed = false;
		if ((entry->getMinViewpoint() == 0) || (minViewpoint < entry->getMinViewpoint())) {
			entry->setMinViewpoint(minViewpoint);
			changed = true;
		}
		if (maxVirtualViewpoint > entry->getMaxViewpoint()) {
			entry->setMaxViewpoint(maxVirtualViewpoint);
			changed = true;
		}
		if (entry->getVirtualSize() != vsize) {
			entry->setVirtualSize(vsize);
			changed = true;
		}

		return changed;
	}
}

template<typename K>
const Information* VirtualKeySpace<K>::isolateInformation(const Information* orginfo, uinttype minViewpoint, uinttype maxViewpoint) {
	const Information* info = null;
	const Information* tmpinfo = orginfo;

	while ((tmpinfo != null) && (tmpinfo->getLevel() == Information::LEVEL_COMMIT)) {
		if (tmpinfo->getViewpoint() <= maxViewpoint) {
			info = tmpinfo;
		}
		
		tmpinfo = tmpinfo->getNext();
		
		if (tmpinfo == orginfo) {
			break;
		}
	}
	
	return info;
}

template<typename K>
Segment<K>* VirtualKeySpace<K>::createSegment(const K key, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint) const {
	Segment<K>* segment = null;

	segment = new Segment<K>(m_map->m_comparator, Properties::DEFAULT_SEGMENT_LEAF_ORDER, Versions::GET_PROTOCOL_CURRENT(), m_map->m_keyBuilder->getKeyParts());
	segment->setMapContext(m_map->m_indexValue);
	segment->put(m_map->m_keyBuilder->cloneKey(key), null);

	segment->vsize(vsize);
	if (vsize > 0) {
		segment->setVirtual(true);
	}
	segment->setPaged(false);

	versionKeySpace(segment, getKeySpaceVersion(), vsize, minViewpoint, maxViewpoint);

	// TODO this is probably some kind of guid to identify key space owner in the fabric
	segment->setRemoteOwner(true);

	return segment;
}

template<typename K>
void VirtualKeySpace<K>::printKeySpace() const {
	typename TreeMap<K,VirtualInfo*>::TreeMapEntrySet entrySet(true);
	typedef typename TreeMap<K,VirtualInfo*>::TreeMapEntrySet::EntrySetIterator KeySpaceEntrySetIterator;

	m_compiledKeySpace->entrySet(&entrySet);
	KeySpaceEntrySetIterator* iter = (KeySpaceEntrySetIterator*) entrySet.reset();

	while (iter->hasNext()) {
		MapEntry<K,VirtualInfo*>* entry = iter->next();

		printf("\n----KEY %d V-SIZE %d MIN VIEWPOINT %d MAX VIEWPOINT %d ", entry->getKey(), entry->getValue()->getVirtualSize(), entry->getValue()->getMinViewpoint(), entry->getValue()->getMaxViewpoint());
	}
	printf("\n");
}

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_VIRTUALKEYSPACE_CXX_ */
#endif
