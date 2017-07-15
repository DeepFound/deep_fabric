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
#ifndef COM_DEEPIS_DATASTORE_API_DEEP_KEYBUILDER_H_
#define COM_DEEPIS_DATASTORE_API_DEEP_KEYBUILDER_H_

#include "com/deepis/datastore/api/deep/DeepTypes.h"

#include "com/deepis/db/store/relative/util/BasicArray.h"
#include "com/deepis/db/store/relative/core/RealTimeBuilder.h"

using namespace com::deepis::db::store::relative::util;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace core {

template<typename K>
class DeepKeyBuilder {
	protected:
		const SchemaBuilder<K>* m_schemaBuilder;
		
		FORCE_INLINE DeepKeyPart* getKeyPart() const {
			return getKeyPart(0);
		}

	public:
		DeepKeyBuilder() :
			m_schemaBuilder(null) {
		}

		~DeepKeyBuilder() {
		}
		
		FORCE_INLINE void setSchemaBuilder(SchemaBuilder<K>* schemaBuilder) {
			m_schemaBuilder = schemaBuilder;
		}

		FORCE_INLINE boolean hasHiddenKey() const {
			return m_schemaBuilder->hasHiddenKey();
		}

		FORCE_INLINE boolean hasReservedKey() const {
			return m_schemaBuilder->hasReservedKey();
		}

		FORCE_INLINE ushorttype getUnpackLength() const {
			return m_schemaBuilder->getUnpackLength();
		}

		FORCE_INLINE uinttype getKeyParts() const {
			return m_schemaBuilder->getKeyParts()->size();
		}

		FORCE_INLINE inttype getIgnoredKey() const {
			return m_schemaBuilder->getIgnoredKey();
		}

		FORCE_INLINE boolean getRequiresRekey() const {
			return m_schemaBuilder->getRequiresRekey();
		}

		FORCE_INLINE inttype hasIgnoredKey() const {
			return m_schemaBuilder->hasIgnoredKey();
		}

		FORCE_INLINE DeepKeyPart* getKeyPart(uinttype keyPart) const {
			#ifdef DEEP_DEBUG 
			if (keyPart >= getKeyParts()) {
				return null;
			}
			#endif
			return m_schemaBuilder->getKeyParts()->get(keyPart);
		}

		FORCE_INLINE ulongtype getReservedKey(const DeepComposite* key) const {
			return m_schemaBuilder->getReservedKey(key);
		}
		
		FORCE_INLINE bytearray getHiddenKey(DeepComposite* key) const {
			return m_schemaBuilder->getHiddenKey(key);
		}
		
		FORCE_INLINE ulongtype maxReservedKey() const {
			return m_schemaBuilder->maxReservedKey();
		}
};

template<typename T>
class DeepNumberKeyBuilder : public DeepKeyBuilder<T> {
	protected:
		using DeepKeyBuilder<T>::getKeyPart;
	public:
		FORCE_INLINE T getKey(const bytearray value) const {
			return *((T*) (value + getKeyPart()->getValueOffset()));
		}

		FORCE_INLINE T newKey() const {
			return (T) 0;
		}

		FORCE_INLINE T cloneKey(T key) const {
			return key;
		}

		FORCE_INLINE T fillKey(const bytearray bkey, T key) const {
			return *((T*) bkey);
		}

		FORCE_INLINE T fillKey(const bytearray pkey, const bytearray value, T key) const {
			return *((T*) (value + getKeyPart()->getValueOffset()));
		}

		FORCE_INLINE ushorttype populateKey(const bytearray value, bytearray keyBuffer) const {
			return getKeyPart()->populateKey(null, value, keyBuffer);
		}

		FORCE_INLINE void copyKeyBuffer(const T key, bytearray keyBuffer) const {
			memcpy(keyBuffer, &key, getKeyPart()->getUnpackLength());
		}

		FORCE_INLINE void copyKey(T in, T* out) const {
			*out = in;
		}

		FORCE_INLINE void clearKey(T key) const {
			// nothing to do
		}

		FORCE_INLINE void setIgnorePrimary(T key, boolean wildcard, boolean isEqual = true) const {
			// nothing to do
		}

		FORCE_INLINE boolean getIgnorePrimary(T key) const {
			return false;
		}

		FORCE_INLINE boolean getPrimaryKey(const T key, bytearray keyBuffer, ushorttype* keyPartOffset, ushorttype* keyPartLength) const {
			return false;
		}

		FORCE_INLINE bytearray getHiddenKey(T key) const {
			throw UnsupportedOperationException("Hidden key cannot be contained in this key");
		}

		FORCE_INLINE ulongtype getReservedKey(const T key) const {
			return (key <= 0) ? 0 : (ulongtype) key;
		}

		FORCE_INLINE ulongtype maxReservedKey() const {
			return (ulongtype) maxValue<T>();
		}
		
		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return getKeyPart()->getPackLength(key);
		}
		
		FORCE_INLINE uinttype getKeyParts() const {
			return 1;
		}

		FORCE_INLINE boolean isPrimitive() const {
			return true;
		}

		FORCE_INLINE boolean isEqual(const T key1, const T key2) const {
			return (key1 == key2);
		}

		FORCE_INLINE boolean isMatch(const Comparator<T>* comparator, const T key, const T matchKey) const {
			return (comparator->compare(key, matchKey) == 0);
		}

		FORCE_INLINE void setMatch(const T key, T matchKey) const {
			// nothing to do
		}

		FORCE_INLINE int toString(const T key, bytearray buffer) const {
			return numberToString<T>(Converter<T>::toData(key), buffer);
		}

		FORCE_INLINE inttype range(T startKey, T endKey, inttype size, const inttype* cardinality) const {
			return size;
		}
};

