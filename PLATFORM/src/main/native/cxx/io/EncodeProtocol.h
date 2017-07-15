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
#ifndef CXX_LANG_IO_ENCODEPROTOCOL_H_
#define CXX_LANG_IO_ENCODEPROTOCOL_H_

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <endian.h>
#include <string.h>
#include <string>
#include <vector>

#include "cxx/io/EOFException.h"
#include "cxx/io/BinaryWireFormat.h"
#include "cxx/lang/types.h"
#include "cxx/lang/array.h"

namespace cxx { namespace io { namespace encodeProtocol {

/**
 * Exceptions to handle errors beyond the LinkManager API
 */
class EncodeProtocolException : public Exception {
public:
	EncodeProtocolException(void) {
	}

	EncodeProtocolException(const char* message)
		: Exception(message) {
	}
}; /** EncodeProtocolException */

struct types {
/** Never modify these values. New types are appended to this list. */
/** Values are bounded by the 0x7f mask. */
	static const ubytetype TYPE_MASK  = 0x7f;
	static const ubytetype TYPE_ARRAY = 0x80;

	static const ubytetype NIL        = 0;
	static const ubytetype INT32      = 1;
	static const ubytetype UINT32     = 2;
	static const ubytetype INT64      = 3;
	static const ubytetype UINT64     = 4;
	static const ubytetype FLOAT      = 5;
	static const ubytetype DOUBLE     = 6;
	static const ubytetype RAW        = 7;
	static const ubytetype STRING     = 8;
	static const ubytetype BOOLEAN    = 9;
	static const ubytetype BUNDLE     = 10;
	static const ubytetype VOIDPTR    = 11;

	static const ubytetype UINT32_ARRAY = (TYPE_ARRAY | UINT32);

	static FORCE_INLINE ubytetype arrayType(ubytetype iType) {
		return (TYPE_ARRAY | iType);
	}

}; /** types */

struct header {

	static const uinttype DEEP_MAGIC  = 0x50454544;
	static const uinttype VERSION     = 0x00000001;
	static const uinttype REPLY_FLAG  = 0x00000100;
	static const uinttype FIRST_FLAG  = 0x00000200;
	static const uinttype LAST_FLAG   = 0x00000400;
	static const uinttype BLOCK_FLAG  = 0x00000800;
	static const uinttype ONE_MSG     = FIRST_FLAG | LAST_FLAG;
	static const uinttype HEADER_SIZE = cxx::io::binaryWireFormat::FIXED32_SIZE * 4;
};

struct fields {
	static const uinttype FIELD_HDR_SIZE  = cxx::io::binaryWireFormat::FIXED32_SIZE;
	static const uinttype FIELD_MAX       = 256;
	static const uinttype FIELD_BUNDLE_ID = 1;

	static FORCE_INLINE uinttype makeHeader(ubytetype iType,
						uinttype  iId) {
		return (((0x00ffffff & iId) << 8) | iType);
	}

	static FORCE_INLINE void getTypeId(uinttype   iFieldHdr,
					   ubytetype& iType,
					   uinttype&  iId) {
		iType = 0x000000ff & iFieldHdr;
		iId   = (iFieldHdr >> 8) & 0x00ffffff;
	}

}; /** fields */

/**
 0     7                    31               0       8                  31
 +---------------------------+    <       >  +------+--------------------+
 |     DEEP_MAGIC="DEEP"     |    |          | Type |         ID         |
 +------+--------------------+    |       |  +------+--------------------+
 | Ver  |       Flags        |    |          |VarInt|
 +------+--------------------+ Header     |  +------+
 |          Msg ID           |    |
 +---------------------------+    |       |  +------+--------------------+
 |    Payload Byte Length    |    |          | Type |         ID         |
 +---------------------------+    <       |  +------+--------------------+
 |                           |               |          VarInt           |
 |                           |            |  +---------------------------+
 |                           |
 |                           |            |  +------+--------------------+
 |                           |               | Type |         ID         |
 |                           |            |  +------+--------------------+
 |                           |               |        String Size        |
 |                           |            |  +---------------------------+
 |                           |               |                           |
 |                           |            |  |                           |
 |                           |               |                           |
 |                           |            |  |        String Data        |
 |                           |               |                           |
 |                           |            |  |                           |
 |                           |               |                           |
 |                           |            |  |                           |
 |                           |               +---------------------------+
 |        Binary Data        |- - - - - - +
 |                           |               +------+--------------------+
 |                           |            |  | Type |         ID         |
 |                           |               +------+-----------+--------+
 |                           |            |  |Count (Var Length)|
 |                           |               +------------------+--------+
 |                           |            |  |    Raw Data Byte Size     |
 |                           |               +---------------------------+
 |                           |            |  |                           |
 |                           |               |         Raw Data          |
 |                           |            |  |                           |
 |                           |               +---------------------------+
 |                           |            |  |    Raw Data Byte Size     |
 |                           |               +---------------------------+
 |                           |            |  |                           |
 |                           |               |         Raw Data          |
 |                           |            |  |                           |
 |                           |               +---------------------------+
 +---------------------------+            |
                                             +------+--------------------+
                                          |  | Type |         ID         |
                                             +------+-----------+--------+
                                          |  |Count (Var Length)|
                                             +------+-----------+
                                          |  |VarInt|
                                             +------+-------+
                                          |  |    VarInt    |
                                             +--------------+
                                          |
                                             +------+--------------------+
                                          |  | Type |         ID         |
                                             +------+-----------+--------+
                                          |  |Count (Var Length)|
                                             +------------------+--------+
                                          |  |           Float           |
                                             +---------------------------+
                                          |  |           Float           |
                                             +---------------------------+
                                          |
                                             +------+--------------------+
                                          |  | Type |         ID         |
                                             +------+--------------------+
                                          |  |                           |
                                             |          Double           |
                                          |  |                           |
                                          >  +---------------------------+
*/

/**
 * writer
 * Has a singular one-shot purpose to construct an encoded message
 * using the setTypeField methods. The Integer types are variable in
 * size and the value is needed in order to compute the
 * space needed to encode the complete field, hence the sizeOfType
 * methods. Use these to determine iBufferLen to encode all of the
 * fields.
 * Each field has to have a unique number between 0..2047. The message
 * id is a schema number that the encoder can use to inform the
 * receiver of the message encoding format.
 * Field order is not important, but fields must always keep the
 * assigned type for verison compatibility. It is possible to not
 * encode a depricated field. The receiver will get a error status
 * for missing fields it tries to get.
 * Once the internal buffer and size is fetched in getMutableBuffer,
 * the class is effectively a zombie and not fit for use. The buffer
 * ownership transfers to the caller of the method.
 */
class writer {
public:
	writer(uinttype iBufferLen,
	       uinttype iMsgId);

	writer(ubytetype* piBuffer,
	       uinttype   iBufferLen,
	       uinttype   iPayloadLen,
	       uinttype   iMsgId,
	       boolean    bBundleShim = false);

	writer(writer& pcClone);

	virtual ~writer();

	inttype setIsReply();
	inttype setFlags(uinttype iFlags);
	static inttype setFlags(ubytetype* piBuffer, uinttype iFlags);

	inttype getFlags(uinttype& iFlags);

	inttype getMutableBuffer(ubytetype*& piBuffer,
				 uinttype&   iBufferLen);
	uinttype getSize(void);

	const voidptr getBufferRef(void) const;

	static uinttype sizeOfInt32(inttype iValue);
	static uinttype sizeOfUint32(uinttype iValue);
	static uinttype sizeOfInt64(longtype iValue);
	static uinttype sizeOfUint64(ulongtype iValue);
	static uinttype sizeOfFloat(floattype fValue);
	static uinttype sizeOfDouble(doubletype fValue);
	static uinttype sizeOfString(cxx::lang::String& cValue);
	static uinttype sizeOfRaw(uinttype iRawLen);

