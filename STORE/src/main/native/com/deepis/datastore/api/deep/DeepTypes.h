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
#ifndef COM_DEEPIS_DATASTORE_API_DEEP_TYPES_H_
#define COM_DEEPIS_DATASTORE_API_DEEP_TYPES_H_

#include <float.h>
#include <limits.h>

#include "cxx/util/Logger.h"
#include "cxx/lang/nbyte.h"
#include "cxx/lang/UnsupportedOperationException.h"

#include "com/deepis/datastore/api/deep/DeepTypeDefs.h"
#include "com/deepis/datastore/api/deep/DeepKeyPart.h"

using namespace cxx::lang;
using namespace cxx::util;
using namespace com::deepis::datastore::api;

typedef inttype    DeepLongInt;
typedef uinttype   DeepULongInt;
typedef shorttype  DeepShortInt;
typedef ushorttype DeepUShortInt;
typedef longtype   DeepLongLong;
typedef ulongtype  DeepULongLong;
typedef chartype   DeepTinyInt;
typedef uchartype  DeepUTinyInt;
typedef doubletype DeepDouble;
typedef floattype  DeepFloat;

#if defined(__i386__)
	#error The x86_32 architecture is unsupported!
#endif
#if defined(WORDS_BIGENDIAN)
	#error Big endian encoding is unsupported!
#endif

#ifndef dp_sint2korr
#define dp_sint2korr(A)	(shorttype) (((shorttype) ((uchartype) (A)[0])) +\
				 ((shorttype) ((shorttype) (A)[1]) << 8))
#endif

#ifndef dp_sint3korr
#define dp_sint3korr(A)	((inttype) ((((uchartype) (A)[2]) & 128) ? \
				  (((uinttype) 255L << 24) | \
				   (((uinttype) (uchartype) (A)[2]) << 16) |\
				   (((uinttype) (uchartype) (A)[1]) << 8) | \
				   ((uinttype) (uchartype) (A)[0])) : \
				  (((uinttype) (uchartype) (A)[2]) << 16) |\
				  (((uinttype) (uchartype) (A)[1]) << 8) | \
				  ((uinttype) (uchartype) (A)[0])))
#endif

#ifndef dp_sint4korr
#define dp_sint4korr(A)	(inttype) (((inttype) ((uchartype) (A)[0])) +\
				(((inttype) ((uchartype) (A)[1]) << 8)) +\
				(((inttype) ((uchartype) (A)[2]) << 16)) +\
				(((inttype) ((shorttype) (A)[3]) << 24)))
#endif

#ifndef dp_uint2korr
#define dp_uint2korr(A)	(ushorttype) (((ushorttype) ((uchartype) (A)[0])) +\
				  ((ushorttype) ((uchartype) (A)[1]) << 8))
#endif

#ifndef dp_uint3korr
#define dp_uint3korr(A)	(uinttype) (((uinttype) ((uchartype) (A)[0])) +\
				  (((uinttype) ((uchartype) (A)[1])) << 8) +\
				  (((uinttype) ((uchartype) (A)[2])) << 16))
#endif

#ifndef dp_uint4korr
#define dp_uint4korr(A)	(uinttype) (((uinttype) ((uchartype) (A)[0])) +\
				  (((uinttype) ((uchartype) (A)[1])) << 8) +\
				  (((uinttype) ((uchartype) (A)[2])) << 16) +\
				  (((uinttype) ((uchartype) (A)[3])) << 24))
#endif

#ifndef dp_int2store
#define dp_int2store(T,A)       do { uinttype def_temp= (uint) (A) ;\
                                  *((uchartype*) (T))=  (uchartype)(def_temp); \
                                   *((uchartype*) (T)+1)=(uchartype)((def_temp >> 8)); \
                             } while(0)
#endif

#ifndef dp_int3store
#define dp_int3store(T,A)       do { /*lint -save -e734 */\
                                  *((uchartype*)(T))=(uchartype) ((A));\
                                  *((uchartype*) (T)+1)=(uchartype) (((A) >> 8));\
                                  *((uchartype*)(T)+2)=(uchartype) (((A) >> 16)); \
                                  /*lint -restore */} while(0)
#endif

#ifndef dp_int4store
#define dp_int4store(T,A)       do { *((chartype *)(T))=(chartype) ((A));\
                                  *(((chartype *)(T))+1)=(chartype) (((A) >> 8));\
                                  *(((chartype *)(T))+2)=(chartype) (((A) >> 16));\
                                  *(((chartype *)(T))+3)=(chartype) (((A) >> 24)); } while(0)
#endif

#ifndef dp_shortget
#define dp_shortget(V,M)	do { V = dp_sint2korr(M); } while(0)
#endif

#ifndef dp_longget
#define dp_longget(V,M)	do { V = dp_sint4korr(M); } while(0)
#endif

#define dp_sint3max 8388607
#define dp_uint3max 16777215