template<>
class KeyBuilder<DeepLongInt> : public DeepNumberKeyBuilder<DeepLongInt> {
};

template<>
class KeyBuilder<DeepULongInt> : public DeepNumberKeyBuilder<DeepULongInt> {
};

template<>
class KeyBuilder<DeepLongLong> : public DeepNumberKeyBuilder<DeepLongLong> {
};

template<>
class KeyBuilder<DeepULongLong> : public DeepNumberKeyBuilder<DeepULongLong> {
};

#ifndef DEEP_REDUCE_TEMPLATES
template<>
class KeyBuilder<DeepShortInt> : public DeepNumberKeyBuilder<DeepShortInt> {
};

template<>
class KeyBuilder<DeepUShortInt> : public DeepNumberKeyBuilder<DeepUShortInt> {
};

template<>
class KeyBuilder<DeepTinyInt> : public DeepNumberKeyBuilder<DeepTinyInt> {
};

template<>
class KeyBuilder<DeepUTinyInt> : public DeepNumberKeyBuilder<DeepUTinyInt> {
};

template<>
class KeyBuilder<DeepDouble> : public DeepNumberKeyBuilder<DeepDouble> {
};

template<>
class KeyBuilder<DeepFloat> : public DeepNumberKeyBuilder<DeepFloat> {
};
#endif

template<>
class KeyBuilder<DeepNByte*> : public DeepKeyBuilder<DeepNByte*> {
	public:
		FORCE_INLINE DeepNByte* newKey() const {
			//#ifdef DEEP_DEBUG 
			#if 1 
			DeepNByte* key = new DeepNByte(getKeyPart()->getUnpackLength());
			key->zero();
			return key;
			#else
			return new DeepNByte(getKeyPart()->getUnpackLength());
			#endif
		}

		FORCE_INLINE DeepNByte* cloneKey(DeepNByte* key) const {
			return (DeepNByte*) key->clone();
		}

		FORCE_INLINE DeepNByte* fillKey(const bytearray bkey, DeepNByte* key) const {
			ushorttype packLength = getKeyPart()->getPackLength(bkey);

			memcpy((bytearray) *key, bkey, packLength);

			key->reassign(((bytearray) *key), packLength);

			return key;
		}

		FORCE_INLINE DeepNByte* fillKey(const bytearray pkey, const bytearray value, DeepNByte* key) const {
			#ifdef DEEP_DEBUG 
			if (value == null) { 
				DEEP_LOG(ERROR, OTHER, "Invalid fill key: value is null\n");

				throw InvalidException("Invalid fill key: value is null");
			}
			#endif

			ushorttype packLength = getKeyPart()->populateKey(pkey, value, (bytearray) *key);

			key->reassign(((bytearray) *key), packLength);

			return key;
		}