	static uinttype sizeOfUint32Array(cxx::lang::array<uinttype>& cValues);

	inttype setInt32Field(uinttype iFieldNum,
			      inttype  iValue);

	inttype setUint32Field(uinttype iFieldNum,
			       uinttype iValue);

	inttype setInt64Field(uinttype iFieldNum,
			      longtype  iValue);

	inttype setUint64Field(uinttype iFieldNum,
			       ulongtype iValue);

	inttype setFloatField(uinttype iFieldNum,
			      float    fValue);

	inttype setDoubleField(uinttype iFieldNum,
			       double   fValue);

	inttype setStringField(uinttype     iFieldNum,
			       cxx::lang::String& cValue);

	inttype setRawField(uinttype         iFieldNum,
			    const ubytetype* piRaw,
			    const uinttype   iRawLen);

	inttype setUint32ArrayField(uinttype                    iFieldNum,
				    cxx::lang::array<uinttype>& cValues);

private:
	void _init(uinttype   iPayloadLen,
		   uinttype   iMsgId);

	ubytetype* m_piBuffer;
	uinttype   m_iActualLen;
	uinttype   m_iBufferLen;
	uinttype   m_iWritten;
	ubytetype* m_piPos;
	
}; /** writer */

class renderReader;

/**
 * reader
 * Is used to decode a buffer previously encoded using the 'writer'
 * object. First the reader must be constructed with the buffer
 * pointer and buffer length. Then, when ready the caller must
 * execute the 'decodeFields' method in order for fields to be
 * parsed and extracted as needed.
 * The 'getMsgId' method can then be used to descriminate on the
 * schema layout of the fields and field types.
 * The order of 'getTypeField' calls is not important. Internally
 * the object has a simple map of field ID to value/value location.
 * Any field fetched that does no exist will return a status error.
 * Any mismatched field/id type get call will result in a error
 * status returned.
 * Raw Encoded data and strings are handled slightly differently to
 * the descrete types. getRawFieldRef and getStringFieldRef return a
 * const reference to the buffer contents where the value is
 * encoded. This is because such types can be large and for the sake
 * of copy/memory efficiency a reference may be sufficient for most
 * needs. NOTE: These types can be encoded as zero length.
 * The descrete types are copies. The integer values are variable
 * sized and require processing to explode them up to the correct
 * type. The float/doubles are fixed sized but are also encoded.
 * Array types of all of these types will be supported as and when
 * this functionality is needed.
 */
class reader {
public:
	reader(ubytetype* piBuffer,
	       uinttype   iBufferLen,
	       boolean    bCopyAll = false);

	reader(const reader& cReader);

	virtual ~reader();

	/** Initiates the decode sequence of the buffer fields. */
	inttype decodeFields();

	/** The message ID found in the message header. */
	const uinttype getMsgId();

	const boolean  isReply();
	const boolean  isFirst();
	const boolean  isLast();
	const boolean  toBlock();

	uinttype getEncodedLength();

	/** Return a copy of the value referenced by the field. */
	inttype getInt32Field(uinttype iId,
			      inttype& iVal);
	inttype getUint32Field(uinttype  iId,
			       uinttype& iVal);
	inttype getInt64Field(uinttype iId,
			      longtype& iVal);
	inttype getUint64Field(uinttype  iId,
			       ulongtype& iVal);
	inttype getFloatField(uinttype   iId,
			      floattype& fVal);
	inttype getDoubleField(uinttype    iId,
			       doubletype& fVal);
	inttype getStringField(uinttype           iId,
			       cxx::lang::String& cStr);

	/**
	 * These types are potentially large and inefficient to
	 * return, so we return pointer and size reference to allow
	 * direct access to the buffer contents that house the value.
	 */
	inttype getRawFieldRef(uinttype          iId,
			       const ubytetype*& pcRaw,
			       uinttype&         iSize);
	inttype getStringFieldRef(uinttype     iId,
				  const char*& pcStr,
				  uinttype&    iSize);

	inttype getUint32ArrayField(uinttype                     iId,
				    cxx::lang::array<uinttype>*& pcValues);

	inttype getBundleBuffer(const ubytetype*& pBuffer,
				uinttype&         iSize);

	inttype getVoidPtr(uinttype iId,
			   voidptr& pPtr);

private:
	reader(uinttype iMsgId);

	struct fieldBucket {
		union values {
			inttype    i32;
			uinttype   ui32;
			longtype   i64;
			ulongtype  ui64;
			floattype  f;
			doubletype d;
			ubytetype* piRaw;
			voidptr    pArray;
		};
		
		fieldBucket() : m_iType(0),
				m_iEncodedLen(0) {
		}

		~fieldBucket() {
			m_iType       = 0;
			m_iEncodedLen = 0;
		}

		ubytetype  m_iType;
		values     m_cValues;
		uinttype   m_iEncodedLen;

	}; /** fieldBucket */

	friend class renderReader;

private:
	/** Member variables. */
	ubytetype*    m_piBuffer;
	uinttype      m_iBufferLen;
	uinttype      m_iMsgId;
	fieldBucket   m_iFields[fields::FIELD_MAX];
	uinttype      m_iFlags;
	boolean       m_bCopyAll;

}; /* reader */

/**
 * renderReader: This is used to create a reader object that mimics
 * having being decoded from binary data. This is useful for creating
 * data objects to send between threads that require no
 * stream/destream overhead.
 */
class renderReader {
public:
	renderReader(uinttype iMsgId);
	virtual ~renderReader();

	reader* mutableReader();

	inttype setInt32Field(uinttype iFieldNum,
			      inttype  iValue);

	inttype setUint32Field(uinttype iFieldNum,
			       uinttype iValue);

	inttype setInt64Field(uinttype iFieldNum,
			      longtype  iValue);

	inttype setUint64Field(uinttype iFieldNum,
			       ulongtype iValue);

	inttype setFloatField(uinttype iFieldNum,
			      float    fValue);

	inttype setDoubleField(uinttype iFieldNum,
			       double   fValue);

	inttype setStringField(uinttype           iFieldNum,
			       cxx::lang::String& cValue);

	inttype setRawField(uinttype         iFieldNum,
			    const ubytetype* piRaw,
			    const uinttype   iRawLen);

	inttype setVoidPtr(uinttype iFieldNum,
			   voidptr  pPtr);

private:
	reader* m_pcReader;

}; /** renderReader */

} /** encodeProtocol */

} /** io */

} /** cxx */


/** writer */

#define WRITE_SPINS(_X) { m_iBufferLen -= _X ; m_piPos += _X; m_iWritten += _X; }

/**
 * writer
 * Constructs a buffer of the specified length + header size.
 * We then write the header to the created buffer.
 * @param iBufferLen - Payload Length (non-header)
 * @param iMsgId     - Messgae ID
 */
FORCE_INLINE cxx::io::encodeProtocol::writer::
writer(uinttype iBufferLen,
       uinttype iMsgId) : m_piBuffer(0),
			  m_iActualLen(iBufferLen + header::HEADER_SIZE),
			  m_iBufferLen(m_iActualLen),
			  m_iWritten(0),
			  m_piPos(0) {

	m_piBuffer = new ubytetype[m_iBufferLen];
	if (0 == m_piBuffer) {
		abort();
	}

	_init(iBufferLen, iMsgId);
}

/**
 * writer
 * Constructs a writer around the provided buffer. Shims any
 * encoded data which is assumed to be header size + field size past
 * the buffer pointer. The assumption is that once created the BUNDLE
 * field will be the only field in the message after the header.
 * Therefore BUNDLE field length is then the payload length - field size.
 * @param piBuffer    - the buffer to encode into.
 * @param iBufferLen  - the actual buffer size
 * @param iPayloadLen - the encoded section length.
 * @param iMsgId      - Messgae ID
 */
