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
#ifndef COM_DEEPIS_DATASTORE_API_DEEP_TABLE_H_
#define COM_DEEPIS_DATASTORE_API_DEEP_TABLE_H_

#include "cxx/lang/types.h"

#include "cxx/lang/Object.h"
#include "cxx/lang/NullPointerException.h"

#include "cxx/util/Logger.h"
#include "cxx/util/ArrayList.h"
#include "cxx/util/Comparator.h"

#include "com/deepis/datastore/api/deep/DeepTypes.h"
#include "com/deepis/datastore/api/deep/DeepKeyPart.h"

#include "com/deepis/db/store/relative/core/RealTime.h"
#include "com/deepis/db/store/relative/core/RealTimeSchema.h"
#include "com/deepis/db/store/relative/core/RealTimeMap.h"
#include "com/deepis/db/store/relative/util/BasicArray.h"

using namespace cxx::lang;
using namespace cxx::util;

using namespace com::deepis::db::store::relative::util;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace core {

class DeepField : public Object {
	private:
		uchartype  m_type;
		uchartype  m_realType;
		uinttype   m_packLength;
		uinttype   m_rowPackLength;
		uinttype   m_keyLength;
		uinttype   m_lengthBytes;
		ushorttype m_index;
		uchartype  m_nullBit;
		uinttype   m_nullOffset;
		uinttype   m_valueOffset;
		inttype    m_characterSet;
		bool       m_gcVirtual;
		const char* m_fieldName;
	public:
		DeepField(uchartype type, uchartype realType, uinttype packLength, uinttype rowPackLength, uinttype keyLength, 
			   uinttype lengthBytes, ushorttype index, uchartype nullBit, uinttype nullOffset, uinttype valueOffset, inttype characterSet, bool gcVirtual, const char* fieldName) :
				m_type(type),
				m_realType(realType),
				m_packLength(packLength),
				m_rowPackLength(rowPackLength),
				m_keyLength(keyLength),
				m_lengthBytes(lengthBytes),
				m_index(index),
				m_nullBit(nullBit),
				m_nullOffset(nullOffset),
				m_valueOffset(valueOffset),
				m_characterSet(characterSet),
				m_gcVirtual(gcVirtual), 
				m_fieldName(fieldName) {
		}
		
		virtual ~DeepField() {
		}

		FORCE_INLINE DeepField* clone() const {
			return new DeepField(m_type, m_realType, m_packLength, m_rowPackLength, m_keyLength, 
					      m_lengthBytes, m_index, m_nullBit, m_nullOffset, m_valueOffset, m_characterSet, m_gcVirtual, m_fieldName);
		}
		
		FORCE_INLINE uchartype getType() const {
			return m_type;
		}
		
		FORCE_INLINE uchartype getRealType() const {
			return m_realType;
		}
		
		FORCE_INLINE uinttype getPackLength() const {
			return m_packLength;
		}
		
		FORCE_INLINE uinttype getRowPackLength() const {
			return m_rowPackLength;
		}
		
		FORCE_INLINE uinttype getKeyLength() const {
			return m_keyLength;
		}
		
		FORCE_INLINE uinttype getLengthBytes() const {
			return m_lengthBytes;
		}
		
		FORCE_INLINE ushorttype getIndex() const {
			return m_index;
		}
		
		FORCE_INLINE uchartype getNullBit() const {
			return m_nullBit;
		}
		
		FORCE_INLINE uinttype getNullOffset() const {
			return m_nullOffset;
		}
		
		FORCE_INLINE uinttype getValueOffset() const {
			return m_valueOffset;
		}
		
		FORCE_INLINE uinttype getCharacterSet() const {
			return m_characterSet;
		}
		
		FORCE_INLINE uchartype isNull(const bytearray value) const {
			return value[m_nullOffset] & m_nullBit;
		}

		FORCE_INLINE bool isGcVirtual() const {
			return m_gcVirtual;
		}

		FORCE_INLINE const char* getFieldName() const {
			return m_fieldName;
		}		
		// Get the length of a field in an unpacked row
		template<uinttype T>
		FORCE_INLINE uinttype getLength(const bytearray value) const;
};

class Store : public Object {
	private:
		static const int INITIAL_CAPACITY = 16;
		
		// Map-related fields
		String m_keyName;
		inttype m_lrtIndex;
		uinttype m_lrtPosition;
		inttype m_protocol;
		inttype m_keySize;
		inttype m_valueSize;
		boolean m_hasPrimary;
		boolean m_isTemporary;
		uinttype m_indexOrientation;
		boolean m_keyCompression;
		boolean m_valueCompression;
		File m_dataDir;
		File m_indexDir;
		
		// Store-related fields
		uinttype m_nullBytes;
		
		// Key-related fields
		ushorttype m_unpackLength;
		inttype m_hiddenKeyPart;
		inttype m_reservedKeyPart;
		inttype m_ignoredKeyPart;
		boolean m_requiresRekey;
		BasicArray<DeepKeyPart*> m_keyParts;
		
		// Field-related fields
		BasicArray<DeepField*> m_fields;
		boolean m_hasVariableLengthFields;
		boolean m_hasVirtualFields;
		uinttype m_blobStartOffset;
		uinttype m_blobPtrSize;
		ulongtype m_unpackedRowLength;
		ulongtype m_autoIncrementValue;
		inttype m_characterSet;

		friend class Comparator<Store*>;
	public:
		Store() :
				m_keyName(),
				m_lrtIndex(0),
				m_lrtPosition(0),
				m_protocol(-1),
				m_keySize(-1),
				m_valueSize(-1),
				m_hasPrimary(false),
				m_isTemporary(false),
				m_indexOrientation(RealTime::INDEX_ORIENTATION_ROW),
				m_keyCompression(false),
				m_valueCompression(false),
				m_dataDir(""),
				m_indexDir(""),
				m_nullBytes(0),
				m_unpackLength(0),
				m_hiddenKeyPart(-1),
				m_reservedKeyPart(-1),
				m_ignoredKeyPart(-1),
				m_requiresRekey(false),
				m_keyParts(INITIAL_CAPACITY, true),
				m_fields(INITIAL_CAPACITY, true),
				m_hasVariableLengthFields(false),
				m_blobStartOffset(0),
				m_blobPtrSize(0),
				m_unpackedRowLength(0),
				m_autoIncrementValue(1),
				m_characterSet(-1) {
		}

		virtual ~Store() {
		}
		
		FORCE_INLINE void addKeyPart(DeepKeyPart* keyPart) {
			if (keyPart == null) {
				throw NullPointerException();
			}
			
			if (keyPart->getType() == CT_DATASTORE_HIDDEN) {
				m_hiddenKeyPart = m_keyParts.size();
			}
			else {
				keyPart->setField(getField(keyPart->getFieldIndex()));
			}
			
			if (keyPart->isReserved() == true) {
				m_reservedKeyPart = m_keyParts.size();
			}

			if ((m_ignoredKeyPart == -1) && (keyPart->isIgnored() == true)) {
				m_ignoredKeyPart = m_keyParts.size();
			}

			if (requiresRekey(keyPart) == true) {
				m_requiresRekey = true;
			}

			m_unpackLength += keyPart->getUnpackLength();
			
			m_keyParts.add(keyPart, true);
		}
		
		FORCE_INLINE const DeepField* getField(uinttype index) const {
			const DeepField* field = m_fields.get(index);
			
			#ifdef DEEP_DEBUG
			if (field->getIndex() != index) {
				throw RuntimeException();
			}
			#endif
			
			return field;
		}
		
		FORCE_INLINE void addField(DeepField* field) {
			if (field == null) {
				throw NullPointerException();
			}
			
			m_fields.add(field, true);
		}

		FORCE_INLINE inttype getFieldCount() const {
			return m_fields.size();
		}
		
		FORCE_INLINE const BasicArray<DeepKeyPart*>* getKeyParts() const {
			return &m_keyParts;
		}
		
		FORCE_INLINE const BasicArray<DeepField*>* getFields() const {
			return &m_fields;
		}

		FORCE_INLINE const char* getKeyName() const {
			return m_keyName.data();
		}

		FORCE_INLINE inttype getLrtIndex() const {
			return m_lrtIndex;
		}
	
		FORCE_INLINE uinttype getLrtPosition() const {
			return m_lrtPosition;
		}
		
		FORCE_INLINE inttype getKeySize() const {
			return m_keySize;
		}
		
		FORCE_INLINE inttype getValueSize() const {
			return m_valueSize;
		}
		
		FORCE_INLINE boolean getHasPrimary() const {
			return m_hasPrimary;
		}
		
		FORCE_INLINE inttype getKeyProtocol() const {
			return m_protocol;
		}
		
		FORCE_INLINE boolean hasHiddenKey() const {
			return (m_hiddenKeyPart != -1);
		}
		
		FORCE_INLINE boolean hasReservedKey() const {
			return (m_reservedKeyPart != -1);
		}
		
		FORCE_INLINE ushorttype getUnpackLength() const {
			return m_unpackLength;
		}
		
		FORCE_INLINE bytearray getHiddenKey(DeepComposite* key) const {
			if (hasHiddenKey() == true) {
				register uinttype offset = 0;
				for (inttype i=0; i<m_hiddenKeyPart; i++) {
					offset += getKeyParts()->get(i)->getPackLength(((bytearray) *key) + offset);
				}

				return ((bytearray) *key) + offset;
			}

			throw UnsupportedOperationException("Primary key is not contained in this key");
		}

		FORCE_INLINE ulongtype getReservedKey(const DeepComposite* key) const {
			if (hasReservedKey() == true) {
				register uinttype offset = 0;
				for (inttype i=0; i<m_reservedKeyPart; i++) {
					offset += getKeyParts()->get(i)->getPackLength(((bytearray) *key) + offset);
				}

				return getKeyParts()->get(m_reservedKeyPart)->toULong(((bytearray) *key) + offset);
			}

			throw UnsupportedOperationException("Reserved key is not contained in this key");
		}

		FORCE_INLINE inttype getIgnoredKey() const {
			return m_ignoredKeyPart;
		}

		FORCE_INLINE boolean hasIgnoredKey() const {
			return (m_ignoredKeyPart != -1);
		}

		FORCE_INLINE boolean getRequiresRekey() const {
			return m_requiresRekey;
		}

		FORCE_INLINE ulongtype maxReservedKey() const {
			return getKeyParts()->get(m_reservedKeyPart)->maxKey();
		}
		
		FORCE_INLINE boolean hasVariableLengthFields() const {
			return m_hasVariableLengthFields;
		}

		FORCE_INLINE boolean hasVirtualFields() const {
			return m_hasVirtualFields;
		}
		
		FORCE_INLINE void setHasVariableLengthFields(boolean hasVariableLengthFields) {
			m_hasVariableLengthFields = hasVariableLengthFields;
		}

		FORCE_INLINE void setHasVirtualFields(boolean hasVirtualFields) {
			m_hasVirtualFields = hasVirtualFields;
		}
		
		FORCE_INLINE uinttype getNullBytes() const {
			return m_nullBytes;
		}
		
		FORCE_INLINE void setNullBytes(uinttype nullBytes) {
			m_nullBytes = nullBytes;
		}
		
		FORCE_INLINE uinttype getBlobStartOffset() const {
			return m_blobStartOffset;
		}
		
		FORCE_INLINE void setBlobStartOffset(uinttype blobStartOffset) {
			m_blobStartOffset = blobStartOffset;
		}
		
		FORCE_INLINE uinttype getBlobPtrSize() const {
			return m_blobPtrSize;
		}
		
		FORCE_INLINE void setBlobPtrSize(uinttype blobPtrSize) {
			m_blobPtrSize = blobPtrSize;
		}
		
		FORCE_INLINE ulongtype getUnpackedRowLength() const {
			return m_unpackedRowLength;
		}
		
		FORCE_INLINE void setUnpackedRowLength(ulongtype unpackedRowLength) {
			m_unpackedRowLength = unpackedRowLength;
		}
		
		FORCE_INLINE ulongtype getAutoIncrementValue() const {
			return m_autoIncrementValue;
		}
		
		FORCE_INLINE void setAutoIncrementValue(ulongtype autoIncrementValue) {
			m_autoIncrementValue = autoIncrementValue;
		}

		FORCE_INLINE void setDirectoryPaths(const File& dataDir, const File& indexDir) {
			m_dataDir = dataDir;
			m_indexDir = indexDir;
		}

		FORCE_INLINE const File& getDataDirectory() const {
			return m_dataDir;
		}

		FORCE_INLINE const File& getIndexDirectory() const {
			return m_indexDir;
		}

		FORCE_INLINE inttype getCharacterSet() const {
			return m_characterSet;
		}
		
		FORCE_INLINE void setCharacterSet(inttype characterSet) {
			m_characterSet = characterSet;
		}

		FORCE_INLINE boolean isTemporary() const {
			return m_isTemporary;
		}
		
		FORCE_INLINE void setIsTemporary(boolean isTemporary) {
			m_isTemporary = isTemporary;
		}
		
		FORCE_INLINE RealTime::IndexOrientation getIndexOrientation() const {
			return (RealTime::IndexOrientation)m_indexOrientation;
		}
		
		FORCE_INLINE void setIndexOrientation(RealTime::IndexOrientation indexOrientation) {
			m_indexOrientation = indexOrientation;
		}

		FORCE_INLINE boolean getKeyCompression() const {
			return m_keyCompression;
		}

		FORCE_INLINE void setKeyCompression(boolean keyCompression) {
			m_keyCompression = keyCompression;
		}

		FORCE_INLINE boolean getValueCompression() const {
			return m_valueCompression;
		}

		FORCE_INLINE void setValueCompression(boolean valueCompression) {
			m_valueCompression = valueCompression;
		}

		FORCE_INLINE void setKeyName(const char* keyName) {
			m_keyName = String(keyName);
		}

		FORCE_INLINE void setLrtIndex(const inttype index) {
			m_lrtIndex = index;
		}

		FORCE_INLINE void setLrtPosition(const uinttype position) {
			m_lrtPosition = position;
		}
		
		FORCE_INLINE void setMapParams(inttype protocol, inttype keySize, inttype valueSize, boolean hasPrimary) {
			m_protocol = protocol;
			m_keySize = keySize;
			m_valueSize = valueSize;
			m_hasPrimary = hasPrimary;
		}
		
		template<typename K>
		FORCE_INLINE void setMapParams(RealTimeMap<K>* map) {
			setMapParams(map->getShare()->getKeyProtocol(), map->getShare()->getKeySize(), map->getShare()->getValueSize(), map->getShare()->getHasPrimary());
		}
		
		FORCE_INLINE inttype compare(const Store* store) const;

		FORCE_INLINE void cloneTo(Store* store) const {

			// XXX: only comparator relevant fields copied over
			for (int i = 0; i < this->getFieldCount(); i++) {
				store->addField(this->getField(i)->clone());
			}
			store->m_protocol = m_protocol;
			store->m_keySize = m_keySize;
			store->m_valueSize = m_valueSize;
			store->m_hasPrimary = m_hasPrimary;
			store->m_nullBytes = m_nullBytes;
			store->m_hiddenKeyPart = m_hiddenKeyPart;
			store->m_reservedKeyPart = m_reservedKeyPart;
			store->m_hasVariableLengthFields = m_hasVariableLengthFields;
			store->m_blobStartOffset = m_blobStartOffset;
			store->m_blobPtrSize = m_blobPtrSize;
			store->m_unpackedRowLength = m_unpackedRowLength;
			store->m_isTemporary = m_isTemporary;
			store->m_characterSet = m_characterSet;
		}

	private:
		FORCE_INLINE boolean requiresRekey(DeepKeyPart* keyPart) {
			switch(keyPart->getType()) {
				case CT_DATASTORE_VARTEXT1:
				case CT_DATASTORE_VARTEXT2:
				case CT_DATASTORE_FIXED_VARTEXT1:
				case CT_DATASTORE_TEXT:
					return true;

				default:
					return false;
			}
		}
};

template<>
class SchemaBuilder<DeepLongInt> : public Store {
};

template<>
class SchemaBuilder<DeepULongInt> : public Store {
};

template<>
class SchemaBuilder<DeepLongLong> : public Store {
};

template<>
class SchemaBuilder<DeepULongLong> : public Store {
};

#ifndef DEEP_REDUCE_TEMPLATES
template<>
class SchemaBuilder<DeepShortInt> : public Store {
};

template<>
class SchemaBuilder<DeepUShortInt> : public Store {
};

template<>
class SchemaBuilder<DeepTinyInt> : public Store {
};

template<>
class SchemaBuilder<DeepUTinyInt> : public Store {
};

template<>
class SchemaBuilder<DeepDouble> : public Store {
};

template<>
class SchemaBuilder<DeepFloat> : public Store {
};
#endif

template<>
class SchemaBuilder<DeepNByte*> : public Store {
};

template<>
class SchemaBuilder<DeepComposite*> : public Store {
};

template<uinttype T>
FORCE_INLINE uinttype DeepField::getLength(const bytearray value) const {
	DEEP_LOG(ERROR, OTHER, "Unsupported operation: DeepField::getLength<%u>()\n", T);
	throw UnsupportedOperationException();
}

template<>
FORCE_INLINE uinttype DeepField::getLength<CT_DATASTORE_BLOB>(const bytearray value) const {
	const bytearray pos = value + getValueOffset();
	switch (getPackLength()) {
		case 1:
			return ((uinttype) pos[0]) & 0x000000FF;
		case 2: {
			ushorttype ret;
			dp_shortget(ret, pos);
			return (uinttype) ret;
		}
		case 3:
			return (uinttype) dp_uint3korr(pos);
		case 4: {
			uinttype ret;
			dp_longget(ret, pos);
			return (uinttype) ret;
		}
	}
	
	DEEP_LOG(ERROR, OTHER, "Invalid pack length: %u\n", getPackLength());
	throw RuntimeException("Invalid pack length");
}

} } } } } } // namespace

