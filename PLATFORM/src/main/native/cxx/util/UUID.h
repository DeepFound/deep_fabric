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
#ifndef CXX_UTIL_UUID_H_
#define CXX_UTIL_UUID_H_

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <sstream>
#include <iomanip>

#include "cxx/lang/Exception.h"
#include "cxx/lang/Comparable.h"
#include "cxx/lang/Object.h"
#include "cxx/lang/String.h"

namespace cxx { namespace util {
class UUIDUnsupportedOperationException : public Exception {
public:
	UUIDUnsupportedOperationException(void) {
	}

	UUIDUnsupportedOperationException(const char* message)
		: Exception(message) {
	}
};

/**
 * A class that represents an immutable universally unique identifier
 * (UUID). A UUID represents a 128-bit value.
 *
 * There exist different variants of these global identifiers. The
 * methods of this class are for manipulating the Leach-Salz variant, although
 * the constructors allow the creation of any variant of UUID (described below).
 *
 * The layout of a variant 2 (Leach-Salz) UUID is as follows:
 * The most significant long consists of the following unsigned fields:
 *
 * 0xFFFFFFFF00000000 time_low
 * 0x00000000FFFF0000 time_mid
 * 0x000000000000F000 version
 * 0x0000000000000FFF time_hi
 *
 * The least significant long consists of the following unsigned fields:
 *
 * 0xC000000000000000 variant
 * 0x3FFF000000000000 clock_seq
 * 0x0000FFFFFFFFFFFF node
 *
 * The variant field contains a value which identifies the layout of the
 * UUID. The bit layout described above is valid only for a UUID with
 * a variant value of 2, which indicates the Leach-Salz variant.
 *
 * The version field holds a value that describes the type of this UUID.
 * There are four different basic types of UUIDs: time-based, DCE
 * security, name-based, and randomly generated UUIDs.
 * These types have a version value of 1, 2, 3 and 4, respectively.
 *
 * For more information including algorithms used to create UUIDs, see
 * RFC 4122: A Universally Unique IDentifier (UUID) URN Namespace,
 * section 4.2 "Algorithms for Creating a Time-Based UUID".
 *
 * The UUID string representation is as described by this BNF:
 * UUID                   = <time_low> "-" <time_mid> "-"
 *                          <time_high_and_version> "-"
 *                          <variant_and_sequence> "-"
 *                          <node>
 * time_low               = 4*<hexOctet>
 * time_mid               = 2*<hexOctet>
 * time_high_and_version  = 2*<hexOctet>
 * variant_and_sequence   = 2*<hexOctet>
 * node                   = 6*<hexOctet>
 * hexOctet               = <hexDigit><hexDigit>
 * hexDigit               =
 *       "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
 *       | "a" | "b" | "c" | "d" | "e" | "f"
 *       | "A" | "B" | "C" | "D" | "E" | "F"
 *
 *  0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          time_low                             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       time_mid                |         time_hi_and_version   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |clk_seq_hi_res |  clk_seq_low  |         node (0-1)            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         node (2-5)                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */
class UUID : public cxx::lang::Comparable {
public:
	UUID(longtype mostSigBits, longtype leastSigBits);
	virtual ~UUID();

	inttype           clockSequence();
	virtual int       compareTo(const Object* obj) const;
	boolean           equals(const Object* obj) const;
	static UUID*      fromString(cxx::lang::String& name);
	longtype          getLeastSignificantBits();
	longtype          getMostSignificantBits();
	virtual long      hashCode(void) const;
	static UUID*      nameUUIDFromBytes(bytearray name);
	longtype          node();
	static UUID*      randomUUID();
	longtype          timestamp();
	cxx::lang::String toString();
	inttype           variant();
	inttype           version();

private:
	UUID(bytearray data);
	cxx::lang::String digits(longtype iVal, inttype iDigits);