FORCE_INLINE cxx::io::encodeProtocol::writer::
writer(ubytetype* piBuffer,
       uinttype   iBufferLen,
       uinttype   iPayloadLen,
       uinttype   iMsgId,
       boolean    bBundleShim) : m_piBuffer(piBuffer),
				 m_iActualLen(iBufferLen),
				 m_iBufferLen(iBufferLen),
				 m_iWritten(0),
				 m_piPos(0) {

	if (header::HEADER_SIZE > iBufferLen) {
		abort();
	}

	_init(iPayloadLen, iMsgId);

	if (true == bBundleShim) {
		uinttype iHdr = fields::makeHeader(types::BUNDLE, fields::FIELD_BUNDLE_ID);
		uinttype iWritten =
			cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos, m_iBufferLen, iHdr);
		if (0 == iWritten) {
			abort();
		}

		m_iWritten = cxx::io::encodeProtocol::header::HEADER_SIZE + iPayloadLen;
	}
}

/**
 * writer
 * Constructs a buffer of the specified length + header size.
 * We then write the header to the created buffer.
 * @param iBufferLen - Payload Length (non-header)
 * @param iMsgId     - Messgae ID
 */
FORCE_INLINE cxx::io::encodeProtocol::writer::
writer(writer& cClone) : m_piBuffer(0),
			 m_iActualLen(cClone.m_iActualLen),
			 m_iBufferLen(cClone.m_iBufferLen),
			 m_iWritten(cClone.m_iWritten),
			 m_piPos(0) {

	m_piBuffer = new ubytetype[cClone.m_iActualLen];
	if (0 == m_piBuffer) {
		abort();
	}

	memcpy(m_piBuffer, cClone.m_piBuffer, cClone.m_iActualLen);

	m_piPos = &m_piBuffer[cClone.m_iActualLen];
}

/**
 * _init
 * Common constructor initialization routine.
 * @param iPayloadLen - the payload data length
 * @param iMsgId      - Message ID to encode into header.
 */
FORCE_INLINE void cxx::io::encodeProtocol::writer::
_init(uinttype   iPayloadLen,
      uinttype   iMsgId) {

	m_piPos = m_piBuffer;
	
	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  header::DEEP_MAGIC);
	if (0 == iWritten) {
		abort();
	}

	WRITE_SPINS(iWritten);

	uinttype iVerFlags = header::VERSION;

	iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iVerFlags);
	if (0 == iWritten) {
		abort();
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iMsgId);

	if (0 == iWritten) {
		abort();
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iPayloadLen);
	if (0 == iWritten) {
		abort();
	}

	WRITE_SPINS(iWritten);
}

/**
 * ~writer
 * Destructor
 */
FORCE_INLINE cxx::io::encodeProtocol::writer::~writer() {
	if (0 != m_piBuffer) {
		delete [] m_piBuffer; m_piBuffer = 0;
	}
	m_iActualLen = 0;
	m_iBufferLen = 0;
	m_piPos      = 0;
}

/**
 * setIsReply
 * Sets the REPLY flag in the message header. Overwrites previous flags.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::setIsReply() {
	return setFlags(header::REPLY_FLAG);
}

/**
 * setIsReply
 * Sets the position flags in the message header. Overwrites previous flags.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::setFlags(uinttype iFlags) {
	if ((0 == m_piBuffer) ||
	    (0 == m_iWritten) ||
	    (m_iWritten < header::HEADER_SIZE)) {
		return -1;
	}

	return setFlags(m_piBuffer, iFlags);
}

/**
 * setFlags
 * Sets the flags bits at the offset from the pointer, which
 * is assumed to be the beginning of the message header,
 * @param piBuffer - reference to the message header.
 * @param iFlags - Flag bits to set in the header.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::setFlags(ubytetype* piBuffer, uinttype iFlags) {
	/**
	 * Just writes the VERSION and position flags.
	 */
	ubytetype* piVerFlags = piBuffer + cxx::io::binaryWireFormat::FIXED32_SIZE;
	uinttype   iVerFlags  = 0;

	uinttype iRead = cxx::io::binaryWireFormat::ReadFixed32<uinttype>(piVerFlags,
									  cxx::io::binaryWireFormat::FIXED32_SIZE,
									  iVerFlags);
	if (0 == iRead) {
		abort();
	}

	iVerFlags |= (header::VERSION | iFlags);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(piVerFlags,
								  cxx::io::binaryWireFormat::FIXED32_SIZE,
								  iVerFlags);
	if (0 == iWritten) {
		abort();
	}

	return 0;
}

/**
 * getFlags
 * Return any set flags in the writer buffer.
 * @param iFlags [out] - the flags from the encoded header.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::getFlags(uinttype& iFlags) {
	if ((0 == m_piBuffer) ||
	    (0 == m_iWritten) ||
	    (m_iWritten < header::HEADER_SIZE)) {
		return -1;
	}

	ubytetype* piVerFlags = m_piBuffer + cxx::io::binaryWireFormat::FIXED32_SIZE;

	uinttype iRead = cxx::io::binaryWireFormat::ReadFixed32<uinttype>(piVerFlags,
									  cxx::io::binaryWireFormat::FIXED32_SIZE,
									  iFlags);
	if (0 == iRead) {
		abort();
	}

	return 0;
}

/**
 * getMutableBuffer
 * Returns a pointer to the mutable buffer and then
 * invalidates the writer from being used any further.
 * @param piBuffer [out] - location to write the buffer pointer.
 * @patam iBufferLen [out] - returns the buffer length.
 * @return pointer to the buffer.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
getMutableBuffer(ubytetype*& piBuffer, uinttype& iBufferLen) {
	piBuffer     = m_piBuffer;
	iBufferLen   = m_iWritten;
	m_piBuffer   = 0;
	m_iActualLen = 0;
	m_iBufferLen = 0;
	m_piPos      = 0;

	return 0;
}

/**
 * getSize
 * Returns the size of the encoded message so far.
 * @return the wriiten size in bytes.
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
getSize(void) {
	return m_iWritten;
}

/**
 * getBufferRef
 * Returns a const reference to the internal buffer for this message.
 * @return a const reference to the internal buffer.
 */
FORCE_INLINE const voidptr cxx::io::encodeProtocol::writer::
getBufferRef(void) const {
	voidptr pRetVal =
		static_cast<const voidptr>(m_piBuffer);
	return pRetVal;
}

