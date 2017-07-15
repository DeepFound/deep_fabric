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
#include "cxx/util/Logger.h"

#include "RealtimeFabricMatrix.h"
#include "cxx/util/HashMap.cxx"
#include "cxx/util/TreeSet.cxx"
#include <json.h>
#include <json_parser.h>

using namespace cxx::lang;
using namespace cxx::util;

/**
 * RealtimeFabricSparseAdj
 * Constructor
 */
cxx::fabric::RealtimeFabricSparseAdj::RealtimeFabricSparseAdj() :
	m_iSequence(0) {
}

/**
 * ~RealtimeFabricSparseAdj
 * Destructor
 */
cxx::fabric::RealtimeFabricSparseAdj::~RealtimeFabricSparseAdj() {
	m_iSequence = 0;
}

/**
 * RealtimeFabricSparseAdj::addPeer
 * @param iServerId - The peer server id to add.
 * @param eState - the state of the peer, UP/DOWN, etc.
 * @return 0 on success.
 */
inttype cxx::fabric::
RealtimeFabricSparseAdj::addPeer(const uinttype iServerId) {
	m_cPeers.add(iServerId);
	return 0;
}

/**
 * RealtimeFabricSparseAdj::delPeer
 * @param iServerId - The Peer server id to remove/delete.
 * @return 0 on success.
 */
inttype cxx::fabric::
RealtimeFabricSparseAdj::delPeer(const uinttype iServerId) {
	m_cPeers.remove(iServerId);
	return 0;
}

/**
 * RealtimeFabricSparseAdj::compareEqual
 * Simple way to compare two sparse adjacencies for equality.
 * @param rhs [in] - The other sparse adjacency to compare with.
 * @return true if equal, else false.
 */
bool cxx::fabric::
RealtimeFabricSparseAdj::equals(const Object* obj) const {
	const cxx::fabric::RealtimeFabricSparseAdj* pcOther =
		static_cast<const cxx::fabric::RealtimeFabricSparseAdj* >(obj);
	if (0 == pcOther) {
		return false;
	}

	if (pcOther->m_cPeers.size() != this->m_cPeers.size()) {
		return false;
	}

	if (pcOther->m_iSequence != this->m_iSequence) {
		return false;
	}

	/**
	 * We use a sequence number protocol to ensure that
	 * if this number is less the 'other' then this is the old copy,
	 * if equal we can assume no change. If greater then the 'other'
	 * is older.
	 */
	return true;
}


/** RealtimeFabricAdjMatrix */


/**
 * RealtimeFabricAdjMatrix
 * Constructor
 */
cxx::fabric::RealtimeFabricAdjMatrix::RealtimeFabricAdjMatrix(uinttype iServerId) :
	m_iServerId(iServerId) {
}

/**
 * ~RealtimeFabricAdjMatrix
 * Destructor
 */
cxx::fabric::RealtimeFabricAdjMatrix::~RealtimeFabricAdjMatrix() {
	m_iServerId = 0;

	cxx::util::Set<uinttype>*      pcSet = m_cPeersAdjs.keySet();
	cxx::util::Iterator<uinttype>* pcIt  = pcSet->iterator();
	while (true == pcIt->hasNext()) {
		uinttype                              iServerId = pcIt->next();
		cxx::fabric::RealtimeFabricSparseAdj* pcSparse  = m_cPeersAdjs.get(iServerId);

		if (0 != pcSparse) {
			delete pcSparse; pcSparse = 0;
		}

		pcIt->remove();
	}

	delete pcIt;  pcIt  = 0;
	delete pcSet; pcSet = 0;
}

/**
 * RealtimeFabricAdjMatrix::addAdj
 * @param iPeerId - The peer server id to add.
 * @return 0 on success.
 */
inttype cxx::fabric::
RealtimeFabricAdjMatrix::addAdj(uinttype iPeerId) {
	cxx::fabric::RealtimeFabricSparseAdj* pcThis = m_cPeersAdjs.get(m_iServerId);
	if (0 == pcThis) {
		pcThis = new cxx::fabric::RealtimeFabricSparseAdj;
		if (0 == pcThis) {
			DEEP_LOG_ERROR(OTHER, "Unable to create a sparse adjacency\n");
			abort();
		}

		m_cPeersAdjs.put(m_iServerId, pcThis);
	}

	pcThis->addPeer(iPeerId);
	pcThis->m_iSequence++;

	return 0;
}

/**
 * RealtimeFabricAdjMatrix::delAdj
 * @param iPeerId - The peer server id to add.
 * @return 0 on success.
 */
inttype cxx::fabric::
RealtimeFabricAdjMatrix::delAdj(uinttype iPeerId) {
	cxx::fabric::RealtimeFabricSparseAdj* pcThis = m_cPeersAdjs.get(m_iServerId);
	if (0 == pcThis) {
		return -1;
	}

	pcThis->delPeer(iPeerId);
	pcThis->m_iSequence++;

	return 0;
}

/**
 * renderAsJSON
 * Can render the topology matrix as a JSON object.
 * @param cJson [out] - reference to a string to encode the JSON
 * object.
 * @return 0 on success.
 */