FORCE_INLINE uinttype getMultiByteTextLength(uinttype length, inttype charset, bytearray value) {
	uinttype valueLength = length;
	uinttype minLen = valueLength / DeepStore::getCharset()->getMultiByteMaxLen(charset);

	// strip space padding  **see ct_defs.cc::get_multibyte_text_length
	while ((valueLength > minLen) && (value[valueLength - 1] == 0x20)) {
		valueLength--;
	}

	return valueLength;
}

FORCE_INLINE void padMultiByteText(uinttype length, uinttype valueLength, bytearray value) {
	// pad with spaces  **see ct_defs.cc::get_multibyte_text_length
	if (valueLength < length) {
		memset(value + valueLength, 0x20, length - valueLength);
	}
}

template<typename T>
FORCE_INLINE inttype compareNumberByteArray(const bytearray o1, const bytearray o2) {
	return *((T*) o1) < *((T*) o2) ? -1 : ((*((T*) o1) == *((T*) o2)) ? 0 : 1);
}

template<typename T>
FORCE_INLINE inttype compareNumberNullByteArray(const bytearray key1, const bytearray key2, boolean* nullKey) {
	if (key1[0]) {
		if (key2[0]) {
			if (nullKey != null) {
				*nullKey = true;
			}
			return 0;

		} else {
			return -1;
		}

	} else if (key2[0]) {
		return 1;

	} else {
		return compareNumberByteArray<T>(key1 + 1, key2 + 1);
	}
}

template<typename T>
FORCE_INLINE int numberToString(const bytearray key, bytearray buffer) {
	throw UnsupportedOperationException("Invalid type in numberToString");
}

template<>
FORCE_INLINE int numberToString<DeepLongInt>(const bytearray key, bytearray buffer) {
	return sprintf(buffer, "%d", *((DeepLongInt*) key));
}

template<>
FORCE_INLINE int numberToString<DeepULongInt>(const bytearray key, bytearray buffer) {
	return sprintf(buffer, "%u", *((DeepULongInt*) key));
}

template<>
FORCE_INLINE int numberToString<DeepShortInt>(const bytearray key, bytearray buffer) {
	return sprintf(buffer, "%hd", *((DeepShortInt*) key));
}

template<>
FORCE_INLINE int numberToString<DeepUShortInt>(const bytearray key, bytearray buffer) {
	return sprintf(buffer, "%hu", *((DeepUShortInt*) key));
}

template<>
FORCE_INLINE int numberToString<DeepLongLong>(const bytearray key, bytearray buffer) {
	return sprintf(buffer, "%lld", *((DeepLongLong*) key));
}

template<>
FORCE_INLINE int numberToString<DeepULongLong>(const bytearray key, bytearray buffer) {
	return sprintf(buffer, "%llu", *((DeepULongLong*) key));
}

template<>
FORCE_INLINE int numberToString<DeepTinyInt>(const bytearray key, bytearray buffer) {
	int i = *((DeepTinyInt*) key);
	return sprintf(buffer, "%d", i);
}

template<>
FORCE_INLINE int numberToString<DeepUTinyInt>(const bytearray key, bytearray buffer) {
	int i = *((DeepUTinyInt*) key);
	return sprintf(buffer, "%d", i);
}

template<>
FORCE_INLINE int numberToString<DeepDouble>(const bytearray key, bytearray buffer) {
	return sprintf(buffer, "%f", *((DeepDouble*) key));
}

template<>
FORCE_INLINE int numberToString<DeepFloat>(const bytearray key, bytearray buffer) {
	return sprintf(buffer, "%f", *((DeepFloat*) key));
}

FORCE_INLINE int nullToString(bytearray buffer) {
	return sprintf(buffer, "nullkey");
}

template<typename T>
FORCE_INLINE ulongtype numberToULong(const bytearray key) {
	T k = *((T*) key);

	return (k <= 0) ? 0 : (ulongtype) k;
}

template<typename T>
FORCE_INLINE ulongtype numberNullToULong(const bytearray key) {
	if (key[0]) {
		return 0;

	} else {
		return numberToULong<T>(key + 1);
	}
}

template<typename T>
FORCE_INLINE T maxValue() {
	throw UnsupportedOperationException("Invalid type in maxValue");
}

template<>
FORCE_INLINE DeepLongInt maxValue<DeepLongInt>() {
	return INT_MAX;
}

template<>
FORCE_INLINE DeepULongInt maxValue<DeepULongInt>() {
	return UINT_MAX;
}

template<>
FORCE_INLINE DeepShortInt maxValue<DeepShortInt>() {
	return SHRT_MAX;
}

template<>
FORCE_INLINE DeepUShortInt maxValue<DeepUShortInt>() {
	return USHRT_MAX;
}

template<>
FORCE_INLINE DeepLongLong maxValue<DeepLongLong>() {
	return LONG_MAX;
}

template<>
FORCE_INLINE DeepULongLong maxValue<DeepULongLong>() {
	return ULONG_MAX;
}

template<>
FORCE_INLINE DeepTinyInt maxValue<DeepTinyInt>() {
	return CHAR_MAX;
}

