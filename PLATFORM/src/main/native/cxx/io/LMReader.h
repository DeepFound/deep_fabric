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
#ifndef CXX_LANG_IO_LMREADER_H_
#define CXX_LANG_IO_LMREADER_H_

#include "cxx/io/EncodeProtocol.h"
#include "cxx/lang/Object.h"
#include "cxx/io/LinkManager.h"

namespace cxx { namespace io {

class lmReader : public cxx::io::encodeProtocol::reader {
public:
	lmReader(ubytetype* piBuffer,
		 uinttype   iBufferLen,
		 voidptr    lmContext) : cxx::io::encodeProtocol::reader(piBuffer,
									 iBufferLen),
					 m_lmContext(lmContext) {
	}

	virtual ~lmReader() {
		cxx::io::closeLmContext(m_lmContext);
	}

private:
	voidptr    m_lmContext;

}; // lmReader

} // io
} // cxx


#endif // CXX_LANG_IO_LMREADER_H_