/**
 * sizeOfInt32
 * Returns the number of bytes needed to encode this value.
 * @param iValue - The value to calculate.
 * @return uinttype - the bytes value
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
sizeOfInt32(inttype iValue) {
	return (cxx::io::binaryWireFormat::SizeofInt32(iValue) +
		fields::FIELD_HDR_SIZE);
}

/**
 * setInt32Field
 * Sets the encoded value and header/id to the internal buffer.
 * @param iFieldNum - the field ID for this value.
 * @param iValue - The value to encode.
 * @return the number of bytes written.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
setInt32Field(uinttype iFieldNum,
	      inttype  iValue) {

	if (fields::FIELD_MAX <= iFieldNum) {
		return -1;
	}

	uinttype iSize = sizeOfInt32(iValue);
	if (m_iBufferLen < iSize) {
		return -1;
	}

	uinttype iHdr = fields::makeHeader(types::INT32,
					   iFieldNum);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iHdr);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteInt32ToBuffer(m_piPos,
							      m_iBufferLen,
							      iValue);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	return 0;
}

/**
 * sizeOfUint32
 * Returns the number of bytes needed to encode this value.
 * @param iValue - The value to calculate.
 * @return uinttype - the bytes value
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
sizeOfUint32(uinttype iValue) {
	return (cxx::io::binaryWireFormat::SizeofUint32(iValue) +
		fields::FIELD_HDR_SIZE);
}

/**
 * setUint32Field
 * Sets the encoded value and header/id to the internal buffer.
 * @param iFieldNum - the field ID for this value.
 * @param iValue - The value to encode.
 * @return the number of bytes written.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
setUint32Field(uinttype iFieldNum,
	       uinttype iValue) {

	if (fields::FIELD_MAX <= iFieldNum) {
		return -1;
	}

	uinttype iSize = sizeOfUint32(iValue);
	if (m_iBufferLen < iSize) {
		return -1;
	}

	uinttype iHdr = fields::makeHeader(types::UINT32,
					   iFieldNum);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iHdr);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteUint32ToBuffer(m_piPos,
							       m_iBufferLen,
							       iValue);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	return 0;
}

/**
 * sizeOfInt64
 * Returns the number of bytes needed to encode this value.
 * @param iValue - The value to calculate.
 * @return uinttype - the bytes value
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
sizeOfInt64(longtype iValue) {
	return (cxx::io::binaryWireFormat::SizeofInt64(iValue) +
		fields::FIELD_HDR_SIZE);
}

/**
 * setInt64Field
 * Sets the encoded value and header/id to the internal buffer.
 * @param iFieldNum - the field ID for this value.
 * @param iValue - The value to encode.
 * @return the number of bytes written.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
setInt64Field(uinttype iFieldNum,
	      longtype iValue) {

	if (fields::FIELD_MAX <= iFieldNum) {
		return -1;
	}

	uinttype iSize = sizeOfInt64(iValue);
	if (m_iBufferLen < iSize) {
		return -1;
	}

	uinttype iHdr = fields::makeHeader(types::INT64,
					   iFieldNum);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iHdr);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteInt64ToBuffer(m_piPos,
							      m_iBufferLen,
							      iValue);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	return 0;
}

/**
 * sizeOfUint64
 * Returns the number of bytes needed to encode this value.
 * @param iValue - The value to calculate.
 * @return uinttype - the bytes value
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
sizeOfUint64(ulongtype iValue) {
	return (cxx::io::binaryWireFormat::SizeofUint64(iValue) +
		fields::FIELD_HDR_SIZE);
}

/**
 * setInt64Field
 * Sets the encoded value and header/id to the internal buffer.
 * @param iFieldNum - the field ID for this value.
 * @param iValue - The value to encode.
 * @return the number of bytes written.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
setUint64Field(uinttype  iFieldNum,
	       ulongtype iValue) {

	if (fields::FIELD_MAX <= iFieldNum) {
		return -1;
	}

	uinttype iSize = sizeOfUint64(iValue);
	if (m_iBufferLen < iSize) {
		return -1;
	}

	uinttype iHdr = fields::makeHeader(types::UINT64,
					   iFieldNum);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iHdr);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteUint64ToBuffer(m_piPos,
							       m_iBufferLen,
							       iValue);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	return 0;
}

/**
 * sizeOfFloat
 * Returns the number of bytes needed to encode this value.
 * @param iValue - The value to calculate.
 * @return uinttype - the bytes value
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
sizeOfFloat(float fValue) {
	return (cxx::io::binaryWireFormat::SizeofFloat(fValue) +
		fields::FIELD_HDR_SIZE);
}

/**
 * setFloatField
 * Sets the encoded value and header/id to the internal buffer.
 * @param iFieldNum - the field ID for this value.
 * @param iValue - The value to encode.
 * @return the number of bytes written.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
setFloatField(uinttype iFieldNum,
	      float    fValue) {

	if (fields::FIELD_MAX <= iFieldNum) {
		return -1;
	}

	uinttype iSize = sizeOfFloat(fValue);
	if (m_iBufferLen < iSize) {
		return -1;
	}

	uinttype iHdr = fields::makeHeader(types::FLOAT,
					   iFieldNum);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iHdr);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteFloatToBuffer(m_piPos,
							      m_iBufferLen,
							      fValue);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	return 0;
}

/**
 * sizeOfDouble
 * Returns the number of bytes needed to encode this value.
 * @param iValue - The value to calculate.
 * @return uinttype - the bytes value
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
sizeOfDouble(doubletype fValue) {
	return (cxx::io::binaryWireFormat::SizeofDouble(fValue) +
		fields::FIELD_HDR_SIZE);
}

/**
 * setDoubleField
 * Sets the encoded value and header/id to the internal buffer.
 * @param iFieldNum - the field ID for this value.
 * @param iValue - The value to encode.
 * @return the number of bytes written.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
setDoubleField(uinttype   iFieldNum,
	       doubletype fValue) {

	if (fields::FIELD_MAX <= iFieldNum) {
		return -1;
	}

	uinttype iSize = sizeOfFloat(fValue);
	if (m_iBufferLen < iSize) {
		return -1;
	}

	uinttype iHdr = fields::makeHeader(types::DOUBLE,
					   iFieldNum);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iHdr);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteDoubleToBuffer(m_piPos,
							       m_iBufferLen,
							       fValue);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	return 0;
}

/**
 * sizeOfString
 * Returns the number of bytes needed to encode this value.
 * @param iValue - The value to calculate.
 * @return uinttype - the bytes value
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
sizeOfString(cxx::lang::String& cValue) {
	return (cxx::io::binaryWireFormat::SizeofString(cValue) +
		fields::FIELD_HDR_SIZE);
}

/**
 * setStringField
 * Sets the encoded value and header/id to the internal buffer.
 * @param iFieldNum - the field ID for this value.
 * @param iValue - The value to encode.
 * @return the number of bytes written.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
setStringField(uinttype           iFieldNum,
	       cxx::lang::String& cValue) {

	if (fields::FIELD_MAX <= iFieldNum) {
		return -1;
	}

	uinttype iSize = sizeOfString(cValue);
	if (m_iBufferLen < iSize) {
		return -1;
	}

	uinttype iHdr = fields::makeHeader(types::STRING,
					   iFieldNum);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iHdr);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteStringToBuffer(m_piPos,
							       m_iBufferLen,
							       cValue);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	return 0;
}

/**
 * sizeOfString
 * Returns the number of bytes needed to encode this value.
 * @param iRawLen - The length to use in the calculation.
 * @return uinttype - the bytes value
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
sizeOfRaw(uinttype iRawLen) {
	return (cxx::io::binaryWireFormat::SizeofRaw(iRawLen) +
		fields::FIELD_HDR_SIZE);
}

/**
 * setRawField
 * Sets the encoded value and header/id to the internal buffer.
 * @param iFieldNum - the field ID for this value.
 * @param piRaw - The value to encode.
 * @param iRawLen - The length of the raw value.
 * @return the number of bytes written.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
setRawField(uinttype         iFieldNum,
	    const ubytetype* piRaw,
	    const uinttype   iRawLen) {

	if (fields::FIELD_MAX <= iFieldNum) {
		return -1;
	}

	uinttype iSize = sizeOfRaw(iRawLen);
	if (m_iBufferLen < iSize) {
		return -1;
	}

	uinttype iHdr = fields::makeHeader(types::RAW,
					   iFieldNum);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iHdr);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	iWritten =
		cxx::io::binaryWireFormat::WriteRawToBuffer(m_piPos,
							    m_iBufferLen,
							    piRaw,
							    iRawLen);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	return 0;
}

/**
 * sizeOfUint32Array
 * Returns the number of bytes needed to encode these values.
 * @param cValue [in] - reference to the values to be written.
 * @return uinttype - the bytes value
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::writer::
sizeOfUint32Array(cxx::lang::array<uinttype>& cValues) {
	uinttype iSize = fields::FIELD_HDR_SIZE;

	for (inttype i=0; i<cValues.length; i++) {
		iSize += cxx::io::binaryWireFormat::SizeofUint32(cValues[i]);
	}

	/** Account for the array size value. */
	iSize += cxx::io::binaryWireFormat::SizeofUint32(cValues.length);

	return iSize;
}