inttype cxx::fabric::
RealtimeFabricAdjMatrix::renderAsJSON(json_object*& pcJson) {
	json_array cMatrix;

	pcJson = new json_object;

	cxx::util::Set<uinttype>*      pcSet = m_cPeersAdjs.keySet();
	cxx::util::Iterator<uinttype>* pcIt  = pcSet->iterator();
	while (true == pcIt->hasNext()) {
		uinttype                              iServerId = pcIt->next();
		cxx::fabric::RealtimeFabricSparseAdj* pcSparse  = m_cPeersAdjs.get(iServerId);

		if (0 == pcSparse) {
			continue;
		}

		json_object cAdj;
		cAdj.add("serverId", iServerId);
		cAdj.add("sequence", pcSparse->m_iSequence);

		json_array cPeers;

		cxx::util::Iterator<uinttype>* pcIt2 = pcSparse->m_cPeers.iterator();
		while (true == pcIt2->hasNext()) {
			uinttype iPeerId = pcIt2->next();
			cPeers.add(iPeerId);
		}

		delete pcIt2; pcIt2 = 0;

		cAdj.add("adj", cPeers);

		cMatrix.add(cAdj);
	}

	delete pcIt;  pcIt  = 0;
	delete pcSet; pcSet = 0;

	pcJson->add("matrix", cMatrix);

	return 0;
}

/**
 * renderAsJSON
 * Can render the topology matrix as a JSON object.
 * @param cJson [out] - reference to a string to encode the JSON
 * object.
 * @return 0 on success.
 */
inttype cxx::fabric::
RealtimeFabricAdjMatrix::renderAsJSON(cxx::lang::String& cJson) {
	json_object* pcJson = 0;

	if (0 != renderAsJSON(pcJson)) {
		return -1;
	}

	pcJson->to_string<cxx::lang::String>(cJson);

	delete pcJson; pcJson = 0;

	return 0;
}

/**
 * processJSON
 * Process the matrix from a Peer and see how this change applies to
 * the knowledge of known peers in the cluster.
 * @param cJson [in] - reference to an encoded JSON Object.
 * @param bChangeApplied [out] - return true if we applied any change.
 * @return 0 on success.
 */
inttype cxx::fabric::
RealtimeFabricAdjMatrix::processJSON(cxx::lang::String& cJson,
				     boolean&           bChangeApplied) {
	json::root root(cJson.c_str());

	if (false == root.exists("matrix")) {
		return -1;
	}

	json::value cMatrix = root["matrix"];

	for (size_t i=0; i<cMatrix.size(); i++) {
		json::value& cAdj   = cMatrix[i];
		json::object* pcAdj = cAdj.to_object();
		if (0 == pcAdj) {
			return -2;
		}

		json::object& cAdjObj = *pcAdj;

		if (false == cAdjObj.exists("serverId")) {
			return -3;
		}

		if (false == cAdjObj.exists("sequence")) {
			return -4;
		}

		if (false == cAdjObj.exists("adj")) {
			return -5;
		}

		uinttype iServer   = 0;
		uinttype iSequence = 0;

		iServer   = static_cast<uinttype>(cAdj["serverId"].numb());
		iSequence = static_cast<uinttype>(cAdj["sequence"].numb());

		cxx::fabric::RealtimeFabricSparseAdj* pcSparse = m_cPeersAdjs.get(iServer);

		if (iServer == m_iServerId) {
			if (0 == pcSparse) {
				pcSparse = new cxx::fabric::RealtimeFabricSparseAdj;
				if (0 == pcSparse) {
					DEEP_LOG_ERROR(OTHER, "Unable to create a sparse adj\n");
					abort();
				}

				m_cPeersAdjs.put(iServer, pcSparse);
			}

			if (iSequence > pcSparse->m_iSequence) {
				pcSparse->m_iSequence = iSequence+1;
				bChangeApplied = true;
			} else if (iSequence <  pcSparse->m_iSequence) {
				bChangeApplied = true;
			}

			continue;

		}

		if ((0 == pcSparse) || (iSequence > pcSparse->m_iSequence)) {
			cxx::fabric::RealtimeFabricSparseAdj* pcNewer =
				new cxx::fabric::RealtimeFabricSparseAdj;
			if (0 == pcNewer) {
				DEEP_LOG_ERROR(OTHER, "Unable to allocate a sparse adj.\n");
				abort();
			}

			pcNewer->m_iSequence = iSequence;

			const json::value& cPeers = cAdjObj["adj"];

			for (size_t j=0; j<cPeers.size(); j++) {
				uinttype iPeerId = static_cast<uinttype>(cPeers[j].numb());
				pcNewer->addPeer(iPeerId);
			}

			pcSparse = m_cPeersAdjs.put(iServer, pcNewer);
			if (0 != pcSparse) {
				delete pcSparse; pcSparse = 0;
			}

		} else if (iSequence < pcSparse->m_iSequence) {
			/** even if this was an old update, tag it as a change. */
			bChangeApplied = true;
		}
	}

	return 0;
}