template<>
FORCE_INLINE DeepUTinyInt maxValue<DeepUTinyInt>() {
	return UCHAR_MAX;
}

template<>
FORCE_INLINE DeepDouble maxValue<DeepDouble>() {
	return DBL_MAX;
}

template<>
FORCE_INLINE DeepFloat maxValue<DeepFloat>() {
	return FLT_MAX;
}

#include "com/deepis/datastore/api/deep/Store.h"

FORCE_INLINE static inttype getCharacterSet(const DeepKeyPart* key) {
	return key->getField()->getCharacterSet();
}

template<typename T>
class DeepKeyPartNumber : public DeepKeyPart {
	public:
		DeepKeyPartNumber(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			return compareNumberByteArray<T>(key1, key2);
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			memcpy(keyBuffer, (value + m_valueOffset), m_length);

			return m_length;
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			return numberToULong<T>(key);
		}

		FORCE_INLINE ulongtype maxKey() const {
			return (ulongtype) ::maxValue<T>();
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			return numberToString<T>(key, buffer);
		}
};

template<typename T>
class DeepKeyPartNumberNull : public DeepKeyPart {
	public:
		DeepKeyPartNumberNull(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			return compareNumberNullByteArray<T>(key1, key2, nullKey);
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			uchartype nullKey = isNull(value);

			memcpy(keyBuffer, &nullKey, 1);

			if (nullKey == 0) {
				memcpy((keyBuffer + 1), (value + m_valueOffset), m_length);
			}

			return m_unpackLength;
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			return numberNullToULong<T>(key);
		}

		FORCE_INLINE ulongtype maxKey() const {
			return (ulongtype) ::maxValue<T>();
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (key[0]) {
				return nullToString(buffer);

			} else {
				return numberToString<T>(key + 1, buffer);
			}
		}
};

class DeepKeyPartMediumInt : public DeepKeyPart {
	public:
		DeepKeyPartMediumInt(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			if (maybeNull()) {
				if (key1[0]) {
					if (key2[0]) {
						if (nullKey != null) {
							*nullKey = true;
						}
						return 0;

					} else {
						return -1;
					}

				} else if (key2[0]) {
					return 1;

				} else {
					longtype k1 = dp_sint3korr(key1 + 1);
					longtype k2 = dp_sint3korr(key2 + 1);

					return (k1 < k2) ? -1 : ((k1 == k2) ? 0 : 1);
				}

			} else {
				longtype k1 = dp_sint3korr(key1);
				longtype k2 = dp_sint3korr(key2);

				return (k1 < k2) ? -1 : ((k1 == k2) ? 0 : 1);
			}
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			if (maybeNull()) {
				uchartype nullKey = isNull(value);

				memcpy(keyBuffer, &nullKey, 1);

				if (nullKey == 0) {
					memcpy((keyBuffer + 1), (value + m_valueOffset), m_length);
				}

			} else {
				memcpy(keyBuffer, (value + m_valueOffset), m_length);
			}

			return m_unpackLength;
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			if (maybeNull() && key[0]) {
				return 0;

			} else {
				longtype k = dp_sint3korr(key + m_nullBytes);

				return (k < 0) ? 0 : (ulongtype) k;
			}
		}

		FORCE_INLINE ulongtype maxKey() const {
			return (ulongtype) dp_sint3max;
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (maybeNull() && key[0]) {
				return nullToString(buffer);

			} else {
				longtype k = dp_sint3korr(key + m_nullBytes);

				return sprintf(buffer, "%lld", k);
			}
		}
};

class DeepKeyPartUMediumInt : public DeepKeyPart {
	public:
		DeepKeyPartUMediumInt(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			if (maybeNull()) {
				if (key1[0]) {
					if (key2[0]) {
						if (nullKey != null) {
							*nullKey = true;
						}
						return 0;

					} else {
						return -1;
					}

				} else if (key2[0]) {
					return 1;

				} else {
					ulongtype k1 = dp_uint3korr(key1 + 1);
					ulongtype k2 = dp_uint3korr(key2 + 1);

					return (k1 < k2) ? -1 : ((k1 == k2) ? 0 : 1);
				}

			} else {
				ulongtype k1 = dp_uint3korr(key1);
				ulongtype k2 = dp_uint3korr(key2);

				return (k1 < k2) ? -1 : ((k1 == k2) ? 0 : 1);
			}
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			if (maybeNull()) {
				uchartype nullKey = isNull(value);

				memcpy(keyBuffer, &nullKey, 1);

				if (nullKey == 0) {
					memcpy((keyBuffer + 1), (value + m_valueOffset), m_length);
				}

			} else {
				memcpy(keyBuffer, (value + m_valueOffset), m_length);
			}

			return m_unpackLength;
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			if (maybeNull() && key[0]) {
				return 0;

			} else {
				return dp_uint3korr(key + m_nullBytes);
			}
		}

		FORCE_INLINE ulongtype maxKey() const {
			return dp_uint3max;
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (maybeNull() && key[0]) {
				return nullToString(buffer);

			} else {
				ulongtype k = dp_uint3korr(key + m_nullBytes);

				return sprintf(buffer, "%llu", k);
			}
		}
};