/**
 * setUint32ArrayField
 * Sets the encoded valuec and header/id to the internal buffer.
 * @param iFieldNum - the field ID for this value.
 * @param cValue [in] - reference to the values to be written
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::writer::
setUint32ArrayField(uinttype                    iFieldNum,
		    cxx::lang::array<uinttype>& cValues) {

	if (fields::FIELD_MAX <= iFieldNum) {
		return -1;
	}

	uinttype iSize = sizeOfUint32Array(cValues);
	if (m_iBufferLen < iSize) {
		return -1;
	}

	uinttype iHdr = fields::makeHeader(types::arrayType(types::UINT32),
					   iFieldNum);

	uinttype iWritten =
		cxx::io::binaryWireFormat::WriteFixed32<uinttype>(m_piPos,
								  m_iBufferLen,
								  iHdr);
	if (0 == iWritten) {
		return -1;
	}

	WRITE_SPINS(iWritten);

	/** Write the array size */
	iWritten =
		cxx::io::binaryWireFormat::WriteUint32ToBuffer(m_piPos,
							       m_iBufferLen,
							       cValues.length);
	WRITE_SPINS(iWritten);

	/** Now write the array values. */
	for (inttype i=0; i<cValues.length; i++) {
		iWritten =
			cxx::io::binaryWireFormat::WriteUint32ToBuffer(m_piPos,
								       m_iBufferLen,
								       cValues[i]);
		if (0 == iWritten) {
			return -1;
		}

		WRITE_SPINS(iWritten);
	}

	return 0;
}

/** reader */

/**
 * reader
 * This constructor, takes the provided message buffer.
 * @param piBuffer [in] - the buffer to parse.
 * @param iBufferLen [in] - the size of the buffer.
 */
FORCE_INLINE cxx::io::encodeProtocol::reader::
reader(ubytetype* piBuffer,
       uinttype   iBufferLen,
       boolean    bCopyAll) : m_piBuffer(piBuffer),
			      m_iBufferLen(iBufferLen),
			      m_iMsgId(0),
			      m_iFlags(0),
			      m_bCopyAll(bCopyAll) {

	for (uinttype i=0; i<fields::FIELD_MAX; i++) {
		m_iFields[i].m_cValues.pArray = 0;
	}
}

/**
 * reader
 * This constructor, takes the provided message buffer.
 * @param piBuffer [in] - the buffer to parse.
 * @param iBufferLen [in] - the size of the buffer.
 */
FORCE_INLINE cxx::io::encodeProtocol::reader::
reader(const reader& cReader) : m_piBuffer(0),
				m_iBufferLen(cReader.m_iBufferLen),
				m_iMsgId(0),
				m_iFlags(0),
				m_bCopyAll(true) {

	for (uinttype i=0; i<fields::FIELD_MAX; i++) {
		m_iFields[i].m_iType       = cReader.m_iFields[i].m_iType;
		m_iFields[i].m_iEncodedLen = cReader.m_iFields[i].m_iEncodedLen;

		switch (cReader.m_iFields[i].m_iType) {
		case types::INT32:
			m_iFields[i].m_cValues.i32 = cReader.m_iFields[i].m_cValues.i32;
			break;
		case types::UINT32:
			m_iFields[i].m_cValues.ui32 = cReader.m_iFields[i].m_cValues.ui32;
			break;
		case types::INT64:
			m_iFields[i].m_cValues.i64 = cReader.m_iFields[i].m_cValues.i64;
			break;
		case types::UINT64:
			m_iFields[i].m_cValues.ui64 = cReader.m_iFields[i].m_cValues.ui64;
			break;
		case types::FLOAT:
			m_iFields[i].m_cValues.f = cReader.m_iFields[i].m_cValues.f;
			break;
		case types::DOUBLE:
			m_iFields[i].m_cValues.d = cReader.m_iFields[i].m_cValues.d;
			break;
		case types::UINT32_ARRAY:
			{
				cxx::lang::array<uinttype>* pcArray =
					new cxx::lang::array<uinttype>(cReader.m_iFields[i].m_iEncodedLen);
				if (0 == pcArray) {
					throw EncodeProtocolException("Unable to allocate an array.");
				}

				m_iFields[i].m_cValues.pArray = reinterpret_cast<void* >(pcArray);

				cxx::lang::array<uinttype>* pcOld =
					reinterpret_cast<cxx::lang::array<uinttype>* >(cReader.m_iFields[i].m_cValues.pArray);
				
				for (uinttype i=0; i<cReader.m_iFields[i].m_iEncodedLen; i++) {
					pcArray->operator[](i) = pcOld->operator[](i);
				}
			}
			break;
		case types::RAW:
		case types::STRING:
		case types::BUNDLE:
			{
				m_iFields[i].m_cValues.piRaw = new ubytetype[cReader.m_iFields[i].m_iEncodedLen];
				memcpy(m_iFields[i].m_cValues.piRaw,
				       cReader.m_iFields[i].m_cValues.piRaw,
				       cReader.m_iFields[i].m_iEncodedLen);
			}
			break;
		default:
			break;
		}
	}
}

/**
 * reader
 * This constructor creates a reader of the message ID provided.
 * The internal buffer is not allocated.
 * @param iMsgId - The message ID to allocate.
 */
FORCE_INLINE cxx::io::encodeProtocol::reader::
reader(uinttype iMsgId)  : m_piBuffer(0),
			   m_iBufferLen(0),
			   m_iMsgId(iMsgId),
			   m_iFlags(header::FIRST_FLAG | header::LAST_FLAG),
			   m_bCopyAll(true) {

	for (uinttype i=0; i<fields::FIELD_MAX; i++) {
		m_iFields[i].m_cValues.pArray = 0;
	}
}

