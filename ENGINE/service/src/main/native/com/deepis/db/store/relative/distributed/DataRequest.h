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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_DATAREQUEST_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_DATAREQUEST_H_

#include "cxx/util/Converter.h"
#include "cxx/lang/types.h"

using namespace cxx::util;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

enum DataRequestType {
	REQUEST_NONE,
	VIRTUAL,
	FULL,
	PARTIAL
};

template<typename K>
struct DataRequest {
	private:
		DataRequestType m_requestType;

		K m_key;
		inttype m_vsize;
		uinttype m_maxVirtualViewpoint;
		uinttype m_maxRealViewpoint;

	public:

		DataRequest() :
			m_requestType(REQUEST_NONE),
			m_key((K) Converter<K>::NULL_VALUE),
			m_vsize(0),
			m_maxVirtualViewpoint(0),
			m_maxRealViewpoint(0) { 
		}

		DataRequest(DataRequestType requestType, K key, inttype vsize, uinttype maxVirtualViewpoint, uinttype maxRealViewpoint) :
			m_requestType(requestType),
			m_key(key),
			m_vsize(vsize),
			m_maxVirtualViewpoint(maxVirtualViewpoint),
			m_maxRealViewpoint(maxRealViewpoint) {

			}

		FORCE_INLINE void setRequestType(DataRequestType requestType) {
			m_requestType = requestType;
		}

		FORCE_INLINE DataRequestType getRequestType() const {
			return m_requestType;
		}

		FORCE_INLINE void setKey(K key) {
			m_key = key;
		}

		FORCE_INLINE K getKey() const {
			return m_key;
		}
		
		FORCE_INLINE void setVirtualSize(inttype vsize) {
			m_vsize = vsize;
		}

		FORCE_INLINE inttype getVirtualSize() const {
			return m_vsize;
		}

		FORCE_INLINE void setMaxVirtualViewpoint(uinttype maxVirtualViewpoint) {
			m_maxVirtualViewpoint = maxVirtualViewpoint;
		}

		FORCE_INLINE uinttype getMaxVirtualViewpoint() const {
			return m_maxVirtualViewpoint;
		}

		FORCE_INLINE void setMaxRealViewpoint(uinttype maxRealViewpoint) {
			m_maxRealViewpoint = maxRealViewpoint;
		}

		FORCE_INLINE uinttype getMaxRealViewpoint() const {
			return m_maxRealViewpoint;
		}

		FORCE_INLINE boolean equals(DataRequest<K>* dr) const {
			if (getRequestType() != dr->getRequestType()) {
				return false;
			}

			if (Converter<K>::equals(getKey(), dr->getKey()) == false) {
				return false;
			}

			if (getVirtualSize() != dr->getVirtualSize()) {
				return false;
			}

			if (getMaxVirtualViewpoint() != dr->getMaxVirtualViewpoint()) {
				return false;
			}

			if (getMaxRealViewpoint() != dr->getMaxRealViewpoint()) {
				return false;
			}	

			return true;
		}
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_DATAREQUEST_H_ */