namespace cxx { namespace util {

template<>
class Comparator<DeepField*> {
	public:
		Comparator() {
		}
		
		#ifdef COM_DEEPIS_DB_CARDINALITY
		FORCE_INLINE int compare(const DeepField* f1, const DeepField* f2, inttype* pos = null) const {
		#else
		FORCE_INLINE int compare(const DeepField* f1, const DeepField* f2) const {
		#endif
			return
				(f1->getType()             - f2->getType()           ) ||
				(f1->getRealType()         - f2->getRealType()       ) ||
				(f1->getPackLength()       - f2->getPackLength()     ) ||
				(f1->getRowPackLength()    - f2->getRowPackLength()  ) ||
				(f1->getKeyLength()        - f2->getKeyLength()      ) ||
				(f1->getLengthBytes()      - f2->getLengthBytes()    ) ||
				(f1->getIndex()            - f2->getIndex()          ) ||
				(f1->getNullBit()          - f2->getNullBit()        ) ||
				(f1->getNullOffset()       - f2->getNullOffset()     ) ||
				(f1->getValueOffset()      - f2->getValueOffset()    ) ||
				(f1->getCharacterSet()     - f2->getCharacterSet()   );
		}
};

template<>
class Comparator<Store*> {
	public:
		Comparator() {
		}
		