/**
 * decodeFields
 * Decodes the header and payload sections pulling out
 * the fields. String/raw fields are references into the payload.
 * This is to minimize copies of large data sections.
 * The descrete types are decoded into storage variables.
 * @return 0 on success, else failure.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
decodeFields() {
	if ((0 == m_piBuffer) || (0 == m_iBufferLen)) {
		/** Nothing to do. */
		return 0;
	}

	ubytetype* piPos      = m_piBuffer;
	uinttype   iLen       = m_iBufferLen;
	uinttype   iHeaderVal = 0;

	/** MAGIC Number */
	uinttype iRead      =
		cxx::io::binaryWireFormat::ReadFixed32<uinttype>(piPos,
								 iLen,
								 iHeaderVal);
	if ((0 == iRead) || (header::DEEP_MAGIC != iHeaderVal)) {
		abort();
	}

	piPos += iRead;
	iLen  -= iRead;

	/** Version */
	iRead = cxx::io::binaryWireFormat::ReadFixed32<uinttype>(piPos,
								 iLen,
								 iHeaderVal);
	if ((0 == iRead) || (header::VERSION != (header::VERSION & iHeaderVal))) {
		abort();
	}

	m_iFlags = iHeaderVal & 0x0000ff00;

	piPos += iRead;
	iLen  -= iRead;

	/** Message ID */
	iRead = cxx::io::binaryWireFormat::ReadFixed32<uinttype>(piPos,
								 iLen,
								 m_iMsgId);
	if (0 == iRead) {
		abort();
	}

	piPos += iRead;
	iLen  -= iRead;

	/** Payload Length in Bytes */
	uinttype iPayloadLen = 0;

	iRead = cxx::io::binaryWireFormat::ReadFixed32<uinttype>(piPos,
								 iLen,
								 iPayloadLen);
	if (0 == iRead) {
		abort();
	}

	if ((iPayloadLen + header::HEADER_SIZE) > m_iBufferLen) {
		abort();
	} else {
		m_iBufferLen = iPayloadLen + header::HEADER_SIZE;
	}

	piPos += iRead;
	iLen   = iPayloadLen;

	/** Start processing the fields in the payload */
	while (0 < iLen) {
		/** Read the field header */
		uinttype iFieldHdr = 0;

		iRead = cxx::io::binaryWireFormat::ReadFixed32<uinttype>(piPos,
									 iLen,
									 iFieldHdr);
		if (0 == iRead) {
			abort();
		}

		piPos += iRead;
		iLen  -= iRead;

		ubytetype  iType = 0;
		uinttype iId   = 0;
		fields::getTypeId(iFieldHdr, iType, iId);

		if (fields::FIELD_MAX <= iId) {
			return -2;
		}

		switch (iType) {
			case types::INT32:
			{
				inttype iVal = 0;
				iRead = cxx::io::binaryWireFormat::
					ReadInt32FromBuffer(piPos,
							    iLen,
							    iVal);
				m_iFields[iId].m_iType       = iType;
				m_iFields[iId].m_cValues.i32 = iVal;
				m_iFields[iId].m_iEncodedLen = iRead;
			}
			break;

			case types::UINT32:
			{
				uinttype iVal = 0;
				iRead = cxx::io::binaryWireFormat::
					ReadUint32FromBuffer(piPos,
							     iLen,
							     iVal);
				m_iFields[iId].m_iType        = iType;
				m_iFields[iId].m_cValues.ui32 = iVal;
				m_iFields[iId].m_iEncodedLen  = iRead;
			}
			break;

			case types::UINT32_ARRAY:
			{
				uinttype   iArrayRead = 0;
				uinttype   iCount     = 0;
				ubytetype* piArrayPos = piPos;
				uinttype   iLen2      = iLen;

				m_iFields[iId].m_iType          = iType;
				m_iFields[iId].m_iEncodedLen    = 0;

				/** Read the array length */
				iArrayRead = cxx::io::binaryWireFormat::
					ReadUint32FromBuffer(piArrayPos,
							     iLen2,
							     iCount);

				if (0 != iArrayRead) {
					iLen2      -= iArrayRead;
					piArrayPos += iArrayRead;

					cxx::lang::array<uinttype>* pcArray =
						reinterpret_cast<cxx::lang::array<uinttype>* >(m_iFields[iId].m_cValues.pArray);

					if (0 != pcArray) {
						delete pcArray;
					}

					m_iFields[iId].m_iEncodedLen = iCount;

					pcArray = new cxx::lang::array<uinttype>(iCount);
					if (0 == pcArray) {
						throw EncodeProtocolException("Unable to allocate an array.");
					}

					m_iFields[iId].m_cValues.pArray = reinterpret_cast<void* >(pcArray);

					for (uinttype i=0; i<iCount; i++) {
						uinttype iValue = 0;

						iArrayRead = cxx::io::binaryWireFormat::
							ReadUint32FromBuffer(piArrayPos,
									     iLen2,
									     iValue);
						if (0 == iArrayRead) {
							break;
						}

						pcArray->operator[](i) = iValue;

						iLen2      -= iArrayRead;
						piArrayPos += iArrayRead;
					}
				}

				if (0 != iArrayRead) {
					iRead = iLen - iLen2;
				}
			}
			break;

			case types::INT64:
			{
				longtype iVal = 0;
				iRead = cxx::io::binaryWireFormat::
					ReadInt64FromBuffer(piPos,
							    iLen,
							    iVal);
				m_iFields[iId].m_iType       = iType;
				m_iFields[iId].m_cValues.i64 = iVal;
				m_iFields[iId].m_iEncodedLen = iRead;
			}
			break;

			case types::UINT64:
			{
				ulongtype iVal = 0;
				iRead = cxx::io::binaryWireFormat::
					ReadUint64FromBuffer(piPos,
							     iLen,
							     iVal);
				m_iFields[iId].m_iType        = iType;
				m_iFields[iId].m_cValues.ui64 = iVal;
				m_iFields[iId].m_iEncodedLen  = iRead;
			}
			break;

			case types::FLOAT:
			{
				floattype fVal = 0;
				iRead = cxx::io::binaryWireFormat::
					ReadFloatFromBuffer(piPos,
							    iLen,
							    fVal);
				m_iFields[iId].m_iType       = iType;
				m_iFields[iId].m_cValues.f   = fVal;
				m_iFields[iId].m_iEncodedLen = iRead;
			}
			break;

			case types::DOUBLE:
			{
				doubletype fVal = 0;
				iRead = cxx::io::binaryWireFormat::
					ReadDoubleFromBuffer(piPos,
							     iLen,
							     fVal);
				m_iFields[iId].m_iType       = iType;
				m_iFields[iId].m_cValues.d   = fVal;
				m_iFields[iId].m_iEncodedLen = iRead;
			}
			break;

			case types::RAW:
			case types::STRING:
			{
				uinttype iRawLen = 0;
				iRead = cxx::io::binaryWireFormat::
					ReadFixed32<uinttype>(piPos,
							      iLen,
							      iRawLen);

				if (0 == iRead) {
					return -4;
				}

				piPos += iRead;
				iLen  -= iRead;

				m_iFields[iId].m_iType         = iType;
				m_iFields[iId].m_iEncodedLen   = iRawLen;

				if (true == m_bCopyAll) {
					m_iFields[iId].m_cValues.piRaw = new ubytetype[iRawLen];
					memcpy(m_iFields[iId].m_cValues.piRaw,
					       piPos,
					       iRawLen);
				} else {
					m_iFields[iId].m_cValues.piRaw = piPos;
				}

				iRead = iRawLen;
			}
			break;

			case types::BUNDLE:
			{
				if (fields::FIELD_BUNDLE_ID != iId) {
					return -2;
				}

				if ((header::HEADER_SIZE + fields::FIELD_HDR_SIZE) != (piPos - m_piBuffer)) {
					return -4;
				}

				m_iFields[iId].m_iType         = iType;
				m_iFields[iId].m_cValues.piRaw = piPos;
				m_iFields[iId].m_iEncodedLen   = iLen;

				iRead = iLen;
			}
			break;

			default:
			{
				/** No a currently supported type. */
				return -1;
			}
		}

		if ((0 == iRead) && (types::RAW != iType) && (types::STRING != iType)) {
			/** Failure in reading from the buffer */
			return -5;
		}

		piPos += iRead;
		iLen  -= iRead;
	}

	return 0;
}


/**
 * ~reader
 * The provider of the buffer is responsible for deleting it.
 */
FORCE_INLINE cxx::io::encodeProtocol::reader::
~reader() {
	m_piBuffer   = 0;
	m_iBufferLen = 0;
	m_iMsgId     = 0;

	if (true == m_bCopyAll) {
		for (uinttype i=0; i<fields::FIELD_MAX; i++) {
			switch (m_iFields[i].m_iType) {
			case types::RAW:
			case types::STRING:
			case types::BUNDLE:
				if (0 != m_iFields[i].m_cValues.piRaw) {
					delete [] m_iFields[i].m_cValues.piRaw;
					m_iFields[i].m_cValues.piRaw = 0;
				}
				break;
			case types::UINT32_ARRAY:
				if (0 != m_iFields[i].m_cValues.pArray) {
					cxx::lang::array<uinttype>* pcArray =
						reinterpret_cast<cxx::lang::array<uinttype>* >(m_iFields[i].m_cValues.pArray);
					delete pcArray;
					m_iFields[i].m_cValues.pArray = 0;
				}
			}
		}
	}

	m_bCopyAll = false;
}

/**
 * getMsgId
 * Returns the message ID encoded in the header.
 * @return the message ID.
 */
FORCE_INLINE const uinttype cxx::io::encodeProtocol::reader::getMsgId() {
	return m_iMsgId;
}

/**
 * isReply
 * Returns true if this is a REP to a REQ message.
 * @return true if a REP, else false.
 */
