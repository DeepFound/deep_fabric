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
#ifndef CXX_LANG_IO_BINARYWIREFORMAT_H_
#define CXX_LANG_IO_BINARYWIREFORMAT_H_

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <endian.h>
#include <string.h>
#include <string>

#include "cxx/lang/types.h"
#include "cxx/lang/String.h"

namespace cxx { namespace io { namespace binaryWireFormat {

static const ubytetype  MSB          =  0x80;
static const ubytetype  LSB          =  0x7f;
static const ubytetype  MSBALL       = ~LSB;
static const ubytetype  BYTE_MASK    = 0xff;
static const inttype    BITINC       = 7;

static const uinttype   INT32_SIZE   = 4;
static const uinttype   UINT32_SIZE  = 4;
static const uinttype   FIXED32_SIZE = 4;
static const uinttype   FLOAT_SIZE   = FIXED32_SIZE;

static const uinttype   INT64_SIZE   = 8;
static const uinttype   UINT64_SIZE  = 8;
static const uinttype   FIXED64_SIZE = 8;
static const uinttype   DOUBLE_SIZE  = FIXED64_SIZE;

/**
 * ReadVarIntFromBuffer<INT>
 * Reads the varint encoded integer from the buffer.
 * Assumes the integer can not be bigger than 64bit. The
 * number of bytes written is returned in 'iWrote',
 * @param piBuffer - The buffer to read from. [in]
 * @param iLen     - The length of the buffer. [in]
 * @param iRead    - The number of bytes read. [out]
 * @param iValue   - The INTeger read is returned [out].
 * @return 0 on success, else -1.
 */
template <typename INT>
FORCE_INLINE inttype ReadVarIntFromBuffer(const ubytetype* piBuffer,
					  const uinttype   iLen,
					  uinttype&        iRead,
					  INT&             iValue) {
	INT              iResult = 0;
	INT              iSubRes = 0;
	inttype          iBits   = 0;
	const ubytetype* piPos   = piBuffer;

	while (*piPos & MSB) {

		if ((piPos - piBuffer) > iLen) {
			return -1;
		}

		iSubRes  = *piPos;
		iResult += ((iSubRes & LSB) << iBits);
		iBits   += BITINC;
		piPos++;
	}

	iSubRes  = *piPos;
	iResult += ((iSubRes & LSB) << iBits);

	iRead    = (uinttype)(piPos - piBuffer + 1);

	iValue   = iResult;

	return 0;
}

/**
 * WriteVarIntToBuffer<INT>
 * Writes the integer as an encoded varint to the buffer.
 * Assumes the integer can not be bigger than 64bit. The
 * number of bytes written is returned in 'iWrote',
 * @param piBuffer - The buffer to write into. [in]
 * @param iLen     - The length of the buffer. [in]
 * @param iWrote   - The number of bytes written. [out]
 * @param iValue   - The INTeger to write. [in]
 * @return 0 on success, else -1.
 */
template <typename INT>
FORCE_INLINE inttype WriteVarIntToBuffer(ubytetype*     piBuffer,
					 const uinttype iLen,
					 uinttype&      iWrote,
					 INT            iValue) {
	inttype iSize = 0;

	while (iValue > LSB) {
		piBuffer[iSize] = (static_cast<ubytetype>(iValue) & LSB) | MSB;
		iValue >>= BITINC;
		iSize++;
	}

	piBuffer[iSize] = static_cast<ubytetype>(iValue) & LSB;
	iSize++;

	iWrote = iSize;

	return 0;
}

/**
 * VarIntLength<INT>
 * Calculates the length number of bytes to encode the integer.
 * Assumes the integer can not be any longer than 64bit.
 * @param iValue - the value to calculate length for.
 * @return the number of bytes.
 */
template <typename INT>
FORCE_INLINE uinttype VarIntLength(INT iValue) {
	if (iValue < (1ull << 35)) {
		if (iValue < (1ull << 7)) {
			return 1;
		} else if (iValue < (1ull << 14)) {
			return 2;
		} else if (iValue < (1ull << 21)) {
			return 3;
		} else if (iValue < (1ull << 28)) {
			return 4;
		} else {
			return 5;
		}
	} else {
		if (iValue < (1ull << 42)) {
			return 6;
		} else if (iValue < (1ull << 49)) {
			return 7;
		} else if (iValue < (1ull << 56)) {
			return 8;
		} else if (iValue < (1ull << 63)) {
			return 9;
		} else {
			return 10;
		}
	}
}

/**
 * ZigZag encoding maps signed integers to unsigned integers so that
 * numbers with a small absolute value (for instance, -1) have a small
 * varint encoded value too. It does this in a way that "zig-zags"
 * back and forth through the positive and negative integers, so that
 * -1 is encoded as 1, 1 is encoded as 2, -2 is encoded as 3, and so on.
 */
static const uinttype ZIGZAG32 = 31;
static const uinttype ZIGZAG64 = 63;

template <typename INT, typename OINT, uinttype BITS>
FORCE_INLINE OINT encodeZigZag(INT iValue) {
	/** BITS={32-1, 64-1} */
	return static_cast<OINT>((iValue << 1) ^ (iValue >> BITS));
}

template <typename INT, typename OINT>
FORCE_INLINE OINT decodeZigZag(INT iValue) {
	return  static_cast<OINT>((iValue >> 1) ^ (-(iValue & 1)));
}

/** There is no point doing this for unsigned integers. */
FORCE_INLINE uinttype encodeZigZag32(inttype iValue) {
	return encodeZigZag<inttype, uinttype, ZIGZAG32>(iValue);
}

FORCE_INLINE ulongtype encodeZigZag64(longtype iValue) {
	return encodeZigZag<longtype, ulongtype, ZIGZAG64>(iValue);
}

FORCE_INLINE inttype decodeZigZag32(uinttype iValue) {
	return decodeZigZag<uinttype, inttype>(iValue);
}

FORCE_INLINE longtype decodeZigZag64(ulongtype iValue) {
	return decodeZigZag<ulongtype, longtype>(iValue);
}

/** Double and Float conversion to integers for wireencoding. */
template <typename F, typename I>
FORCE_INLINE I convertFloatToInt(F f) {
	union {
		F f;
		I i;
	} u;

	u.f = f;
	return u.i;
}

template <typename F, typename I>
FORCE_INLINE F convertIntToFloat(I i) {
	union {
		F f;
		I i;
	} u;

	u.i = i;
	return u.f;
}

FORCE_INLINE inttype convertFloat32ToInt32(float f) {
	return convertFloatToInt<float, inttype>(f);
}

FORCE_INLINE longtype convertFloat64ToInt64(double f) {
	return convertFloatToInt<double, longtype>(f);
}

FORCE_INLINE float convertInt32ToFloat32(inttype i) {
	return convertIntToFloat<float, inttype>(i);
}

FORCE_INLINE double convertInt64ToFloat64(longtype i) {
	return convertIntToFloat<double, longtype>(i);
}

/**
 * ReadFixed32<T>
 * Read in a fixed sized 32bit value from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param value [out] - location to place the value.
 * @return number of bytes read.
 */
template <typename T>
FORCE_INLINE uinttype ReadFixed32(const ubytetype* piBuffer,
				  const uinttype   iLen,
				  T&               value) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	memcpy(&value, piBuffer, sizeof(value));
#else
	value =
		(static_cast<uint32>(piBuffer[0])      ) |
		(static_cast<uint32>(piBuffer[1]) <<  8) |
		(static_cast<uint32>(piBuffer[2]) << 16) |
		(static_cast<uint32>(piBuffer[3]) << 24);
#endif
	return FIXED32_SIZE;
}

/**
 * WriteFixed32<T>
 * Write a fixed sized 32bit value to the buffer provided.
 * @param piBuffer - the location to write
 * @param iLen - the remaining buffer length
 * @param value [in] - the value to write
 * @return number of bytes written.
 */
template <typename T>
FORCE_INLINE uinttype WriteFixed32(ubytetype*     piBuffer,
				   const uinttype iLen,
				   T              value) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	memcpy(piBuffer, &value, sizeof(value));
#else
	piBuffer[0] = static_cast<ubytetype>(value      );
	piBuffer[1] = static_cast<ubytetype>(value >>  8);
	piBuffer[2] = static_cast<ubytetype>(value >> 16);
	piBuffer[3] = static_cast<ubytetype>(value >> 24);
#endif
	return FIXED32_SIZE;
}

/**
 * ReadFixed64<T>
 * Read in a fixed sized 64bit value from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param value [out] - location to place the value.
 * @return number of bytes read.
 */
template <typename T>
FORCE_INLINE uinttype ReadFixed64(const ubytetype* piBuffer,
				  const uinttype iLen,
				  T&             value) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	memcpy(&value, piBuffer, sizeof(value));
#else
	uinttype part0 =
		(static_cast<uint32>(piBuffer[0])      ) |
		(static_cast<uint32>(piBuffer[1]) <<  8) |
		(static_cast<uint32>(piBuffer[2]) << 16) |
		(static_cast<uint32>(piBuffer[3]) << 24);
	uinttype part1 =
		(static_cast<uint32>(piBuffer[4])      ) |
		(static_cast<uint32>(piBuffer[5]) <<  8) |
		(static_cast<uint32>(piBuffer[6]) << 16) |
		(static_cast<uint32>(piBuffer[7]) << 24);
	value =
		(static_cast<uint64>(part0)      ) |
		(static_cast<uint64>(part1) << 32);
#endif
	return FIXED64_SIZE;
}

