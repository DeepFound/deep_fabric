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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_VIRTUALINFO_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_VIRTUALINFO_H_

#include "cxx/lang/types.h"

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

class VirtualInfo {
	private:
		inttype m_vsize;
		uinttype m_minViewpoint;
		uinttype m_maxViewpoint;

		longtype m_version;

	public:
		VirtualInfo() :
			m_vsize(0),
			m_minViewpoint(0),
			m_maxViewpoint(0),
			m_version(0) {

		}

		VirtualInfo(inttype vsize, uinttype minViewpoint, uinttype maxViewpoint, longtype version = 0) :
			m_vsize(vsize),
			m_minViewpoint(minViewpoint),
			m_maxViewpoint(maxViewpoint),
			m_version(version) {
		}

		FORCE_INLINE void setVirtualSize(inttype vsize) {
			m_vsize = vsize;
		}
	
		FORCE_INLINE inttype getVirtualSize() const {
			return m_vsize;
		}

		FORCE_INLINE void setMinViewpoint(uinttype minViewpoint) {
			m_minViewpoint = minViewpoint;
		}

		FORCE_INLINE uinttype getMinViewpoint() const {
			return m_minViewpoint;
		}

		FORCE_INLINE void setMaxViewpoint(uinttype maxViewpoint) {
			m_maxViewpoint = maxViewpoint;
		}

		FORCE_INLINE uinttype getMaxViewpoint() const {
			return m_maxViewpoint;
		}

		FORCE_INLINE void setVersion(longtype version) {
			m_version = version;
		}

		FORCE_INLINE longtype getVersion() const {
			return m_version;
		}

		FORCE_INLINE boolean equals(VirtualInfo* vi) const {
			if (getVirtualSize() != vi->getVirtualSize()) {
				return false;
			}
			
			if (getMinViewpoint() != vi->getMinViewpoint()) {
				return false;
			}	

			if (getMaxViewpoint() != vi->getMaxViewpoint()) {
				return false;
			}

			if (getVersion() != vi->getVersion()) {
				return false;
			}

			return true;
		}
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_VIRTUALINFO_H_ */
