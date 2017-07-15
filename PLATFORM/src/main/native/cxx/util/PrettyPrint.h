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
#ifndef CXX_UTIL_PRETTYPRINT
#define CXX_UTIL_PRETTYPRINT

#include <sstream>
#include "cxx/lang/String.h"
#include <stdlib.h>
#include "cxx/lang/types.h"

namespace cxx { namespace util { namespace prettyprint {

static const int MEMORY_EPOC = (1024);

/**
 * sizeUnit<T>
 * PrettyPrint the provided value. Suffix is defaulted to B (Bytes).
 * @param num - Numeric value type that can be converted to the
 * largest range Gi, Mi, Ki etc
 * @param retVal - reference to the string to return the pretty value.
 * @param suffix - 'B' or 'b'.
 */
template<typename T>
inline void _sizeUnit(T num, cxx::lang::String& retVal, char suffix='B') {
	static const int   unitCount = 8;
	static const char* units[unitCount] = {"",
					       "Ki",
					       "Mi",
					       "Gi",
					       "Ti",
					       "Pi",
					       "Ei",
					       "Zi"};
	std::stringstream ss;

	doubletype fNum = static_cast<doubletype>(num);

	for (int i=0; i<unitCount; i++) {
		if (fNum < MEMORY_EPOC) {
			ss << fNum << units[i] << suffix;
			retVal = ss.str();
			return;
		}

		fNum /= MEMORY_EPOC;
	}

	ss << fNum << "Yi" << suffix;
	retVal = ss.str();
}

/**
 * Bytes<T>
 * @params num - Numeric Type to convert to KiB, MiB, GiB, etc.
 * @params retVal - reference to return the pretty string into.
 */
template<typename T>
inline void Bytes(T num, cxx::lang::String& cRetVal) {
	return _sizeUnit<T>(num, cRetVal, 'B');
}

/**
 * Bits<T>
 * @params num - Numeric Type to convert to Kib, Mib, Gib, etc.
 * @params retVal - reference to return the pretty string into.
 */
template<typename T>
inline void Bits(T num, cxx::lang::String& cRetVal) {
	return _sizeUnit<T>(num, cRetVal, 'b');
}

} /** prettyprint */

} /** util */

} /** cxx */

#endif /** CXX_UTIL_PRETTYPRINT */
