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
#ifndef COM_DEEPIS_DATASTORE_API_DEEP_COMPARATOR_H_
#define COM_DEEPIS_DATASTORE_API_DEEP_COMPARATOR_H_

#include "cxx/util/Comparator.h"

#include "com/deepis/datastore/api/deep/DeepTypes.h"

#include "com/deepis/db/store/relative/util/BasicArray.h"

using namespace com::deepis::db::store::relative::util;

namespace cxx { namespace util {

template<typename K>
class DeepComparator : public Object {
	protected:
		const SchemaBuilder<K>* m_schemaBuilder;
		
		FORCE_INLINE DeepKeyPart* getKeyPart() const {
			return getKeyPart(0);
		}

	public:
		DeepComparator() :
			m_schemaBuilder(null) {
		}

		~DeepComparator() {
		}
		
		FORCE_INLINE void setSchemaBuilder(SchemaBuilder<K>* schemaBuilder) {
			m_schemaBuilder = schemaBuilder;
		}

		FORCE_INLINE uinttype getKeyParts() const {
			return m_schemaBuilder->getKeyParts()->size();
		}

		FORCE_INLINE DeepKeyPart* getKeyPart(uinttype keyPart) const {
			#ifdef DEEP_DEBUG 
			if (keyPart >= getKeyParts()) {
				return null;
			}
			#endif
			return m_schemaBuilder->getKeyParts()->get(keyPart);
		}
};

template<>
class Comparator<DeepNByte*> : public DeepComparator<DeepNByte*> {
	public:
		Comparator() {
		}

		~Comparator() {
		}
		
		FORCE_INLINE uinttype getKeyParts() const {
			return 1;
		}

		FORCE_INLINE int compare(const DeepNByte* o1, const DeepNByte* o2, inttype* pos = null) const {
			return getKeyPart()->compareKey((bytearray) *o1, (bytearray) *o2);
		}
};

template<>
class Comparator<DeepComposite*> : public DeepComparator<DeepComposite*> {
	private:

	public:
		Comparator() {
		}

		FORCE_INLINE int compare(const DeepComposite* o1, const DeepComposite* o2, inttype* pos = null) const {

			register uinttype keyPartMask1 = o1->getKeyPartMask();
			register uinttype keyPartMask2 = o2->getKeyPartMask();

			register boolean isNull = false;

			register bytearray d1 = *o1;
			register bytearray d2 = *o2;

			register inttype compare = 0;
			register inttype i = 0;

			register inttype keyParts = getKeyParts();

			do {
				register DeepKeyPart* keyPart = getKeyPart(i);

				compare = keyPart->compareKey(d1, d2, &isNull);
				if (compare != 0) {
					break;
				}

				keyPartMask1 >>= 1;
				keyPartMask2 >>= 1;

				if (keyPartMask1 == 0) {
					compare = (keyPartMask2 == 0) ? 0 : o1->getKeyPartWeight();
					break;
				}

				if (keyPartMask2 == 0) {
					// if any two key parts are both null, consult the primary suffix (i.e. ignored key part)
					if ((isNull == true) && (o2->getIgnorePrimary() == true)) {
						keyPartMask2 = ~0;

					} else {
						compare = (o2->getKeyPartWeight() == 0) ? 0 : ((o2->getKeyPartWeight() < 0) ? 1 : -1);
						break;
					}
				}

				d1 += keyPart->getPackLength(d1);
				d2 += keyPart->getPackLength(d2);

			} while (++i < keyParts);

			if ((pos != null) && (((inttype)i) > *pos)) {
				*pos = (inttype)i;
			}

			return compare;
		}
};

} }  // namespace

#endif //COM_DEEPIS_DATASTORE_API_DEEP_CONVERTER_H_