class DeepKeyPartVarChar : public DeepKeyPart {
	public:
		DeepKeyPartVarChar(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_nullBytes + m_lengthBytes + ((maybeNull() && key[0]) ? 0 : *((ushorttype*) (key + m_nullBytes)));
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			if (maybeNull()) {
				if (key1[0]) {
					if (key2[0]) {
						if (nullKey != null) {
							*nullKey = true;
						}
						return 0;

					} else {
						return -1;
					}

				} else if (key2[0]) {
					return 1;

				} else {
					ushorttype len1 = *((ushorttype*) (key1 + 1));
					ushorttype len2 = *((ushorttype*) (key2 + 1));

					#if COM_DEEPIS_DB_CHARSET
					return DeepStore::getCharset()->compare(getCharacterSet(this), key1 + 1 + sizeof(ushorttype), len1, key2 + 1 + sizeof(ushorttype), len2);
					#else
					int cmp = strncmp(key1 + 1 + sizeof(ushorttype), key2 + 1 + sizeof(ushorttype), min(len1, len2));

					if ((cmp == 0) && (len1 != len2)) {
						cmp = (len1 > len2) ? 1 : -1;
					}

					return cmp;
					#endif
				}

			} else {
				ushorttype len1 = *((ushorttype*) key1);
				ushorttype len2 = *((ushorttype*) key2);

				#if COM_DEEPIS_DB_CHARSET
				return DeepStore::getCharset()->compare(getCharacterSet(this), key1 + sizeof(ushorttype), len1, key2 + sizeof(ushorttype), len2);
				#else
				int cmp = strncmp(key1 + sizeof(ushorttype), key2 + sizeof(ushorttype), min(len1, len2));

				if ((cmp == 0) && (len1 != len2)) {
					cmp = (len1 > len2) ? 1 : -1;
				}

				return cmp;
				#endif
			}
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			if (maybeNull()) {
				uchartype nullKey = isNull(value);

				memcpy(keyBuffer, &nullKey, 1);

				if (nullKey == 0) {
					// (value + m_valueOffset) == start of variable length data (i.e. blob data)
					const bytearray ptr = value + m_valueOffset;

					uinttype offset = 0;
					for (shorttype i=0; i<m_variablePosition; i++) {
						offset += (*((uinttype*) (ptr + offset))) + sizeof(uinttype);
					}

					ushorttype length = *((uinttype*) (ptr + offset));
					bytearray data = (bytearray) (ptr + offset + sizeof(uinttype));

					#if COM_DEEPIS_DB_CHARSET
					length = DeepStore::getCharset()->populate(getCharacterSet(this), keyBuffer + 1 + sizeof(ushorttype), data, length, m_length);
					#else
					length = (length < m_length) ? length : m_length;
					memcpy(keyBuffer + 1 + sizeof(ushorttype), data, length);
					#endif

					memcpy(keyBuffer + 1, &length, sizeof(ushorttype));
				}

			} else {
				// (value + m_valueOffset) == start of variable length data (i.e. blob data)
				const bytearray ptr = value + m_valueOffset;

				uinttype offset = 0;
				for (shorttype i=0; i<m_variablePosition; i++) {
					offset += (*((uinttype*) (ptr + offset))) + sizeof(uinttype);
				}

				ushorttype length = *((uinttype*) (ptr + offset));
				bytearray data = (bytearray) (ptr + offset + sizeof(uinttype));

				#if COM_DEEPIS_DB_CHARSET
				length = DeepStore::getCharset()->populate(getCharacterSet(this), keyBuffer + sizeof(ushorttype), data, length, m_length);
				#else
				length = (length < m_length) ? length : m_length;
				memcpy(keyBuffer + sizeof(ushorttype), data, length);
				#endif

				memcpy(keyBuffer, &length, sizeof(ushorttype));
			}

			return DeepKeyPartVarChar::getPackLength(keyBuffer);
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long");

			throw UnsupportedOperationException("Key part cannot be converted to long");
		}

		FORCE_INLINE ulongtype maxKey() const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long in maxKey");

			throw UnsupportedOperationException("Key part cannot be converted to long in maxKey");
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (maybeNull() && key[0]) {
				return nullToString(buffer);

			} else {
				ushorttype strlen = *((ushorttype*) (key + m_nullBytes));

				memcpy(buffer, key + m_nullBytes + sizeof(ushorttype), strlen);

				return strlen;
			}
		}
};