/**
 * WriteFixed64<T>
 * Write a fixed sized 64bit value to the buffer provided.
 * @param piBuffer - the location to write
 * @param iLen - the remaining buffer length
 * @param value [in] - the value to write
 * @return number of bytes written.
 */
template <typename T>
FORCE_INLINE uinttype WriteFixed64(ubytetype*     piBuffer,
				   const uinttype iLen,
				   T              value) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	memcpy(piBuffer, &value, sizeof(value));
#else
	inttype part0 = static_cast<inttype>(value      );
	inttype part1 = static_cast<inttype>(value >> 32);

	piBuffer[0] = static_cast<ubytetype>(part0      );
	piBuffer[1] = static_cast<ubytetype>(part0 >>  8);
	piBuffer[2] = static_cast<ubytetype>(part0 >> 16);
	piBuffer[3] = static_cast<ubytetype>(part0 >> 24);
	piBuffer[4] = static_cast<ubytetype>(part1      );
	piBuffer[5] = static_cast<ubytetype>(part1 >>  8);
	piBuffer[6] = static_cast<ubytetype>(part1 >> 16);
	piBuffer[7] = static_cast<ubytetype>(part1 >> 24);
#endif
	return FIXED64_SIZE;
}

/**
 * WriteInt32ToBuffer
 * Write a variable sized Int32 to the buffer provided.
 * @param piBuffer - the location to write
 * @param iLen - the remaining buffer length
 * @param value [in] - the value to write
 * @return number of bytes written.
 */