		FORCE_INLINE ushorttype populateKey(const bytearray value, bytearray keyBuffer) const {
			return getKeyPart()->populateKey(null, value, keyBuffer);
		}

		FORCE_INLINE void copyKeyBuffer(const DeepNByte* key, bytearray keyBuffer) const {
			memcpy(keyBuffer, (bytearray) *key, getKeyPart()->getPackLength((bytearray) *key));
		}

		FORCE_INLINE void copyKey(DeepNByte* in, DeepNByte** out) const {
			// assume in->length is the correct pack length
			memcpy(*(*out), *in, in->length);

			(*out)->reassign(((bytearray) *(*out)), in->length);
		}

		FORCE_INLINE void clearKey(DeepNByte* key) const {
			key->zero();
		}

		FORCE_INLINE void setIgnorePrimary(DeepNByte* key, boolean wildcard, boolean isEqual = true) const {
			// nothing to do
		}

		FORCE_INLINE boolean getIgnorePrimary(DeepNByte* key) const {
			return false;
		}

		FORCE_INLINE boolean getPrimaryKey(const DeepNByte* key, bytearray keyBuffer, ushorttype* keyPartOffset, ushorttype* keyPartLength) const {
			return false;
		}

		FORCE_INLINE bytearray getHiddenKey(DeepNByte* key) const {
			throw UnsupportedOperationException("Hidden key cannot be contained in this key");
		}

		FORCE_INLINE ulongtype getReservedKey(const DeepNByte* key) const {
			return getKeyPart()->toULong((bytearray) *key);
		}

		FORCE_INLINE ulongtype maxReservedKey() const {
			return getKeyPart()->maxKey();
		}
		
		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return getKeyPart()->getPackLength(key);
		}
		
		FORCE_INLINE uinttype getKeyParts() const {
			return 1;
		}

		FORCE_INLINE boolean isPrimitive() const {
			return false;
		}

		FORCE_INLINE boolean isEqual(const DeepNByte* key1, const DeepNByte* key2) const {
			if (key1->length == key2->length) {
				return (memcmp(((bytearray) *key1), ((bytearray) *key2), key1->length) == 0);

			} else {
				return false;
			}
		}

		FORCE_INLINE boolean isMatch(const Comparator<DeepNByte*>* comparator, const DeepNByte* key, const DeepNByte* matchKey) const {
			return (comparator->compare(key, matchKey) == 0);
		}

		FORCE_INLINE void setMatch(const DeepNByte* key, DeepNByte* matchKey) const {
			// nothing to do
		}

		FORCE_INLINE int toString(const DeepNByte* key, bytearray buffer) const {
			return getKeyPart()->toString(((bytearray) *key), buffer);
		}

		FORCE_INLINE inttype range(DeepNByte* startKey, DeepNByte* endKey, inttype size, const inttype* cardinality) const {
			return size;
		}
};

template<>
class KeyBuilder<DeepComposite*> : public DeepKeyBuilder<DeepComposite*> {
	protected:
		using DeepKeyBuilder<DeepComposite*>::getKeyPart;

	public:
		KeyBuilder() {
		}

		FORCE_INLINE DeepComposite* newKey() const {
			//#ifdef DEEP_DEBUG 
			#if 1
			DeepComposite* key = new DeepComposite(getUnpackLength());
			key->zero();
			return key;
			#else
			return new DeepComposite(getUnpackLength());
			#endif
		}

		FORCE_INLINE DeepComposite* cloneKey(DeepComposite* key) const {
			DeepComposite* bytes = new DeepComposite(key->length /* XXX: don't include mask or weight */);
			memcpy((bytearray) *bytes, (bytearray) *key, key->length);
			return bytes;
		}

		FORCE_INLINE DeepComposite* fillKey(const bytearray bkey, DeepComposite* key) const {
			ushorttype packLength = getPackLength(bkey);

			memcpy((bytearray) *key, bkey, packLength);

			key->reassignComposite(((bytearray) *key), packLength, ~0, -1);

			return key;
		}

