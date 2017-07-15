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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_LOGENTRY_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_LOGENTRY_H_

#include "cxx/lang/types.h"
#include "cxx/util/Converter.h"
#include "com/deepis/db/store/relative/distributed/VirtualInfo.h"

using namespace cxx::util;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

enum LogEntryAction {
	ACTION_NONE,
	ADD,
	UPDATE,
	REMOVE
};     

template<typename K>
class DataLogEntry {
	private:
		LogEntryAction m_action;
		K m_key;
		uinttype m_viewpoint;
		uinttype m_size;
		bytearray m_data;

	public:
		DataLogEntry() :
			m_action(ACTION_NONE),
			m_key((K)Converter<K>::NULL_VALUE),
			m_viewpoint(0),
			m_size(0),
			m_data(null) {

		}

		DataLogEntry(LogEntryAction action, K key, uinttype viewpoint, uinttype size, bytearray data):
			m_action(action),
			m_key(key),
			m_viewpoint(viewpoint),
			m_size(size),
			m_data(null) {

			if ((size != 0) && (data != null)) {
				m_data = (bytearray) malloc(size);
				memcpy(m_data, data, size);
			}
		}

		FORCE_INLINE ubytetype* getBytes(uinttype* size) const {
			(*size) = sizeof(LogEntryAction) /* action */ +
			       	sizeof(inttype) /* TODO: key types */ +
				sizeof(uinttype) /* viewpoint */ +
				sizeof(uinttype) /* size */ +
				m_size; /* data */;

			ubytetype* bytes = (ubytetype*) malloc(*size);
			
			uinttype pointer = 0;
			memcpy(bytes, &m_action, sizeof(LogEntryAction));
			pointer += sizeof(LogEntryAction);

			// TODO: key types
			memcpy((bytes + pointer), &m_key, sizeof(inttype));
			pointer += sizeof(inttype);
				
			memcpy((bytes + pointer), &m_viewpoint, sizeof(uinttype));
			pointer += sizeof(uinttype);

			memcpy((bytes + pointer), &m_size, sizeof(uinttype));
			pointer += sizeof(uinttype);

			memcpy((bytes + pointer), m_data, m_size);
			pointer += m_size;

			return bytes;
		}

		FORCE_INLINE uinttype setBytes(const ubytetype* bytes) {
			uinttype pointer = 0;

			memcpy(&m_action, bytes, sizeof(LogEntryAction));
			pointer += sizeof(LogEntryAction);

			// TODO: key types
			memcpy(&m_key, (bytes + pointer), sizeof(inttype));
			pointer += sizeof(inttype);

			memcpy(&m_viewpoint, (bytes + pointer), sizeof(uinttype));
			pointer += sizeof(uinttype);

			memcpy(&m_size, (bytes + pointer), sizeof(uinttype));
			pointer += sizeof(uinttype);

			m_data = (bytearray) malloc(m_size);

			memcpy(m_data, (bytes + pointer), m_size);
			pointer += m_size;

			return pointer;
		}

		DataLogEntry<K>* clone() const {
			DataLogEntry<K>* entry = new DataLogEntry<K>(m_action, m_key, m_viewpoint, m_size, m_data);
			return entry;
		}

		virtual ~DataLogEntry() {
			if (m_data != null) {
				free(m_data);
			}
		}

		FORCE_INLINE void setAction(LogEntryAction action) {
			m_action = action;
		}

		FORCE_INLINE LogEntryAction getAction() const {
			return m_action;
		}

		FORCE_INLINE void setKey(K key) {
			m_key = key;
		}

		FORCE_INLINE K getKey() const { 
			return m_key;
		}

		FORCE_INLINE void setViewpoint(uinttype viewpoint) {
			m_viewpoint = viewpoint;
		}

		FORCE_INLINE uinttype getViewpoint() const {
			return m_viewpoint;
		}

		FORCE_INLINE void setSize(uinttype size) {
			m_size = size;
		}