FORCE_INLINE uinttype WriteInt32ToBuffer(ubytetype*     piBuffer,
					 const uinttype iLen,
					 const inttype  iValue) {
	uinttype iWrite  = encodeZigZag32(iValue);
	uinttype iBytes  = 0;
	inttype  iResult = WriteVarIntToBuffer<uinttype>(piBuffer,
							 iLen,
							 iBytes,
							 iWrite);
	if (0 != iResult) {
		return 0;
	}

	return iBytes;
}

/**
 * SizeofInt32
 * Returns the expected encoding size for a Int32.
 * @param iValue - the value to get an encoding size for.
 * @return number of bytes the value encodes into.
 */
FORCE_INLINE uinttype SizeofInt32(const inttype iValue) {
	uinttype iWrite  = encodeZigZag32(iValue);
	return VarIntLength<uinttype>(iWrite);
}

/**
 * WriteUint32ToBuffer
 * Write a variable sized UINT32 to the buffer provided.
 * @param piBuffer - the location to write
 * @param iLen - the remaining buffer length
 * @param value [in] - the value to write
 * @return number of bytes written.
 */
FORCE_INLINE uinttype WriteUint32ToBuffer(ubytetype*     piBuffer,
					  const uinttype iLen,
					  const uinttype iValue) {
	uinttype iBytes  = 0;
	inttype  iResult = WriteVarIntToBuffer<uinttype>(piBuffer,
							 iLen,
							 iBytes,
							 iValue);
	if (0 != iResult) {
		return 0;
	}

	return iBytes;
}

/**
 * SizeofUint32
 * Returns the expected encoding size for a UINT32.
 * @param iValue - the value to get an encoding size for.
 * @return number of bytes the value encodes into.
 */
FORCE_INLINE uinttype SizeofUint32(const uinttype iValue) {
	return VarIntLength<uinttype>(iValue);
}

/**
 * WriteInt64ToBuffer
 * Write a variable sized INT64 to the buffer provided.
 * @param piBuffer - the location to write
 * @param iLen - the remaining buffer length
 * @param value [in] - the value to write
 * @return number of bytes written.
 */