		FORCE_INLINE DeepComposite* fillKey(const bytearray pkey, const bytearray value, DeepComposite* key) const {
			#ifdef DEEP_DEBUG 
			if (value == null) { 
				DEEP_LOG(ERROR, OTHER, "Invalid fill key: value is null\n");

				throw InvalidException("Invalid fill key: value is null");
			}
			#endif

			ushorttype packLength = populateKey(pkey, value, (bytearray) *key);

			key->reassignComposite(((bytearray) *key), packLength, ~0, -1);

			#ifdef DEEP_DEBUG
			if (packLength != getPackLength((bytearray) *key)) {
				DEEP_LOG(ERROR, OTHER, "Invalid fill key: packLength mismatch\n");

				throw InvalidException("Invalid fill key: packLength mismatch");
			}
			#endif

			return key;
		}

		FORCE_INLINE DeepComposite* reassignKey(DeepComposite* key, const bytearray bkey) const {
			ushorttype packLength = getPackLength(bkey);

			key->reassignComposite(bkey, packLength, key->getKeyPartMask(), key->getKeyPartWeight(), key->getKeyPartMatch());

			return key;
		}

		FORCE_INLINE ushorttype populateKey(const bytearray value, bytearray keyBuffer) const {
			return populateKey(null, value, keyBuffer);
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) const {
			register uinttype i = 0;
			register ushorttype keyOffset = 0;

			do {
				keyOffset += getKeyPart(i)->populateKey(pkey, value, keyBuffer + keyOffset);

			} while (++i < getKeyParts());

			return keyOffset;
		}

		FORCE_INLINE void copyKeyBuffer(const DeepComposite* key, bytearray keyBuffer) const {
			memcpy(keyBuffer, (bytearray) *key, getPackLength((bytearray) *key));
		}

		FORCE_INLINE void copyKey(DeepComposite* in, DeepComposite** out) const {
			// assume in->length is the correct pack length
			memcpy(*(*out), *in, in->length);

			// XXX: not reassignComposite
			(*out)->reassign(((bytearray) *(*out)), in->length);
		}

		FORCE_INLINE void clearKey(DeepComposite* key) const {
			key->zero();
		}

		FORCE_INLINE void setIgnorePrimary(DeepComposite* key, boolean wildcard, boolean isEqual = true) const {
			if (key != null) {
				if (getIgnoredKey() != -1) {
					if (wildcard == true) {
						if (key->getKeyPartMask() == (uinttype) ~0) {
							key->setKeyPartMask((1 << getIgnoredKey()) - 1);
							key->setIgnorePrimary(true);
							if (isEqual == true) {
								key->setKeyPartWeight(0);
							}
						}

					} else if (key->getIgnorePrimary() == true) {
						key->setKeyPartMask(~0);
						key->setIgnorePrimary(false);
						if (isEqual == true) {
							key->setKeyPartWeight(-1);
						}
					}
				}
			}
		}

		FORCE_INLINE boolean getIgnorePrimary(DeepComposite* key) const {
			return key->getIgnorePrimary();
		}

		FORCE_INLINE boolean getPrimaryKey(const DeepComposite* key, bytearray keyBuffer, ushorttype* keyPartOffset, ushorttype* keyPartLength) const {
			register uinttype i = 0;
			register uinttype keyOffset = 0;
			register uinttype primaryKeyParts = 0;

			do {
				register DeepKeyPart* keyPart = getKeyPart(i);

				register ushorttype packLength = keyPart->getPackLength(((bytearray) *key) + keyOffset);

				if (keyPart->getPrimaryPosition() != -1) {
					keyPartOffset[keyPart->getPrimaryPosition()] = keyOffset;
					keyPartLength[keyPart->getPrimaryPosition()] = packLength;
					primaryKeyParts++;
				}

				keyOffset += packLength;

			} while (++i < getKeyParts());

			i = 0;
			keyOffset = 0;

			do {
				memcpy(keyBuffer + keyOffset, ((bytearray) *key) + keyPartOffset[i], keyPartLength[i]);

				keyOffset += keyPartLength[i];

			} while (++i < primaryKeyParts);

			return true;
		}

		FORCE_INLINE void packKey(DeepComposite* key, bytearray keyBuffer) const {
			register uinttype i = 0;
			register inttype keyOffset = 0;
			register inttype bufOffset = 0;
			register uinttype keyPartMask = key->getKeyPartMask();

			do {
				register DeepKeyPart* keyPart = getKeyPart(i);

				register ushorttype packLength = keyPart->getPackLength(keyBuffer + bufOffset);

				memcpy(((bytearray) *key) + keyOffset, keyBuffer + bufOffset, packLength);

				keyPartMask >>= 1;

				keyOffset += packLength;
				bufOffset += keyPart->getUnpackLength();

			} while ((++i < getKeyParts()) && (keyPartMask != 0));

			key->reassign((const bytearray) *key, keyOffset);
		}