class DeepKeyPartFixedVarChar : public DeepKeyPart {
	public:
		DeepKeyPartFixedVarChar(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			if (maybeNull()) {
				if (key1[0]) {
					if (key2[0]) {
						if (nullKey != null) {
							*nullKey = true;
						}
						return 0;

					} else {
						return -1;
					}

				} else if (key2[0]) {
					return 1;

				} else {
					ushorttype len1 = *((ushorttype*) (key1 + 1));
					ushorttype len2 = *((ushorttype*) (key2 + 1));

					#if COM_DEEPIS_DB_CHARSET
					return DeepStore::getCharset()->compare(getCharacterSet(this), key1 + 1 + sizeof(ushorttype), len1, key2 + 1 + sizeof(ushorttype), len2);
					#else
					int cmp = strncmp(key1 + 1 + sizeof(ushorttype), key2 + 1 + sizeof(ushorttype), min(len1, len2));

					if ((cmp == 0) && (len1 != len2)) {
						cmp = (len1 > len2) ? 1 : -1;
					}

					return cmp;
					#endif
				}

			} else {
				ushorttype len1 = *((ushorttype*) key1);
				ushorttype len2 = *((ushorttype*) key2);
				#if COM_DEEPIS_DB_CHARSET
				return DeepStore::getCharset()->compare(getCharacterSet(this), key1 + sizeof(ushorttype), len1, key2 + sizeof(ushorttype), len2);
				#else
				int cmp = strncmp(key1 + sizeof(ushorttype), key2 + sizeof(ushorttype), min(len1, len2));

				if ((cmp == 0) && (len1 != len2)) {
					cmp = (len1 > len2) ? 1 : -1;
				}

				return cmp;
				#endif
			}
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			// XXX: needed here for 'fixed' length var types (see KeyBuilder->isEqual())
			memset(keyBuffer, 0, getUnpackLength());

			if (maybeNull()) {
				uchartype nullKey = isNull(value);

				memcpy(keyBuffer, &nullKey, 1);

				if (nullKey == 0) {
					ushorttype length = *((uchartype*) (value + m_valueOffset));
					bytearray data = (bytearray) (value + m_valueOffset + sizeof(uchartype));

					#if COM_DEEPIS_DB_CHARSET
					length = DeepStore::getCharset()->populate(getCharacterSet(this), keyBuffer + 1 + sizeof(ushorttype), data, length, m_length);
					#else
					length = (length < m_length) ? length : m_length;
					memcpy((keyBuffer + 1 + sizeof(ushorttype)), data, length);
					#endif

					memcpy((keyBuffer + 1), &length, sizeof(ushorttype));
				}

			} else {
				ushorttype length = *((uchartype*) (value + m_valueOffset));
				bytearray data = (bytearray) (value + m_valueOffset + sizeof(uchartype));

				#if COM_DEEPIS_DB_CHARSET
				length = DeepStore::getCharset()->populate(getCharacterSet(this), keyBuffer + sizeof(ushorttype), data, length, m_length);
				#else
				length = (length < m_length) ? length : m_length;
				memcpy((keyBuffer + sizeof(ushorttype)), data, length);
				#endif

				memcpy(keyBuffer, &length, sizeof(ushorttype));
			}

			return m_unpackLength;
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long");

			throw UnsupportedOperationException("Key part cannot be converted to long");
		}

		FORCE_INLINE ulongtype maxKey() const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long in maxKey");

			throw UnsupportedOperationException("Key part cannot be converted to long in maxKey");
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (maybeNull() && key[0]) {
				return nullToString(buffer);

			} else {
				ushorttype strlen = *((ushorttype*) (key + m_nullBytes));

				memcpy(buffer, key + m_nullBytes + sizeof(ushorttype), strlen);

				return strlen;
			}
		}
};

class DeepKeyPartChar : public DeepKeyPart {
	public:
		DeepKeyPartChar(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			if (maybeNull()) {
				if (key1[0]) {
					if (key2[0]) {
						if (nullKey != null) {
							*nullKey = true;
						}
						return 0;

					} else {
						return -1;
					}

				} else if (key2[0]) {
					return 1;

				} else {
					#if COM_DEEPIS_DB_CHARSET
					return DeepStore::getCharset()->compare(getCharacterSet(this), key1 + 1, m_length, key2 + 1, m_length);
					#else
					return strncmp(key1 + 1, key2 + 1, m_length);
					#endif
				}

			} else {
				#if COM_DEEPIS_DB_CHARSET
				return DeepStore::getCharset()->compare(getCharacterSet(this), key1, m_length, key2, m_length);
				#else
				return strncmp(key1, key2, m_length);
				#endif
			}
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			if (maybeNull()) {
				uchartype nullKey = isNull(value);

				memcpy(keyBuffer, &nullKey, 1);

				if (nullKey == 0) {
					memcpy((keyBuffer + 1), (value + m_valueOffset), m_length);
				}

			} else {
				memcpy(keyBuffer, (value + m_valueOffset), m_length);
			}

			return m_unpackLength;
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long");

			throw UnsupportedOperationException("Key part cannot be converted to long");
		}

		FORCE_INLINE ulongtype maxKey() const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long in maxKey");

			throw UnsupportedOperationException("Key part cannot be converted to long in maxKey");
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (maybeNull() && key[0]) {
				return nullToString(buffer);

			} else {
				memcpy(buffer, key + m_nullBytes, m_length);

				return m_length;
			}
		}
};