	longtype m_mostSigBits;
	longtype m_leastSigBits;
	inttype  m_version;
	inttype  m_variant;
	longtype m_timestamp;
	inttype  m_sequence;
	longtype m_node;

}; /** UUID */

} /** util */

} /** cxx */

/**
 * UUID
 * Constructs a new UUID using the specified data.
 * mostSigBits is used for the most significant 64 bits of the UUID
 * and leastSigBits becomes the least significant 64 bits of the UUID.
 * @param mostSigBits - The most significant bits of the UUID
 * @param leastSigBits - The least significant bits of the UUID
 */
FORCE_INLINE cxx::util::UUID::UUID(longtype mostSigBits, longtype leastSigBits)
	: m_mostSigBits(mostSigBits),
	  m_leastSigBits(leastSigBits),
	  m_version(-1),
	  m_variant(-1),
	  m_timestamp(-1),
	  m_sequence(-1),
	  m_node(-1) {
}

FORCE_INLINE cxx::util::UUID::UUID(bytearray data)
	: m_mostSigBits(0),
	  m_leastSigBits(0),
	  m_version(-1),
	  m_variant(-1),
	  m_timestamp(-1),
	  m_sequence(-1),
	  m_node(-1) {
	for (inttype i=0; i<8; i++) {
		m_mostSigBits = (m_mostSigBits << 8) | (data[i] & 0xff);
	}

	for (int i=8; i<16; i++) {
		m_leastSigBits = (m_leastSigBits << 8) | (data[i] & 0xff);
	}

	srand(time(NULL));
}

FORCE_INLINE cxx::util::UUID::~UUID() {
	m_mostSigBits  = -1;
	m_leastSigBits = -1;
	m_version      = -1;
	m_variant      = -1;
	m_timestamp    = -1;
	m_sequence     = -1;
	m_node         = -1;
}

/**
 *
 * The 14 bit clock sequence value is constructed from the clock
 * sequence field of this UUID. The clock sequence field is used
 * to guarantee temporal uniqueness in a time-based UUID.
 * The clockSequence value is only meaningful in a time-based UUID,
 * which has version type 1.
 * @return The clock sequence value associated with this UUID.
 */
FORCE_INLINE inttype cxx::util::UUID::clockSequence() {
	if (version() != 1) {
		throw UUIDUnsupportedOperationException("Not a time-based UUID");
	}

	if (m_sequence < 0) {
		m_sequence = (inttype)(((m_leastSigBits & 0x3FFF000000000000L) >> 48) & 0x3FFF);
	}

	return m_sequence;
}

/**
 *
 * Compares this UUID with the specified UUID.
 * The first of two UUIDs is greater than the second if the
 * most significant field in which the UUIDs differ is
 * greater for the first UUID.
 * @param obj UUID to which this UUID is to be compared.
 * @return -1, 0 or 1 as this UUID is less than, equal to, or greater than obj.
 */
FORCE_INLINE int cxx::util::UUID::compareTo(const Object* obj) const {
	const UUID* pcOther = static_cast<const UUID* >(obj);

	return (this->m_mostSigBits < pcOther->m_mostSigBits ? -1 :
		(this->m_mostSigBits > pcOther->m_mostSigBits ? 1 :
		 (this->m_leastSigBits < pcOther->m_leastSigBits ? -1 :
		  (this->m_leastSigBits > pcOther-> m_leastSigBits ? 1 :
		   0))));
}

/**
 * equals
 * Compares this object to the specified object. The result is
 * true if and only if the argument is not null, is a UUID object,
 * has the same variant, and contains the same value, bit for bit,
 * as this UUID.
 * @param obj - The object to be compared.
 * @return true if the objects are the same; false otherwise.
 */
FORCE_INLINE boolean cxx::util::UUID::equals(const Object* obj) const {
	const UUID* pcOther = static_cast<const UUID* >(obj);

	if (this == pcOther) {
		return true;
	}

	if (pcOther->m_variant != m_variant) {
		return false;
	}

        return ((m_mostSigBits == pcOther->m_mostSigBits) &&
                (m_leastSigBits == pcOther->m_leastSigBits));
}