		#ifdef COM_DEEPIS_DB_CARDINALITY
		FORCE_INLINE int compare(const Store* s1, const Store* s2, inttype* pos = null) const {
		#else
		FORCE_INLINE int compare(const Store* s1, const Store* s2) const {
		#endif
			inttype cmp = 
				#if 0
				s1->m_filePath.compareTo(           &s2->getFilePath()                ) ||
				#endif
				(s1->getKeyProtocol()              - s2->getKeyProtocol()             ) ||
				(s1->getKeySize()                  - s2->getKeySize()                 ) ||
				(s1->getValueSize()                - s2->getValueSize()               ) ||
				(s1->getHasPrimary()               - s2->getHasPrimary()              ) ||
				(s1->getNullBytes()                - s2->getNullBytes()               ) ||
				(s1->getUnpackLength()             - s2->getUnpackLength()            ) ||
				(s1->hasHiddenKey()                - s2->hasHiddenKey()               ) ||
				(s1->hasReservedKey()              - s2->hasReservedKey()             ) ||
				(s1->hasVariableLengthFields()     - s2->hasVariableLengthFields()    ) ||
				(s1->getBlobStartOffset()          - s2->getBlobStartOffset()         ) ||
				(s1->getBlobPtrSize()              - s2->getBlobPtrSize()             ) ||
				(s1->getUnpackedRowLength()        - s2->getUnpackedRowLength()       ) ||
				(s1->isTemporary()                 - s2->isTemporary()                ) ||
				(s1->getCharacterSet()             - s2->getCharacterSet()            ) /* ||
				(s1->getAutoIncrementValue()       - s2->getAutoIncrementValue()      ) */; // TODO: handle case when auto increment is unknown

			if (cmp != 0) {
				return cmp;
			}
			
			struct Iter {
				Iterator<DeepKeyPart*>* k1;
				Iterator<DeepKeyPart*>* k2;
				
				Iterator<DeepField*>* f1;
				Iterator<DeepField*>* f2;
				
				Iter(const Store* s1, const Store* s2) {
					k1 = Collections::unmodifiableIteratorFrom< DeepKeyPart*,BasicArray<DeepKeyPart*>::BasicArrayIterator,BasicArray<DeepKeyPart*> >(s1->getKeyParts());
					k2 = Collections::unmodifiableIteratorFrom< DeepKeyPart*,BasicArray<DeepKeyPart*>::BasicArrayIterator,BasicArray<DeepKeyPart*> >(s2->getKeyParts());
					
					f1 = Collections::unmodifiableIteratorFrom< DeepField*,BasicArray<DeepField*>::BasicArrayIterator,BasicArray<DeepField*> >(s1->getFields());
					f2 = Collections::unmodifiableIteratorFrom< DeepField*,BasicArray<DeepField*>::BasicArrayIterator,BasicArray<DeepField*> >(s2->getFields());
				}
				
				~Iter() {
					delete k1;
					delete k2;
					delete f1;
					delete f2;
				}
			} iter(s1, s2);
			
			Comparator<DeepKeyPart*> keyCmp;
			Comparator<DeepField*> fieldCmp;
			Collections::IteratorComparator<DeepKeyPart*, Iterator<DeepKeyPart*> > keyIterCmp(&keyCmp);
			Collections::IteratorComparator<DeepField*, Iterator<DeepField*> > fieldIterCmp(&fieldCmp);
			
			cmp = keyIterCmp.compare(iter.k1, iter.k2);
			if (cmp != 0) {
				return cmp;
			}
			
			return fieldIterCmp.compare(iter.f1, iter.f2);
		}
};

} } // namespace

FORCE_INLINE inttype Store::compare(const Store* store) const {
	Comparator<Store*> cmp;
	return cmp.compare(this,store);
}

#endif //COM_DEEPIS_DATASTORE_API_DEEP_TABLE_H_