class DeepKeyPartCharMultiByte : public DeepKeyPart {
	public:
		DeepKeyPartCharMultiByte(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {

			// XXX: stored as variable size fields in the value but keys are still fixed as they need to match Deep protocol
			m_lengthBytes   = 0;
			m_unpackLength  = m_nullBytes + m_length;
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			if (maybeNull()) {
				if (key1[0]) {
					if (key2[0]) {
						if (nullKey != null) {
							*nullKey = true;
						}
						return 0;

					} else {
						return -1;
					}

				} else if (key2[0]) {
					return 1;

				} else {
					#if COM_DEEPIS_DB_CHARSET
					return DeepStore::getCharset()->compare(getCharacterSet(this), key1 + 1, m_length, key2 + 1, m_length);
					#else
					return strncmp(key1 + 1, key2 + 1, m_length);
					#endif
				}

			} else {
				#if COM_DEEPIS_DB_CHARSET
				return DeepStore::getCharset()->compare(getCharacterSet(this), key1, m_length, key2, m_length);
				#else
				return strncmp(key1, key2, m_length);
				#endif
			}
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			if (maybeNull()) {
				uchartype nullKey = isNull(value);

				memcpy(keyBuffer, &nullKey, 1);

				if (nullKey == 0) {
					// (value + m_valueOffset) == start of variable length data (i.e. blob data)
					const bytearray ptr = value + m_valueOffset;

					uinttype offset = 0;
					for (shorttype i=0; i<m_variablePosition; i++) {
						offset += (*((uinttype*) (ptr + offset))) + sizeof(uinttype);
					}

					ushorttype length = *((uinttype*) (ptr + offset));
					bytearray data = (bytearray) (ptr + offset + sizeof(uinttype));

					#if COM_DEEPIS_DB_CHARSET
					length = DeepStore::getCharset()->populate(getCharacterSet(this), keyBuffer + 1, data, length, m_length);
					#else
					length = (length < m_length) ? length : m_length;
					memcpy(keyBuffer + 1, data, length);
					#endif

					padMultiByteText(m_length, length, keyBuffer + 1);
				}

			} else {
				// (value + m_valueOffset) == start of variable length data (i.e. blob data)
				const bytearray ptr = value + m_valueOffset;

				uinttype offset = 0;
				for (shorttype i=0; i<m_variablePosition; i++) {
					offset += (*((uinttype*) (ptr + offset))) + sizeof(uinttype);
				}

				ushorttype length = *((uinttype*) (ptr + offset));
				bytearray data = (bytearray) (ptr + offset + sizeof(uinttype));

				#if COM_DEEPIS_DB_CHARSET
				length = DeepStore::getCharset()->populate(getCharacterSet(this), keyBuffer, data, length, m_length);
				#else
				length = (length < m_length) ? length : m_length;
				memcpy(keyBuffer, data, length);
				#endif

				padMultiByteText(m_length, length, keyBuffer);
			}

			return m_unpackLength;
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long");

			throw UnsupportedOperationException("Key part cannot be converted to long");
		}

		FORCE_INLINE ulongtype maxKey() const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long in maxKey");

			throw UnsupportedOperationException("Key part cannot be converted to long in maxKey");
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (maybeNull() && key[0]) {
				return nullToString(buffer);

			} else {
				memcpy(buffer, key + m_nullBytes, m_length);

				return m_length;
			}
		}
};

class DeepKeyPartVarBinary : public DeepKeyPart {
	public:
		DeepKeyPartVarBinary(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_nullBytes + m_lengthBytes + ((maybeNull() && key[0]) ? 0 : *((ushorttype*) (key + m_nullBytes)));
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			if (maybeNull()) {
				if (key1[0]) {
					if (key2[0]) {
						if (nullKey != null) {
							*nullKey = true;
						}
						return 0;

					} else {
						return -1;
					}

				} else if (key2[0]) {
					return 1;

				} else {
					ushorttype len1 = *((ushorttype*) (key1 + 1));
					ushorttype len2 = *((ushorttype*) (key2 + 1));

					int cmp = memcmp(key1 + 1 + sizeof(ushorttype), key2 + 1 + sizeof(ushorttype), min(len1, len2));

					if ((cmp == 0) && (len1 != len2)) {
						cmp = (len1 > len2) ? 1 : -1;
					}

					return cmp;
				}

			} else {
				ushorttype len1 = *((ushorttype*) key1);
				ushorttype len2 = *((ushorttype*) key2);

				int cmp = memcmp(key1 + sizeof(ushorttype), key2 + sizeof(ushorttype), min(len1, len2));

				if ((cmp == 0) && (len1 != len2)) {
					cmp = (len1 > len2) ? 1 : -1;
				}

				return cmp;
			}
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			if (maybeNull()) {
				uchartype nullKey = isNull(value);

				memcpy(keyBuffer, &nullKey, 1);

				if (nullKey == 0) {
					// (value + m_valueOffset) == start of variable length data (i.e. blob data)
					const bytearray ptr = value + m_valueOffset;

					uinttype offset = 0;
					for (shorttype i=0; i<m_variablePosition; i++) {
						offset += (*((uinttype*) (ptr + offset))) + sizeof(uinttype);
					}

					ushorttype length = *((uinttype*) (ptr + offset));
					length = (length < m_length) ? length : m_length;

					bytearray data = (bytearray) (ptr + offset + sizeof(uinttype));

					memcpy(keyBuffer + 1, &length, sizeof(ushorttype));
					memcpy(keyBuffer + 1 + sizeof(ushorttype), data, length);
				}

			} else {
				// (value + m_valueOffset) == start of variable length data (i.e. blob data)
				const bytearray ptr = value + m_valueOffset;

				uinttype offset = 0;
				for (shorttype i=0; i<m_variablePosition; i++) {
					offset += (*((uinttype*) (ptr + offset))) + sizeof(uinttype);
				}

				ushorttype length = *((uinttype*) (ptr + offset));
				length = (length < m_length) ? length : m_length;

				bytearray data = (bytearray) (ptr + offset + sizeof(uinttype));

				memcpy(keyBuffer, &length, sizeof(ushorttype));
				memcpy(keyBuffer + sizeof(ushorttype), data, length);
			}

			return DeepKeyPartVarBinary::getPackLength(keyBuffer);
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long");

			throw UnsupportedOperationException("Key part cannot be converted to long");
		}