		FORCE_INLINE void unpackKey(const DeepComposite* key, bytearray keyBuffer) const {
			register uinttype i = 0;
			register inttype keyOffset = 0;
			register inttype bufOffset = 0;

			do {
				register DeepKeyPart* keyPart = getKeyPart(i);

				register ushorttype packLength = keyPart->getPackLength(((bytearray) *key) + keyOffset);

				memcpy(keyBuffer + bufOffset, ((bytearray) *key) + keyOffset, packLength);

				keyOffset += packLength;
				bufOffset += keyPart->getUnpackLength();

			} while (++i < getKeyParts());
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			register ushorttype length = 0;
			for (uinttype i=0; i<getKeyParts(); i++) {
				length += getKeyPart(i)->getPackLength(key + length);
			}

			return length;
		}

		FORCE_INLINE boolean isPrimitive() const {
			return false;
		}

		FORCE_INLINE boolean isEqual(const DeepComposite* key1, const DeepComposite* key2) const {
			if (key1->length == key2->length) {
				return (memcmp(((bytearray) *key1), ((bytearray) *key2), key1->length) == 0);

			} else {
				return false;
			}
		}

		FORCE_INLINE boolean isMatch(const Comparator<DeepComposite*>* comparator, const DeepComposite* key, const DeepComposite* matchKey) const {
			boolean status = true;

			inttype pos = 0;

			inttype keyPartWeight = matchKey->getKeyPartWeight();
			((DeepComposite*) matchKey)->setKeyPartWeight(0);

			if (comparator->compare(key, matchKey, &pos) != 0) {
				// (pos-1) is last matched key part as long as mask is full
				status = ((pos-1) >= matchKey->getKeyPartMatch());
			}

			((DeepComposite*) matchKey)->setKeyPartWeight(keyPartWeight);

			return status;
		}

		FORCE_INLINE void setMatch(const DeepComposite* key, DeepComposite* matchKey) const {
			matchKey->setKeyPartMatch(key->getKeyPartMatch());
		}

		FORCE_INLINE int toString(const DeepComposite* key, bytearray buffer) const {
			register uinttype i = 0;
			register inttype offset = 0;
			register inttype keyOffset = 0;
			register uinttype keyPartMask = key->getKeyPartMask();
			register boolean hasIgnored = false;

			do {
				register DeepKeyPart* keyPart = getKeyPart(i);

				if (i > 0) {
					offset += sprintf(buffer + offset, ".");
				}

				keyPartMask >>= 1;

				if ((keyPart->isIgnored() == true) && (hasIgnored == false)) {
					strncpy(buffer + offset, "!!< ", 4);
					hasIgnored = true;
					offset += 4;
				}

				offset += keyPart->toString(((bytearray) *key) + keyOffset, buffer + offset);

				keyOffset += keyPart->getPackLength(((bytearray) *key) + keyOffset);

			} while ((++i < getKeyParts()) && (keyPartMask != 0));
	
			if (hasIgnored == true) {
				strncpy(buffer + offset, " >!!", 4);
				offset += 4;
			}

			return offset;
		}

		FORCE_INLINE inttype range(DeepComposite* startKey, DeepComposite* endKey, inttype size, const inttype* cardinality) const {
			uinttype keyPartMask = (startKey->getKeyPartMask() < endKey->getKeyPartMask()) ? startKey->getKeyPartMask() : endKey->getKeyPartMask();

			inttype numKeys = 0;
			for (uinttype i=0; (i<getKeyParts()) && (keyPartMask != 0); i++, keyPartMask>>=1) {
				numKeys += cardinality[i];
			}

			// TODO: these guards should not be necessary
			if (numKeys == 0) {
				numKeys = 1;

			} else if (numKeys > size) {
				numKeys = size;
			}

			return size / numKeys;
		}
};


} } } } } } // namespace

#endif //COM_DEEPIS_DATASTORE_API_DEEP_KEYBUILDER_H_