		FORCE_INLINE uinttype getSize() const {
			return m_size;
		}

		FORCE_INLINE void setData(bytearray data) {
			m_data = data;
		}
	
		FORCE_INLINE bytearray getData() const {
			return m_data;
		}

		FORCE_INLINE boolean equals(DataLogEntry<K>* dle) const {
			if (getAction() != dle->getAction()) {
				return false;
			}

			if (Converter<K>::equals(getKey(), dle->getKey()) == false) {
				return false;
			}

			if (getViewpoint() != dle->getViewpoint()) {
				return false;
			}

			if (getSize() != dle->getSize()) {
				return false;
			}

			if (memcmp(getData(), dle->getData(), getSize())) {
				return false;
			}

			return true;
		}
};

template<typename K>
class VirtualLogEntry {
	private:
		LogEntryAction m_action;
		K m_key;
		boolean m_processed;
		inttype m_vsize;
		uinttype m_minViewpoint;
		uinttype m_maxViewpoint;

	public:
		VirtualLogEntry() :
			m_action(ACTION_NONE),
			m_key(Converter<K>::NULL_VALUE),
			m_processed(false),
			m_vsize(0),
			m_minViewpoint(0),
			m_maxViewpoint(0) {

		}

		VirtualLogEntry(LogEntryAction action, K key, VirtualInfo* virtualInfo) :
			m_action(action),
			m_key(key),
			m_processed(false),
			m_vsize(virtualInfo->getVirtualSize()),
			m_minViewpoint(virtualInfo->getMinViewpoint()),
			m_maxViewpoint(virtualInfo->getMaxViewpoint()) {

		}

		VirtualLogEntry(LogEntryAction action, K key, inttype vsize, uinttype minViewpoint, uinttype maxViewpoint):
			m_action(action),
			m_key(key),
			m_processed(false),
			m_vsize(vsize),
			m_minViewpoint(minViewpoint),
			m_maxViewpoint(maxViewpoint) {
		}

		FORCE_INLINE VirtualInfo* createVirtualInfo() const {
			return new VirtualInfo(m_vsize, m_minViewpoint, m_maxViewpoint);
		}

		FORCE_INLINE void setAction(LogEntryAction action) {
			m_action = action;
		}

		FORCE_INLINE LogEntryAction getAction() const {
			return m_action;
		}

		FORCE_INLINE void setKey(K key) {
			m_key = key;
		}

		FORCE_INLINE K getKey() const { 
			return m_key;
		}

		FORCE_INLINE void setProcessed(boolean processed) {
			m_processed = processed;
		}

		FORCE_INLINE boolean getProcessed() const {
			return m_processed;
		}

		FORCE_INLINE void setVirtualSize(inttype vsize) {
			m_vsize = vsize;
		}

		FORCE_INLINE inttype getVirtualSize() const {
			return m_vsize;
		}

		FORCE_INLINE void setMinViewpoint(uinttype viewpoint) {
			m_minViewpoint = viewpoint;
		}

		FORCE_INLINE uinttype getMinViewpoint() const {
			return m_minViewpoint;
		}

		FORCE_INLINE void setMaxViewpoint(uinttype viewpoint) {
			m_maxViewpoint = viewpoint;
		}

		FORCE_INLINE uinttype getMaxViewpoint() const {
			return m_maxViewpoint;
		}

		FORCE_INLINE boolean equals(VirtualLogEntry* vle) const {
			if (getAction() != vle->getAction()) {
				return false;
			}

			if (Converter<K>::equals(getKey(), vle->getKey()) == false) {
				return false;
			}
		
			if (getProcessed() != vle->getProcessed()) {
				return false;
			}

			if (getVirtualSize() != vle->getVirtualSize()) {
				return false;
			}

			if (getMinViewpoint() != vle->getMinViewpoint()) {
				return false;
			}

			if (getMaxViewpoint() != vle->getMaxViewpoint()) {
				return false;
			}

			return true;
		}
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_LOGENTRY_H_ */