		FORCE_INLINE ulongtype maxKey() const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long in maxKey");

			throw UnsupportedOperationException("Key part cannot be converted to long in maxKey");
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (maybeNull() && key[0]) {
				return nullToString(buffer);

			} else {
				ushorttype strlen = *((ushorttype*) (key + m_nullBytes));

				memcpy(buffer, key + m_nullBytes + m_lengthBytes, strlen);

				return strlen;
			}
		}
};

class DeepKeyPartFixedVarBinary : public DeepKeyPart {
	public:
		DeepKeyPartFixedVarBinary(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			if (maybeNull()) {
				if (key1[0]) {
					if (key2[0]) {
						if (nullKey != null) {
							*nullKey = true;
						}
						return 0;

					} else {
						return -1;
					}

				} else if (key2[0]) {
					return 1;

				} else {
					ushorttype len1 = *((ushorttype*) (key1 + 1));
					ushorttype len2 = *((ushorttype*) (key2 + 1));

					int cmp = memcmp(key1 + 1 + sizeof(ushorttype), key2 + 1 + sizeof(ushorttype), min(len1, len2));

					if ((cmp == 0) && (len1 != len2)) {
						cmp = (len1 > len2) ? 1 : -1;
					}

					return cmp;
				}

			} else {
				ushorttype len1 = *((ushorttype*) key1);
				ushorttype len2 = *((ushorttype*) key2);

				int cmp = memcmp(key1 + sizeof(ushorttype), key2 + sizeof(ushorttype), min(len1, len2));

				if ((cmp == 0) && (len1 != len2)) {
					cmp = (len1 > len2) ? 1 : -1;
				}

				return cmp;
			}
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			// XXX: needed here for 'fixed' length var types (see KeyBuilder->isEqual())
			memset(keyBuffer, 0, getUnpackLength());

			if (maybeNull()) {
				uchartype nullKey = isNull(value);

				memcpy(keyBuffer, &nullKey, 1);

				if (nullKey == 0) {
					ushorttype length = *((uchartype*) (value + m_valueOffset));

					memcpy((keyBuffer + 1), &length, sizeof(ushorttype));
					memcpy((keyBuffer + 1 + sizeof(ushorttype)), (value + m_valueOffset + sizeof(uchartype)), length);
				}

			} else {
				ushorttype length = *((uchartype*) (value + m_valueOffset));

				memcpy((keyBuffer), &length, sizeof(ushorttype));
				memcpy((keyBuffer + sizeof(ushorttype)), (value + m_valueOffset + sizeof(uchartype)), length);
			}

			return m_unpackLength;
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long");

			throw UnsupportedOperationException("Key part cannot be converted to long");
		}

		FORCE_INLINE ulongtype maxKey() const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long in maxKey");

			throw UnsupportedOperationException("Key part cannot be converted to long in maxKey");
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (maybeNull() && key[0]) {
				return nullToString(buffer);

			} else {
				ushorttype strlen = *((ushorttype*) (key + m_nullBytes));

				memcpy(buffer, key + m_nullBytes + m_lengthBytes, strlen);

				return strlen;
			}
		}
};

class DeepKeyPartBinary : public DeepKeyPart {
	public:
		DeepKeyPartBinary(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			if (maybeNull()) {
				if (key1[0]) {
					if (key2[0]) {
						if (nullKey != null) {
							*nullKey = true;
						}
						return 0;

					} else {
						return -1;
					}

				} else if (key2[0]) {
					return 1;

				} else {
					return memcmp(key1 + 1, key2 + 1, m_length);
				}

			} else {
				return memcmp(key1, key2, m_length);
			}
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			if (maybeNull()) {
				uchartype nullKey = isNull(value);

				memcpy(keyBuffer, &nullKey, 1);

				if (nullKey == 0) {
					memcpy((keyBuffer + 1), (value + m_valueOffset), m_length);
				}

			} else {
				memcpy(keyBuffer, (value + m_valueOffset), m_length);
			}

			return m_unpackLength;
		}