FORCE_INLINE const boolean cxx::io::encodeProtocol::reader::isReply() {
	return (header::REPLY_FLAG & m_iFlags) ? true : false;
}

/**
 * isFirst
 * Returns true if this is a first in the sequence of bundled REQ messages.
 * @return true if a REP, else false.
 */
FORCE_INLINE const boolean cxx::io::encodeProtocol::reader::isFirst() {
	return (header::FIRST_FLAG & m_iFlags) ? true : false;
}

/**
 * isLast
 * Returns true if this is the last in the sequence of bundled REQ messages.
 * @return true if a REP, else false.
 */
FORCE_INLINE const boolean cxx::io::encodeProtocol::reader::isLast() {
	return (header::LAST_FLAG & m_iFlags) ? true : false;
}

/**
 * toBlock
 * Returns true if the BLOCK_FLAG is set.
 * @returns trueif the BLOCK_FLAG is set, else false.
 */
FORCE_INLINE const boolean cxx::io::encodeProtocol::reader::toBlock() {
	return (header::BLOCK_FLAG & m_iFlags) ? true : false;
}

/**
 * getInt32Field
 * Returns the value at the specified field.
 * @param iId [in] - the field ID
 * @param iVal [out] - the value to return.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getInt32Field(uinttype iId, inttype& iVal) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::INT32 != m_iFields[iId].m_iType) {
		return -2;
	}

	if (0 == m_iFields[iId].m_iEncodedLen) {
		return -3;
	}

	iVal = m_iFields[iId].m_cValues.i32;

	return 0;
}

/**
 * getUint32Field
 * Returns the value at the specified field.
 * @param iId [in] - the field ID
 * @param iVal [out] - the value to return.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getUint32Field(uinttype iId, uinttype& iVal) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::UINT32 != m_iFields[iId].m_iType) {
		return -2;
	}

	if (0 == m_iFields[iId].m_iEncodedLen) {
		return -3;
	}

	iVal = m_iFields[iId].m_cValues.ui32;

	return 0;
}

/**
 * getInt64Field
 * Returns the value at the specified field.
 * @param iId [in] - the field ID
 * @param iVal [out] - the value to return.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getInt64Field(uinttype iId, longtype& iVal) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::INT64 != m_iFields[iId].m_iType) {
		return -2;
	}

	if (0 == m_iFields[iId].m_iEncodedLen) {
		return -3;
	}

	iVal = m_iFields[iId].m_cValues.i64;

	return 0;
}

/**
 * getUint64Field
 * Returns the value at the specified field.
 * @param iId [in] - the field ID
 * @param iVal [out] - the value to return.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getUint64Field(uinttype iId, ulongtype& iVal) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::UINT64 != m_iFields[iId].m_iType) {
		return -2;
	}

	if (0 == m_iFields[iId].m_iEncodedLen) {
		return -3;
	}

	iVal = m_iFields[iId].m_cValues.ui64;

	return 0;
}

/**
 * getFloatField
 * Returns the value at the specified field.
 * @param iId [in] - the field ID
 * @param iVal [out] - the value to return.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getFloatField(uinttype iId, floattype& fVal) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::FLOAT != m_iFields[iId].m_iType) {
		return -2;
	}

	if (0 == m_iFields[iId].m_iEncodedLen) {
		return -3;
	}

	fVal = m_iFields[iId].m_cValues.f;

	return 0;
}

/**
 * getDoubleField
 * Returns the value at the specified field.
 * @param iId [in] - the field ID
 * @param iVal [out] - the value to return.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getDoubleField(uinttype iId, doubletype& fVal) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::DOUBLE != m_iFields[iId].m_iType) {
		return -2;
	}

	if (0 == m_iFields[iId].m_iEncodedLen) {
		return -3;
	}

	fVal = m_iFields[iId].m_cValues.d;

	return 0;
}

/**
 * getStringField
 * Returns the value at the specified field.
 * @param iId [in] - the field ID
 * @param cStr [out] - the value to return.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getStringField(uinttype iId, cxx::lang::String& cStr) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::STRING != m_iFields[iId].m_iType) {
		return -2;
	}

	if (0 == m_iFields[iId].m_iEncodedLen) {
		cStr = "";
	} else {
		cStr.assign(reinterpret_cast<const char*>(m_iFields[iId].m_cValues.piRaw),
			    m_iFields[iId].m_iEncodedLen);
	}

	return 0;
}

/**
 * getRawFieldRef
 * Returns the value at the specified field.
 * @param iId [in] - the field ID
 * @param pcRaw [out] - the raw value to return.
 * @param iSize [out] - the size of the raw value.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getRawFieldRef(uinttype iId, const ubytetype*& pcRaw, uinttype& iSize) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::RAW != m_iFields[iId].m_iType) {
		return -2;
	}

	if (0 == m_iFields[iId].m_iEncodedLen) {
		pcRaw = 0;
	} else {
		pcRaw = static_cast<const ubytetype*>(m_iFields[iId].m_cValues.piRaw);
	}

	iSize = m_iFields[iId].m_iEncodedLen;

	return 0;
}

/**
 * getStringFieldRef
 * Returns the value at the specified field.
 * @param iId [in] - the field ID
 * @param pcStr [out] - the string pointer to return. Not zero terminated.
 * @param iSize [out] - the number of characters in the string.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getStringFieldRef(uinttype iId, const char*& pcStr, uinttype& iSize) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::STRING != m_iFields[iId].m_iType) {
		return -2;
	}

	if (0 == m_iFields[iId].m_iEncodedLen) {
		pcStr = 0;
	} else {
		pcStr = reinterpret_cast<const char*>(m_iFields[iId].m_cValues.piRaw);
	}

	iSize = m_iFields[iId].m_iEncodedLen;

	return 0;
}

/**
 * getUint32ArrayField
 * This returns an array allocated by the reader. This is a onetime
 * call, as the caller then owns the array and therefore responsible
 * for deleting it after use.
 * @param iId [in] - the field ID
 * @param pcValues [out] - reference of the array data.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getUint32ArrayField(uinttype                     iId,
		    cxx::lang::array<uinttype>*& pcValues) {

	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::UINT32_ARRAY != m_iFields[iId].m_iType) {
		return -2;
	}

	pcValues = reinterpret_cast<cxx::lang::array<uinttype>*>(m_iFields[iId].m_cValues.pArray);

	m_iFields[iId].m_cValues.pArray = 0;
	m_iFields[iId].m_iEncodedLen    = 0;

	return 0;
}

/**
 * getBundleBuffer
 * Returns a const reference to the bundle buffer.
 * @param pBuffer [out] - the raw value to return.
 * @param iSize [out] - the size of the raw value.
 * @return 0 on success, -1 if an invalid ID, -2 if a type mismatch.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getBundleBuffer(const ubytetype*& pBuffer, uinttype& iSize) {
	if (types::BUNDLE != m_iFields[fields::FIELD_BUNDLE_ID].m_iType) {
		return -2;
	}

	if (0 == m_iFields[fields::FIELD_BUNDLE_ID].m_iEncodedLen) {
		pBuffer = 0;
	} else {
		pBuffer = static_cast<const ubytetype*>(m_iFields[fields::FIELD_BUNDLE_ID].m_cValues.piRaw);
	}

	iSize = m_iFields[fields::FIELD_BUNDLE_ID].m_iEncodedLen;

	return 0;
}

/**
 * getVoidPtr
 * Returns a voidptr encoded by a (renderReader).
 * @param iId [in] - the field ID
 * @param pPtr [out] - reference of the pointer value.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::reader::
getVoidPtr(uinttype iId, voidptr& pPtr) {
	if (fields::FIELD_MAX <= iId) {
		return -1;
	}

	if (types::VOIDPTR != m_iFields[iId].m_iType) {
		return -2;
	}

	pPtr = m_iFields[iId].m_cValues.pArray;

	return 0;
}

/**
 * getEncodedLength
 * Returns the number of bytes that encode for this message object.
 * @return the number of bytes.
 */
