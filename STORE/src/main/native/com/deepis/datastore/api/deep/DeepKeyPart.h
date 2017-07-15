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
#ifndef COM_DEEPIS_DATASTORE_API_DEEP_KEYPART_H_
#define COM_DEEPIS_DATASTORE_API_DEEP_KEYPART_H_

#include "cxx/lang/Object.h"
#include "cxx/util/Collections.h"

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace core {
class DeepField;
} } } } } }

using namespace cxx::lang;
using namespace cxx::util;
using namespace com::deepis::db::store::relative::core;

class DeepKeyPart : public Object {
	protected:
		uinttype   m_fieldIndex;
		uchartype  m_type;
		ushorttype m_length;
		uinttype   m_valueOffset;
		uinttype   m_nullOffset;
		uchartype  m_nullBit;
		boolean    m_isIgnored;
		boolean    m_isReserved;
		shorttype  m_variablePosition;
		shorttype  m_primaryPosition;
		ushorttype m_lengthBytes;
		ushorttype m_nullBytes;
		ushorttype m_unpackLength;
		const DeepField* m_field;
		
		friend class Comparator<DeepKeyPart*>;

	public:
		DeepKeyPart() {
		}

		DeepKeyPart(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset = 0, uchartype nullBit = 0, boolean isIgnored = false, boolean isReserved = false, shorttype variablePosition = -1, shorttype primaryPosition = -1) :
			m_fieldIndex(fieldIndex),
			m_type(type),
			m_length(length),
			m_valueOffset(valueOffset),
			m_nullOffset(nullOffset),
			m_nullBit(nullBit),
			m_isIgnored(isIgnored),
			m_isReserved(isReserved),
			m_variablePosition(variablePosition),
			m_primaryPosition(primaryPosition),
			m_field(null) {

			m_lengthBytes   = (m_variablePosition != -1) ? 2 : 0;
			m_nullBytes     = (m_nullBit != 0) ? 1 : 0;
			m_unpackLength  = m_lengthBytes + m_nullBytes + m_length;
		}
		
		FORCE_INLINE uinttype getFieldIndex() const {
			return m_fieldIndex;
		}
		
		FORCE_INLINE const DeepField* getField() const {
			return m_field;
		}
		
		FORCE_INLINE void setField(const DeepField* field) {
			m_field = field;
		}

		FORCE_INLINE ushorttype getLength() const {
			return m_length;
		}

		virtual ushorttype getPackLength(const bytearray key) const = 0;

		FORCE_INLINE ushorttype getUnpackLength() const {
			return m_unpackLength;
		}

		FORCE_INLINE uchartype getType() const {
			return m_type;
		}

		FORCE_INLINE uinttype getValueOffset() const {
			return m_valueOffset;
		}

		FORCE_INLINE boolean isIgnored() const {
			return m_isIgnored;
		}

		FORCE_INLINE boolean isReserved() const {
			return m_isReserved;
		}

		FORCE_INLINE shorttype getVariablePosition() const {
			return m_variablePosition;
		}

		FORCE_INLINE shorttype getPrimaryPosition() const {
			return m_primaryPosition;
		}

		FORCE_INLINE uchartype getNullBit () const {
			return m_nullBit;
		}

		FORCE_INLINE uinttype getNullOffset () const {
			return m_nullOffset;
		}

		FORCE_INLINE boolean maybeNull() const {
			return (m_nullBit != 0);
		}

		FORCE_INLINE uchartype isNull(const bytearray value) const {
			return value[m_nullOffset] & m_nullBit;
		}

		FORCE_INLINE void setNull(const bytearray value) const {
			value[m_nullOffset] = (value[m_nullOffset] | m_nullBit);
		}

		virtual inttype compareKey(const bytearray key1, const bytearray key2, boolean* isNull = null) = 0;

		virtual ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) = 0;

		virtual ulongtype maxKey() const = 0;

		virtual ulongtype toULong(const bytearray key) const = 0;

		virtual inttype toString(const bytearray key, bytearray buffer) const = 0;
};

namespace cxx { namespace util {

template<>
class Comparator<DeepKeyPart*> {
	public:
		Comparator() {
		}
		
		#ifdef COM_DEEPIS_DB_CARDINALITY
		FORCE_INLINE int compare(const DeepKeyPart* k1, const DeepKeyPart* k2, inttype* pos = null) const {
		#else
		FORCE_INLINE int compare(const DeepKeyPart* k1, const DeepKeyPart* k2) const {
		#endif
			return
				(k1->m_type             - k2->m_type            ) ||
				(k1->m_length           - k2->m_length          ) ||
				(k1->m_valueOffset      - k2->m_valueOffset     ) ||
				(k1->m_nullOffset       - k2->m_nullOffset      ) ||
				(k1->m_nullBit          - k2->m_nullBit         ) ||
				(k1->m_isIgnored        - k2->m_isIgnored       ) ||
				(k1->m_isReserved       - k2->m_isReserved      ) ||
				(k1->m_variablePosition - k2->m_variablePosition) ||
				(k1->m_lengthBytes      - k2->m_lengthBytes     ) ||
				(k1->m_nullBytes        - k2->m_nullBytes       ) ||
				(k1->m_unpackLength     - k2->m_unpackLength    );
		}
};
 
} } // namespace

#endif //COM_DEEPIS_DATASTORE_API_DEEP_KEYPART_H_