		FORCE_INLINE ulongtype toULong(const bytearray key) const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long");

			throw UnsupportedOperationException("Key part cannot be converted to long");
		}

		FORCE_INLINE ulongtype maxKey() const {
			DEEP_LOG(ERROR, OTHER, "Key part cannot be converted to long in maxKey");

			throw UnsupportedOperationException("Key part cannot be converted to long in maxKey");
		}

		FORCE_INLINE inttype toString(const bytearray key, bytearray buffer) const {
			if (maybeNull() && key[0]) {
				return nullToString(buffer);

			} else {
				memcpy(buffer, "BINARY", 6);

				return 6;
			}
		}
};

/* Used when adding a hidden key to a secondary to make it unique */
class DeepKeyPartHidden : public DeepKeyPartNumber<DeepULongLong> {
	public:
		DeepKeyPartHidden(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) :
			DeepKeyPartNumber<DeepULongLong>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition) {
		}

		FORCE_INLINE ushorttype getPackLength(const bytearray key) const {
			return m_unpackLength;
		}

		FORCE_INLINE inttype compareKey(const bytearray key1, const bytearray key2, boolean* nullKey) {
			return compareNumberByteArray<DeepULongLong>(key1, key2);
		}

		FORCE_INLINE ushorttype populateKey(const bytearray pkey, const bytearray value, bytearray keyBuffer) {
			if (pkey != null) {
				memcpy(keyBuffer, pkey, m_length);

				return m_length;

			} else {
				throw UnsupportedOperationException("Primary key is null during hidden key populate");
			}
		}

		FORCE_INLINE ulongtype toLong(const bytearray key) const {
			return numberToULong<DeepULongLong>(key + m_nullBytes);
		}

		FORCE_INLINE ulongtype maxKey() const {
			return (ulongtype) ::maxValue<DeepULongLong>();
		}

		inttype toString(const bytearray key, bytearray buffer) const {
			return DeepKeyPartNumber<DeepULongLong>::toString(key, buffer);
		}
};

DeepKeyPart* createDeepKeyPart(uinttype fieldIndex, uchartype type, ushorttype length, uinttype valueOffset, uinttype nullOffset, uchartype nullBit, boolean isIgnored, boolean isReserved, shorttype variablePosition, shorttype primaryPosition) {
	switch(type) {
		case CT_DATASTORE_LONG_INT:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepLongInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepLongInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		case CT_DATASTORE_ULONG_INT:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepULongInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepULongInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		case CT_DATASTORE_LONGLONG:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepLongLong>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepLongLong>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		case CT_DATASTORE_ULONGLONG:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepULongLong>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepULongLong>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		case CT_DATASTORE_SHORT_INT:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepShortInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepShortInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		case CT_DATASTORE_USHORT_INT:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepUShortInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepUShortInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		// TODO: separate null versions

		case CT_DATASTORE_INT24:
			return new DeepKeyPartMediumInt(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

		case CT_DATASTORE_UINT24:
			return new DeepKeyPartUMediumInt(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

		case CT_DATASTORE_INT8:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepTinyInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepTinyInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		case CT_DATASTORE_UINT8:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepUTinyInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepUTinyInt>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		case CT_DATASTORE_FLOAT:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepFloat>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepFloat>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		case CT_DATASTORE_DOUBLE:
			if (nullBit == 0) {
				return new DeepKeyPartNumber<DeepDouble>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

			} else {
				return new DeepKeyPartNumberNull<DeepDouble>(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);
			}

		// TODO: separate null versions

		case CT_DATASTORE_VARTEXT1:
		case CT_DATASTORE_VARTEXT2:
			return new DeepKeyPartVarChar(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition);

		case CT_DATASTORE_FIXED_VARTEXT1:
			return new DeepKeyPartFixedVarChar(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, 0, primaryPosition);

		case CT_DATASTORE_TEXT:
			return new DeepKeyPartChar(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

		case CT_DATASTORE_TEXT_MULTIBYTE:
			return new DeepKeyPartCharMultiByte(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition);

		case CT_DATASTORE_BINARY:
			return new DeepKeyPartBinary(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

		case CT_DATASTORE_VARBINARY1:
		case CT_DATASTORE_VARBINARY2:
			return new DeepKeyPartVarBinary(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition);

		case CT_DATASTORE_FIXED_VARBINARY1:
			return new DeepKeyPartFixedVarBinary(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, 0, primaryPosition);

		case CT_DATASTORE_HIDDEN:
			return new DeepKeyPartHidden(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, -1, primaryPosition);

		default:
			DEEP_LOG(ERROR, OTHER, "(Invalid key part type): create key part type %d)\n", type);
			throw UnsupportedOperationException("Invalid key part type");
	}
}

#endif //COM_DEEPIS_DATASTORE_API_DEEP_TYPES_H_

