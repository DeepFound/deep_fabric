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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_STATUSREQUEST_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_STATUSREQUEST_H_

#include "cxx/lang/types.h"
#include "cxx/util/Converter.h"

using namespace cxx::util;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

template<typename K>
class StatusRequest {
	private:
		K m_key;
		uinttype m_maxViewpoint;

	public:
		StatusRequest() :
			m_key((K) Converter<K>::NULL_VALUE),
			m_maxViewpoint(0) { 
		}

		StatusRequest(K key, uinttype maxViewpoint) :
			m_key(key),
			m_maxViewpoint(maxViewpoint) {
		}

		FORCE_INLINE void setKey(K key) {
			m_key = key;
		}

		FORCE_INLINE K getKey() const {
			return m_key;
		}

		FORCE_INLINE void setMaxViewpoint(uinttype maxViewpoint) {
			m_maxViewpoint = maxViewpoint;
		}

		FORCE_INLINE uinttype getMaxViewpoint() const {
			return m_maxViewpoint;
		}

		FORCE_INLINE boolean equals(StatusRequest<K>* sr) const {
			if (Converter<K>::equals(getKey(), sr->getKey()) == false) {
				return false;
			}

			if (getMaxViewpoint() != sr->getMaxViewpoint()) {
				return false;
			}

			return true;
		}
};

class StatusResponse {
	private:
		uinttype m_maxViewpoint;

	public:
		StatusResponse() :
			m_maxViewpoint(0) { 
		}

		StatusResponse(uinttype maxViewpoint) :
			m_maxViewpoint(maxViewpoint) {
		}

		FORCE_INLINE void setMaxViewpoint(uinttype maxViewpoint) {
			m_maxViewpoint = maxViewpoint;
		}

		FORCE_INLINE uinttype getMaxViewpoint() const {
			return m_maxViewpoint;
		}

		FORCE_INLINE boolean equals(StatusResponse* sr) const {
			if (sr->getMaxViewpoint() != getMaxViewpoint()) {
				return false;
			}

			return true;
		}
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_STATUSREQUEST_H_ */