FORCE_INLINE uinttype WriteInt64ToBuffer(ubytetype*     piBuffer,
					 const uinttype iLen,
					 const longtype iValue) {
	ulongtype iWrite  = encodeZigZag64(iValue);
	uinttype  iBytes  = 0;
	inttype   iResult = WriteVarIntToBuffer<ulongtype>(piBuffer,
							   iLen,
							   iBytes,
							   iWrite);
	if (0 != iResult) {
		return 0;
	}

	return iBytes;
}

/**
 * SizeofInt64
 * Returns the expected encoding size for a INT64.
 * @param iValue - the value to get an encoding size for.
 * @return number of bytes the value encodes into.
 */
FORCE_INLINE uinttype SizeofInt64(const longtype iValue) {
	ulongtype iWrite = encodeZigZag64(iValue);
	return VarIntLength<ulongtype>(iWrite);
}

/**
 * WriteUint64ToBuffer
 * Write a variable sized UINT64 to the buffer provided.
 * @param piBuffer - the location to write
 * @param iLen - the remaining buffer length
 * @param value [in] - the value to write
 * @return number of bytes written.
 */
FORCE_INLINE uinttype WriteUint64ToBuffer(ubytetype*      piBuffer,
					  const uinttype  iLen,
					  const ulongtype iValue) {
	uinttype iBytes  = 0;
	inttype  iResult = WriteVarIntToBuffer<ulongtype>(piBuffer,
							  iLen,
							  iBytes,
							  iValue);
	if (0 != iResult) {
		return 0;
	}

	return iBytes;
}

/**
 * SizeofUint64
 * Returns the expected encoding size for a UINT64.
 * @param iValue - the value to get an encoding size for.
 * @return number of bytes the value encodes into.
 */
FORCE_INLINE uinttype SizeofUint64(const ulongtype iValue) {
	return VarIntLength<ulongtype>(iValue);
}

/**
 * WriteFloatToBuffer
 * Write a float to the buffer provided.
 * @param piBuffer - the location to write
 * @param iLen - the remaining buffer length
 * @param value [in] - the value to write
 * @return number of bytes written.
 */
FORCE_INLINE uinttype WriteFloatToBuffer(ubytetype*     piBuffer,
					 const uinttype iLen,
					 const float    iValue) {
	if (INT32_SIZE > iLen) {
		return 0;
	}

	inttype iVal = convertFloat32ToInt32(iValue);

	return WriteFixed32<inttype>(piBuffer, iLen, iVal);
}

/**
 * SizeofFloat
 * Returns the expected encoding size for a float.
 * @param iValue - the value to get an encoding size for.
 * @return number of bytes the value encodes into.
 */
FORCE_INLINE uinttype SizeofFloat(const inttype iValue) {
	return FLOAT_SIZE;
}

/**
 * WriteDoubleToBuffer
 * Write a double to the buffer provided.
 * @param piBuffer - the location to write
 * @param iLen - the remaining buffer length
 * @param value [in] - the value to write
 * @return number of bytes written.
 */
FORCE_INLINE uinttype WriteDoubleToBuffer(ubytetype*     piBuffer,
					  const uinttype iLen,
					  const double   iValue) {
	longtype iVal = convertFloat64ToInt64(iValue);

	return WriteFixed64<longtype>(piBuffer, iLen, iVal);
}

/**
 * SizeofDouble
 * Returns the expected encoding size for a float.
 * @param iValue - the value to get an encoding size for.
 * @return number of bytes the value encodes into.
 */
FORCE_INLINE uinttype SizeofDouble(const inttype iValue) {
	return DOUBLE_SIZE;
}

/**
 * ReadInt32FromBuffer
 * Read in a variable sized INT32 value from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param value [out] - location to place the value.
 * @return number of bytes read.
 */
FORCE_INLINE inttype ReadInt32FromBuffer(const ubytetype* piBuffer,
					 const uinttype   iLen,
					 inttype&         iValue) {
	uinttype iBytes = 0;
	uinttype iRead  = 0;
	
	inttype iResult = ReadVarIntFromBuffer<uinttype>(piBuffer,
							 iLen,
							 iBytes,
							 iRead);
	if (0 != iResult) {
		return 0;
	}

	iValue = decodeZigZag32(iRead);

	return iBytes;
}

/**
 * ReadUint32FromBuffer
 * Read in a variable sized UINT32 value from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param value [out] - location to place the value.
 * @return number of bytes read.
 */
FORCE_INLINE inttype ReadUint32FromBuffer(const ubytetype* piBuffer,
					  const uinttype iLen,
					  uinttype&      iValue) {
	uinttype iBytes  = 0;
	inttype  iResult = ReadVarIntFromBuffer<uinttype>(piBuffer,
							  iLen,
							  iBytes,
							  iValue);
	if (0 != iResult) {
		return 0;
	}

	return iBytes;
}

