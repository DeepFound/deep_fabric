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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_PEERINFO_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_PEERINFO_H_

#include "cxx/lang/types.h"
#include "cxx/lang/String.h"
#include "com/deepis/db/store/relative/distributed/MapBehavior.h"
#include "com/deepis/db/store/relative/distributed/Facilitator.h"

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

class PeerInfo {
	private:
		uinttype m_serverId;
		String m_address;
		MapBehavior m_mapBehavior;
		void* m_facilitator;
		longtype m_virtualKeySpaceVersion;

	public:
		PeerInfo() :
			m_serverId(0),
			m_address(""),
			m_mapBehavior(DISTRIBUTED_STANDALONE),
			m_facilitator(null),
			m_virtualKeySpaceVersion(0) { 
		}

		PeerInfo(uinttype serverId, String address, MapBehavior mapBehavior, void* facilitator = null) :
			m_serverId(serverId),
			m_address(address),
			m_mapBehavior(mapBehavior),
			m_facilitator(facilitator),
			m_virtualKeySpaceVersion(0) {
		}

		FORCE_INLINE void setServerId(uinttype serverId) {
			m_serverId = serverId;
		}

		FORCE_INLINE uinttype getServerId() const {
			return m_serverId;
		}

		FORCE_INLINE void setAddress(const char* address) {
			m_address = String(address);
		}

		FORCE_INLINE String* getAddressString() {
			return &m_address;
		}

		FORCE_INLINE const char* getAddress() const {
			return m_address.c_str();
		}

		FORCE_INLINE void setMapBehavior(MapBehavior mapBehavior) {
			m_mapBehavior = mapBehavior;
		}

		FORCE_INLINE MapBehavior getMapBehavior() const {
			return m_mapBehavior;
		}

		FORCE_INLINE void setFacilitator(void* facilitator) {
			m_facilitator = facilitator;
		}

		FORCE_INLINE void* getFacilitator() const {
			return m_facilitator;
		}

		FORCE_INLINE void setVirtualKeySpaceVersion(longtype version) {
			m_virtualKeySpaceVersion = version;
		}

		FORCE_INLINE longtype getVirtualKeySpaceVersion() const {
			return m_virtualKeySpaceVersion;
		}

		FORCE_INLINE boolean equals(PeerInfo* pi) const {
			if (getServerId() != pi->getServerId()) {
				return false;
			}

			if (m_address != (String(pi->getAddress()))) {
				return false;
			}

			if (getMapBehavior() != pi->getMapBehavior()) {
				return false;
			}

			if (getFacilitator() != pi->getFacilitator()) {
				return false;
			}

			return true;
		}
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_PEERINFO_H_ */