FORCE_INLINE uinttype cxx::io::encodeProtocol::reader::
getEncodedLength() {
	return m_iBufferLen;
}

/** renderReader */

/**
 * renderReader
 * Constructor, internally creates a reader with the
 * supplied message id. Message is also flagged as
 * FIRST | LAST.
 * @param iMsgId - Message ID.
 */
FORCE_INLINE cxx::io::encodeProtocol::
renderReader::renderReader(uinttype iMsgId) :
	m_pcReader(new reader(iMsgId)) {
}

/**
 * ~renderReader
 * Destructor, this will delete the internal
 * reader object if it has not been obtained by mutableReader.
 */
FORCE_INLINE cxx::io::encodeProtocol::
renderReader::~renderReader() {
	if (0 == m_pcReader) {
		delete m_pcReader; m_pcReader = 0;
	}
}

/**
 * mutableReader
 * Returns the reader object and the renderReader will not be the
 * owner of the reader object. The caller obtains ownership.
 * @return reader* - reference to a reader object or NULL.
 */
FORCE_INLINE cxx::io::encodeProtocol::reader* cxx::io::encodeProtocol::
renderReader::mutableReader() {
	cxx::io::encodeProtocol::reader* pcRetVal = m_pcReader;
	m_pcReader = 0;

	return pcRetVal;
}

/**
 * setInt32Field
 * Set the value for the given field number.
 * @param iFieldNum - field number to set.
 * @param iValue - value to set in the field.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::
renderReader::setInt32Field(uinttype iFieldNum,
			    inttype  iValue) {
	if (0 == m_pcReader) {
		return -1;
	}

	if (fields::FIELD_MAX <= iFieldNum) {
		return -2;
	}

	m_pcReader->m_iFields[iFieldNum].m_iType       = types::INT32;
	m_pcReader->m_iFields[iFieldNum].m_cValues.i32 = iValue;
	m_pcReader->m_iFields[iFieldNum].m_iEncodedLen = sizeof(iValue);

	return 0;
}

/**
 * setUint32Field
 * Set the value for the given field number.
 * @param iFieldNum - field number to set.
 * @param iValue - value to set in the field.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::
renderReader::setUint32Field(uinttype iFieldNum,
			     uinttype iValue) {
	if (0 == m_pcReader) {
		return -1;
	}

	if (fields::FIELD_MAX <= iFieldNum) {
		return -2;
	}

	m_pcReader->m_iFields[iFieldNum].m_iType        = types::UINT32;
	m_pcReader->m_iFields[iFieldNum].m_cValues.ui32 = iValue;
	m_pcReader->m_iFields[iFieldNum].m_iEncodedLen  = sizeof(iValue);

	return 0;
}

/**
 * setInt64Field
 * Set the value for the given field number.
 * @param iFieldNum - field number to set.
 * @param iValue - value to set in the field.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::
renderReader::setInt64Field(uinttype iFieldNum,
			    longtype  iValue) {
	if (0 == m_pcReader) {
		return -1;
	}

	if (fields::FIELD_MAX <= iFieldNum) {
		return -2;
	}

	m_pcReader->m_iFields[iFieldNum].m_iType       = types::INT64;
	m_pcReader->m_iFields[iFieldNum].m_cValues.i64 = iValue;
	m_pcReader->m_iFields[iFieldNum].m_iEncodedLen = sizeof(iValue);

	return 0;
}

/**
 * setUint64Field
 * Set the value for the given field number.
 * @param iFieldNum - field number to set.
 * @param iValue - value to set in the field.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::
renderReader::setUint64Field(uinttype iFieldNum,
			     ulongtype iValue) {
	if (0 == m_pcReader) {
		return -1;
	}

	if (fields::FIELD_MAX <= iFieldNum) {
		return -2;
	}

	m_pcReader->m_iFields[iFieldNum].m_iType        = types::UINT64;
	m_pcReader->m_iFields[iFieldNum].m_cValues.ui64 = iValue;
	m_pcReader->m_iFields[iFieldNum].m_iEncodedLen  = sizeof(iValue);

	return 0;
}

/**
 * setFloatField
 * Set the value for the given field number.
 * @param iFieldNum - field number to set.
 * @param iValue - value to set in the field.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::
renderReader::setFloatField(uinttype iFieldNum,
			    float    fValue) {
	if (0 == m_pcReader) {
		return -1;
	}

	if (fields::FIELD_MAX <= iFieldNum) {
		return -2;
	}

	m_pcReader->m_iFields[iFieldNum].m_iType       = types::FLOAT;
	m_pcReader->m_iFields[iFieldNum].m_cValues.f   = fValue;
	m_pcReader->m_iFields[iFieldNum].m_iEncodedLen = sizeof(fValue);

	return 0;
}

/**
 * setDoubleField
 * Set the value for the given field number.
 * @param iFieldNum - field number to set.
 * @param iValue - value to set in the field.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::
renderReader::setDoubleField(uinttype iFieldNum,
			     double   fValue) {
	if (0 == m_pcReader) {
		return -1;
	}

	if (fields::FIELD_MAX <= iFieldNum) {
		return -2;
	}

	m_pcReader->m_iFields[iFieldNum].m_iType       = types::DOUBLE;
	m_pcReader->m_iFields[iFieldNum].m_cValues.d   = fValue;
	m_pcReader->m_iFields[iFieldNum].m_iEncodedLen = sizeof(fValue);

	return 0;
}

/**
 * setStringField
 * Set the value for the given field number.
 * @param iFieldNum - field number to set.
 * @param iValue - value to set in the field.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::
renderReader::setStringField(uinttype           iFieldNum,
			     cxx::lang::String& cValue) {
	inttype iRetVal = setRawField(iFieldNum,
				      reinterpret_cast<const ubytetype* >( cValue.c_str()),
				      cValue.size());

	m_pcReader->m_iFields[iFieldNum].m_iType = types::STRING;
	return iRetVal;
}

/**
 * setRawField
 * Set the value for the given field number.
 * @param iFieldNum - field number to set.
 * @param piRaw - the binary data to set.
 * @param iRawLen - the length of the data.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::
renderReader::setRawField(uinttype         iFieldNum,
			  const ubytetype* piRaw,
			  const uinttype   iRawLen) {
	if (0 == m_pcReader) {
		return -1;
	}

	if (fields::FIELD_MAX <= iFieldNum) {
		return -2;
	}

	m_pcReader->m_iFields[iFieldNum].m_iType         = types::RAW;
	m_pcReader->m_iFields[iFieldNum].m_cValues.piRaw = new ubytetype[iRawLen];
	m_pcReader->m_iFields[iFieldNum].m_iEncodedLen   = iRawLen;

	memcpy(m_pcReader->m_iFields[iFieldNum].m_cValues.piRaw,
	       piRaw,
	       iRawLen);

	return 0;
}

/**
 * setStringField
 * Set the value for the given field number.
 * @param iFieldNum - field number to set.
 * @param iValue - value to set in the field.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::encodeProtocol::
renderReader::setVoidPtr(uinttype iFieldNum,
			 voidptr  pPtr) {
	if (0 == m_pcReader) {
		return -1;
	}

	if (fields::FIELD_MAX <= iFieldNum) {
		return -2;
	}

	m_pcReader->m_iFields[iFieldNum].m_iType          = types::VOIDPTR;
	m_pcReader->m_iFields[iFieldNum].m_cValues.pArray = pPtr;
	m_pcReader->m_iFields[iFieldNum].m_iEncodedLen    = sizeof(pPtr);

	return 0;
}

#include "cxx/io/MsgBundle.h"

#endif /** CXX_LANG_IO_ENCODEPROTOCOL_H_ */