/**
 * fromString
 * Creates a UUID from the string standard representation as
 * described in the toString() method.
 * @param name - A string that specifies a UUID
 * @return A UUID with the specified value.
 */
FORCE_INLINE cxx::util::UUID* cxx::util::UUID::fromString(cxx::lang::String& name) {
	size_t  i      = 0;
	size_t  pos    = name.find('-');
	inttype iCount = 0;
	cxx::lang::String components[5];

	while (pos != std::string::npos) {
		components[iCount] = name.substr(i, pos-i);
		iCount++;

		i = ++pos;
		pos = name.find('-', pos);

		if (5 <= iCount) {
			throw UUIDUnsupportedOperationException("Invalid UUID format");
		}

		if (pos == string::npos) {
			components[iCount] = name.substr(i, name.length());
		}
	}

	if (4 != iCount)  {
		throw UUIDUnsupportedOperationException("Invalid UUID format");
	}

	longtype mostSigBits = strtoll(components[0], 0, 16);
	mostSigBits <<= 16;
	mostSigBits |= strtoll(components[1], 0, 16);
	mostSigBits <<= 16;
	mostSigBits |= strtoll(components[2], 0, 16);

	longtype leastSigBits = strtoll(components[3], 0, 16);
	leastSigBits <<= 48;

	leastSigBits |= strtoll(components[4], 0, 16);

	return new UUID(mostSigBits, leastSigBits);
}

/**
 * getLeastSignificantBits
 * Returns the least significant 64 bits of this UUID's 128 bit value.
 * @return The least significant 64 bits of this UUID's 128 bit value.
 */
FORCE_INLINE longtype cxx::util::UUID::getLeastSignificantBits() {
	return m_leastSigBits;
}

/**
 * getMostSignificantBits
 * Returns the most significant 64 bits of this UUID's 128 bit value.
 * @return The most significant 64 bits of this UUID's 128 bit value.
 */
FORCE_INLINE longtype cxx::util::UUID::getMostSignificantBits() {
	return m_mostSigBits;
}

/**
 * hashCode
 * Returns a hash code for this UUID.
 * @return A hash code value for this UUID.
 */
FORCE_INLINE long cxx::util::UUID::hashCode(void) const {
	long hashCode = (long)((m_mostSigBits >> 32) ^
			       m_mostSigBits ^
			       (m_leastSigBits >> 32) ^
			       m_leastSigBits);
	return hashCode;
}

/**
 * nameUUIDFromBytes
 * Static factory to retrieve a type 3 (name based) UUID based
 * on the specified byte array.
 * @param name - A byte array to be used to construct a UUID
 * @return A UUID generated from the specified array.
 */
FORCE_INLINE cxx::util::UUID* cxx::util::UUID::nameUUIDFromBytes(bytearray name) {
	throw UUIDUnsupportedOperationException("nameUUIDFromBytes not yet supported");
	return new UUID(0);
}

/**
 *
 * The 48 bit node value is constructed from the node field of this
 * UUID. This field is intended to hold the IEEE 802 address of the
 * machine that generated this UUID to guarantee spatial uniqueness.
 * The node value is only meaningful in a time-based UUID,
 * which has version type 1.
 * @return The node value of this UUID.
 */
FORCE_INLINE longtype cxx::util::UUID::node() {
	if (version() != 1) {
		throw new UUIDUnsupportedOperationException("Not a time-based UUID");
	}

	if (m_node < 0) {
		m_node = m_leastSigBits & 0x0000FFFFFFFFFFFFL;
	}

	return m_node;
}

/**
 * randomUUID
 * Static factory to retrieve a type 4 (pseudo randomly generated)
 * UUID. The UUID is generated using a cryptographically strong
 * pseudo random number generator.
 * @return A randomly generated UUID
 */
