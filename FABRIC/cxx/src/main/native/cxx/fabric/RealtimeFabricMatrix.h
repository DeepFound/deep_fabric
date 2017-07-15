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
#ifndef CXX_LANG_RTFABRICMATRIX_H_
#define CXX_LANG_RTFABRICMATRIX_H_

#include <stdio.h>
#include <errno.h>

#include "cxx/io/EOFException.h"
#include "cxx/lang/types.h"
#include "cxx/util/HashMap.h"
#include "cxx/util/TreeSet.h"
#include <json.h>
#include <json_parser.h>

namespace cxx { namespace fabric {

class RealtimeFabricAdjMatrix;

class RealtimeFabricSparseAdj : public cxx::lang::Object {
public:
	RealtimeFabricSparseAdj();
	virtual ~RealtimeFabricSparseAdj();

	inttype addPeer(const uinttype iServerId);
	inttype delPeer(const uinttype iServerId);

	boolean equals(const Object* obj) const;

private:
	friend class RealtimeFabricAdjMatrix;

	uinttype                     m_iSequence;
	cxx::util::TreeSet<uinttype> m_cPeers;

}; /** RealtimeFabricSparseAdj */

class RealtimeFabricAdjMatrix {
public:
	RealtimeFabricAdjMatrix(uinttype m_iServerId);
	virtual ~RealtimeFabricAdjMatrix();

	inttype addAdj(uinttype iPeerId);
	inttype delAdj(uinttype iPeerId);

	inttype renderAsJSON(cxx::lang::String& cJson);
	inttype renderAsJSON(json_object*& pcJson);

	int processJSON(cxx::lang::String& cJson,
			boolean&           bChangeApplied);

private:
	uinttype m_iServerId;
	cxx::util::HashMap<uinttype, RealtimeFabricSparseAdj* > m_cPeersAdjs;

};


} /** fabric */

} /** cxx */

#endif /** CXX_LANG_RTFABRICMATRIX_H_ */