/**
 * ReadInt64FromBuffer
 * Read in a variable sized INT64 value from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param value [out] - location to place the value.
 * @return number of bytes read.
 */
FORCE_INLINE inttype ReadInt64FromBuffer(const ubytetype* piBuffer,
					 const uinttype   iLen,
					 longtype&        iValue) {
	uinttype iBytes = 0;
	ulongtype iRead  = 0;
	
	inttype iResult = ReadVarIntFromBuffer<ulongtype>(piBuffer,
							  iLen,
							  iBytes,
							  iRead);
	if (0 != iResult) {
		return 0;
	}

	iValue = decodeZigZag64(iRead);

	return iBytes;
}

/**
 * ReadUint64FromBuffer
 * Read in a variable sized UINT64 value from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param value [out] - location to place the value.
 * @return number of bytes read.
 */
FORCE_INLINE int ReadUint64FromBuffer(const ubytetype* piBuffer,
				      const uinttype   iLen,
				      ulongtype&       iValue) {
	uinttype iBytes  = 0;
	inttype  iResult = ReadVarIntFromBuffer<ulongtype>(piBuffer,
							   iLen,
							   iBytes,
							   iValue);
	if (0 != iResult) {
		return 0;
	}

	return iBytes;
}

/**
 * ReadFloatFromBuffer
 * Read in a float value from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param value [out] - location to place the value.
 * @return number of bytes read.
 */
FORCE_INLINE uinttype ReadFloatFromBuffer(const ubytetype* piBuffer,
					  const uinttype   iLen,
					  float&           fValue) {
	if (FIXED32_SIZE > iLen) {
		return 0;
	}

	inttype  iVal  = 0;
	uinttype iSize = ReadFixed32<inttype>(piBuffer, iLen, iVal);

	fValue = convertInt32ToFloat32(iVal);

	return iSize;
}

/**
 * ReadDoubleFromBuffer
 * Read in a double value from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param value [out] - location to place the value.
 * @return number of bytes read.
 */
FORCE_INLINE uinttype ReadDoubleFromBuffer(const ubytetype* piBuffer,
					   const uinttype   iLen,
					   double&          fValue) {
	if (FIXED64_SIZE > iLen) {
		return 0;
	}

	longtype  iVal  = 0;
	uinttype iSize = ReadFixed64<longtype>(piBuffer, iLen, iVal);
	
	fValue = convertInt64ToFloat64(iVal);

	return iSize;
}

/**
 * WriteRawToBuffer
 * Writes an opaque blob to the buffer. This is a UINT32
 * length prefixed blob for binary data.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param piRaw - data blob to write
 * @param iRawLen - length of the blob.
 * @return number of bytes read.
 */
FORCE_INLINE uinttype WriteRawToBuffer(ubytetype*       piBuffer,
				       const uinttype   iLen,
				       const ubytetype* piRaw,
				       const uinttype   iRawLen) {
	if ((iRawLen + FIXED32_SIZE) > iLen) {
		return 0;
	}

	/** Write the length of the binary blob. 32Bit Length. */
	WriteFixed32<uinttype>(piBuffer, iLen, iRawLen);

	if (0 != iRawLen) {
		/** Write the binary blob. */
		memcpy(piBuffer + FIXED32_SIZE, piRaw, iRawLen);
	}

	return iRawLen + FIXED32_SIZE;
}

/**
 * WriteStringToBuffer
 * Writes a char string array to the buffer. This is a UINT32
 * length prefixed char string. Not zero terminated.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param piStr - string to write
 * @param iRawLen - length of the string.
 * @return number of bytes read.
 */
FORCE_INLINE uinttype WriteStringToBuffer(ubytetype*     piBuffer,
					  const uinttype iLen,
					  const char*    piStr,
					  const uinttype iStrLen) {
	if ((iStrLen + FIXED32_SIZE) > iLen) {
		return 0;
	}

	/** Write the length of the binary blob. 32Bit Length. */
	WriteFixed32<uinttype>(piBuffer, iLen, iStrLen);

	if (0 != iStrLen) {
		/** Write the binary blob. */
		memcpy(piBuffer + FIXED32_SIZE, piStr, iStrLen);
	}

	return iStrLen + FIXED32_SIZE;
}

/**
 * WriteStringToBuffer
 * Writes a cxx::lang::String to the buffer. This is a UINT32
 * length prefixed char string. Not zero terminated.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param cStr - string to write
 * @return number of bytes read.
 */