FORCE_INLINE cxx::util::UUID* cxx::util::UUID::randomUUID() {
	bytetype randomBytes[16] = { 0 };

	for (inttype i = 0; i < (inttype)sizeof(randomBytes); i++) {
		randomBytes[i] = (bytetype)(rand() % 256);
	}

	randomBytes[6]  &= 0x0f;  /* clear version        */
	randomBytes[6]  |= 0x40;  /* set to version 4     */
	randomBytes[8]  &= 0x3f;  /* clear variant        */
	randomBytes[8]  |= 0x80;  /* set to IETF variant  */

	return new UUID(randomBytes);
}

/**
 * timestamp
 * The 60 bit timestamp value is constructed from the time_low,
 * time_mid, and time_hi fields of this UUID. The resulting timestamp
 * is measured in 100-nanosecond units since midnight, October 15, 1582 UTC.
 * The timestamp value is only meaningful in a time-based UUID, which
 * has version type 1.
 * return The timestamp value associated with this UUID.
 */
FORCE_INLINE longtype cxx::util::UUID::timestamp() {
	if (version() != 1) {
		throw new UUIDUnsupportedOperationException("Not a time-based UUID");
	}

	longtype result = m_timestamp;
	if (result == -1) {
		result = (m_mostSigBits & 0x0000000000000FFFL) << 48;
		result |= ((m_mostSigBits >> 16) & 0xFFFFL) << 32;
		result |= ((m_mostSigBits >> 32) & 0xFFFFFFFFL);
		m_timestamp = result;
	}

	return result;
}

/**
 * toString
 * Returns a String object representing this UUID.
 * @return A string representation of this UUID.
 */
FORCE_INLINE cxx::lang::String cxx::util::UUID::toString() {
	return (digits(m_mostSigBits >> 32, 8) + '-' +
		digits(m_mostSigBits >> 16, 4) + '-' +
		digits(m_mostSigBits,  4) + '-' +
		digits(m_leastSigBits >> 48, 4) + '-' +
		digits(m_leastSigBits, 12));
}

FORCE_INLINE cxx::lang::String cxx::util::UUID::digits(longtype iVal, inttype iDigits) {
	std::stringstream ss;

	longtype hi    = 1L << (iDigits * 4);
	longtype width = (iVal & (hi - 1));

	ss << std::hex << setfill('0') << std::setw(iDigits) << width;

	return ss.str();
}

/**
 *
 * The variant number associated with this UUID.
 * The variant number describes the layout of the UUID.
 * The variant number has the following meaning:
 *   0 Reserved for NCS backward compatibility.
 *   2 IETF RFC 4122 (Leach-Salz).
 *   6 Reserved, Microsoft Corporation backward compatibility.
 *   7 Reserved for future definition.
 * @return The variant number of this UUID.
 */
FORCE_INLINE inttype cxx::util::UUID::variant() {
	if (m_variant < 0) {
		// This field is composed of a varying number of bits
		if (((m_leastSigBits >> 63) & 0x1) == 0) {
			m_variant = 0;
		} else if (((m_leastSigBits >> 62) & 0x3) == 2) {
			m_variant = 2;
		} else {
			m_variant = (inttype)((m_leastSigBits >> 61) & 0x7);
		}
	}

	return m_variant;
}

/**
 *
 * The version number associated with this UUID. The version number
 * describes how this UUID was generated.
 * The version number has the following meaning:
 *  1 Time-based UUID.
 *  2 DCE security UUID.
 *  3 Name-based UUID.
 *  4 Randomly generated UUID.
 * @return The version number of this UUID
 */
FORCE_INLINE inttype cxx::util::UUID::version() {
	if (m_version < 0) {
		// Version is bits masked by 0x000000000000F000 in MS long
		m_version = (inttype)((m_mostSigBits >> 12) & 0x0f);
	}

	return m_version;
}


#endif /** CXX_UTIL_UUID_H_ */