FORCE_INLINE uinttype WriteStringToBuffer(ubytetype*         piBuffer,
					  const uinttype     iLen,
					  cxx::lang::String& cStr) {
	return WriteStringToBuffer(piBuffer, iLen, cStr.c_str(), cStr.size());
}

/**
 * SizeofRaw
 * Returns the expected encoding size for a binary encoding.
 * @param iValue - the value to get an encoding size for.
 * @return number of bytes this encodes into.
 */
FORCE_INLINE uinttype SizeofRaw(const uinttype iRawLen) {
	return (iRawLen + FIXED32_SIZE);
}

/**
 * SizeofString
 * Returns the expected encoding size for a string encoding.
 * @param iValue - the value to get an encoding size for.
 * @return number of bytes this encodes into.
 */
FORCE_INLINE uinttype SizeofString(const uinttype iStrLen) {
	return (iStrLen + FIXED32_SIZE);
}

/**
 * SizeofString
 * Returns the expected encoding size for a string encoding.
 * @param iValue - the value to get an encoding size for.
 * @return number of bytes this encodes into.
 */
FORCE_INLINE uinttype SizeofString(cxx::lang::String& cStr) {
	return (cStr.size() + FIXED32_SIZE);
}

/**
 * ReadRawFromBuffer
 * Read in a raw opaque blob from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param piRaw [out] - location to place the blob.
 * @param iRawLen [out] - location to write the length.
 * @return number of bytes read.
 */
FORCE_INLINE uinttype ReadRawFromBuffer(const ubytetype* piBuffer,
					const uinttype   iLen,
					ubytetype*       piRaw,
					uinttype&        iRawLen) {
	if (FIXED32_SIZE > iLen) {
		return 0;
	}

	/** Find the blobs length in the prefixed 32bits. */
	uinttype iFound = 0;
	ReadFixed32<uinttype>(piBuffer, iLen, iFound);

	if (iFound > (iLen - FIXED32_SIZE)) {
		return 0;
	}

	iRawLen = iFound;

	if (0 != iRawLen) {
		/** Read in the binary blob. */
		memcpy(piRaw, piBuffer + FIXED32_SIZE, iRawLen);
	}

	return iRawLen + FIXED32_SIZE;
}

/**
 * ReadStringFromBuffer
 * Read in a char string from the buffer.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param piStr [out] - location to place the string.
 * @param iStrLen [out] - location to write the length.
 * @return number of bytes read.
 */
FORCE_INLINE uinttype ReadStringFromBuffer(const ubytetype* piBuffer,
					   const uinttype   iLen,
					   char*            piStr,
					   uinttype&        iStrLen) {
	if (FIXED32_SIZE > iLen) {
		return 0;
	}

	/** Find the blobs length in the prefixed 32bits. */
	uinttype iFound = 0;
	ReadFixed32<uinttype>(piBuffer, iLen, iFound);

	if (iFound > (iLen - FIXED32_SIZE)) {
		return 0;
	}

	iStrLen = iFound;

	if (0 != iStrLen) {
		/** Read in the binary blob. */
		memcpy(piStr, piBuffer + FIXED32_SIZE, iStrLen);
	}

	return iStrLen + FIXED32_SIZE;
}

/**
 * ReadStringFromBuffer
 * Read in a char string from the buffer and make this a
 * cxx::lang::String.
 * @param piBuffer - position to read from
 * @param iLen - the remaining buffer length
 * @param cStr [out] - location to place the string.
 * @return number of bytes read.
 */
FORCE_INLINE uinttype ReadStringFromBuffer(const ubytetype*   piBuffer,
					   const uinttype     iLen,
					   cxx::lang::String& cStr) {
	if (FIXED32_SIZE > iLen) {
		return 0;
	}

	/** Find the blobs length in the prefixed 32bits. */
	uinttype iFound = 0;
	ReadFixed32<uinttype>(piBuffer, iLen, iFound);

	if (iFound > (iLen - FIXED32_SIZE)) {
		return 0;
	}

	if (0 != iFound) {
		const char* piStr = reinterpret_cast<const char*>(piBuffer);

		cxx::lang::String cRetVal((piStr + FIXED32_SIZE), iFound);
		cStr = cRetVal;
	}

	return iFound + FIXED32_SIZE;
}

} /** binaryWireFormat */

} /** io  */

} /** cxx */

#endif /** CXX_LANG_UTILS_BINARYWIREFORMAT_H_ */
