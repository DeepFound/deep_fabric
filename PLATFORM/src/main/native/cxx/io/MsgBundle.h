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
#ifndef CXX_LANG_IO_MSGBUNDLE_H_
#define CXX_LANG_IO_MSGBUNDLE_H_

#include <errno.h>

namespace cxx { namespace io { namespace MsgBundle {

/**
 * Exceptions to handle errors beyond the LinkManager API
 */
class MsgBundleException : public Exception {
public:
	MsgBundleException(void) {
	}

	MsgBundleException(const char* message)
		: Exception(message) {
	}
}; /** MsgBundleException */

static const uinttype MSG_ID_PREFIX = 0x00020000;

class writer : public cxx::lang::Object {
public:
	writer();
	virtual ~writer();

	uinttype remainingSpace();

	cxx::io::encodeProtocol::writer* reserve(const uinttype iBufferLen,
						 const uinttype iMsgId);

	inttype append(cxx::io::encodeProtocol::writer*& pcMsg);

	inttype finalizeReservation(cxx::io::encodeProtocol::writer*& pcWriter);

	inttype open();
	inttype close(cxx::io::encodeProtocol::writer*& pcWriter,
		      const uinttype                    iMsgId);

private:

	static const uinttype BUNDLE_SIZE     = 4096 << 2;
	static const uinttype BUNDLE_PREAMBLE = \
		cxx::io::encodeProtocol::header::HEADER_SIZE +	\
		cxx::io::encodeProtocol::fields::FIELD_HDR_SIZE;

	ubytetype* m_piBuffer;
	uinttype   m_iBufferLen;
	uinttype   m_iWritten;

}; /** writer */

class reader : public cxx::lang::Object {
public:
	reader(const ubytetype* pBuffer, uinttype iSize);
	virtual ~reader();

	cxx::io::encodeProtocol::reader* getNextMsg();

private:
	const ubytetype* m_pBuffer;
	ubytetype*       m_piPos;
	uinttype         m_iSize;

}; /** reader */

} /** MsgBundle */

} /** io */
} /** cxx */

/** MsgBundle::writer */

/**
 * writer
 * Construct a bunder writer.
 */
FORCE_INLINE cxx::io::MsgBundle::
writer::writer() :
	m_piBuffer(0),
	m_iBufferLen(BUNDLE_SIZE),
	m_iWritten(0) {

	m_piBuffer = new ubytetype[BUNDLE_SIZE];
	if (0 == m_piBuffer) {
		throw MsgBundleException("Unable to allocate the BUNDLE Buffer.");
	}

	/**
	 * Reserve space to shim the data with a message header and a
	 * field header for the encapsulated data.
	 */
	m_iWritten = BUNDLE_PREAMBLE;
}

/**
 * ~writer
 * Destructor
 */
FORCE_INLINE cxx::io::MsgBundle::
writer::~writer() {
	if (0 != m_piBuffer) {
		delete [] m_piBuffer;
		m_piBuffer = 0;
	}

	m_iBufferLen = 0;
	m_iWritten   = 0;
}

/**
 * reserve
 * This reserves a block from the bundle buffer enough to encode a
 * message block. A message ID is required and the size of the data
 * that allows a writer object to be returned.
 * @param iBufferLen - Length of the encoded message to write.
 * @param iMsgId - The message ID.
 * @return 0 on success, can throw exceptions on errors.
 */
FORCE_INLINE cxx::io::encodeProtocol::writer* cxx::io::MsgBundle::
writer::reserve(const uinttype iBufferLen,
		const uinttype iMsgId) {
	const uinttype iRemain  = m_iBufferLen - m_iWritten;
	const uinttype iReserve = iBufferLen + cxx::io::encodeProtocol::header::HEADER_SIZE;

	if (iRemain < iReserve) {
		throw MsgBundleException("Reservation exceeds remaining buffer size.");
	}

	ubytetype* piPos = &m_piBuffer[m_iWritten];
	if (0 == piPos) {
		throw MsgBundleException("Internal buffer error.");
	}

	cxx::io::encodeProtocol::writer* pcWriter = new cxx::io::encodeProtocol::writer(piPos,
											iReserve,
											iBufferLen,
											iMsgId);
	if (0 == pcWriter) {
		throw MsgBundleException("Unable to allocate a writer.");
	}

	return pcWriter;
}

/**
 * append
 * Append the writer buffer to the bundle buffer. Will throw
 * exceptions if the sizing exceeds the remaining bundle buffer size.
 * @param pcMsg - The writer to append. Will invalidate this writer on
 * success.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::MsgBundle::
writer::append(cxx::io::encodeProtocol::writer*& pcMsg) {

	ubytetype* piBuffer   = 0;
	uinttype   iBufferLen = 0;

	if (0 != pcMsg->getMutableBuffer(piBuffer, iBufferLen)) {
		throw MsgBundleException("Unable to extract buffer from the supplied writer.");
	}

	const uinttype iRemain  = m_iBufferLen - m_iWritten;

	if (iRemain < iBufferLen) {
		throw MsgBundleException("Reservation exceeds remaining buffer size.");
	}

	ubytetype* piPos = &m_piBuffer[m_iWritten];
	if (0 == piPos) {
		throw MsgBundleException("Internal buffer error.");
	}

	memcpy(piPos, piBuffer, iBufferLen);
	m_iWritten += iBufferLen;

	delete pcMsg; pcMsg = 0;
	delete [] piBuffer; piBuffer = 0;

	return 0;
}

/**
 * finalizeReservation
 * This will advance the internal postion of the bundle. The result is
 * the encodeProtocol writer will be deleted.
 * @param pcWriter [in/out] - a reference to the writer returned by
 * reserve.
 * @return 0 on success, can throw exceptions on errors.
 */
FORCE_INLINE inttype cxx::io::MsgBundle::
writer::finalizeReservation(cxx::io::encodeProtocol::writer*& pcWriter) {
	ubytetype* piBuffer   = 0;
	uinttype   iBufferLen = 0;

	if (0 != pcWriter->getMutableBuffer(piBuffer, iBufferLen)) {
		return -1;
	}

	m_iWritten += iBufferLen;
	if (m_iWritten > m_iBufferLen) {
		throw MsgBundleException("Finalize operation caused buffer overrun.");
	}

	delete pcWriter; pcWriter = 0;

	return 0;
}

/**
 * remainingSpace
 * Returns the remaining space in the bundle buffer. Call this before
 * reserve to ensure an out of space exception is not thrown.
 * @return byte count of the remaining space.
 */
FORCE_INLINE uinttype cxx::io::MsgBundle::
writer::remainingSpace() {
	if (m_iWritten > m_iBufferLen) {
		throw MsgBundleException("Remaining space detected a buffer error.");
	}

	return (m_iBufferLen - m_iWritten);
}

/**
 * open
 * (re)Opens a writer for more bundling. If called after close it will
 * create a new buffer, otherwise we restart the bundle and the old
 * data is overwritten.
 * @return 0 on success, can throw exceptions on errors.
 */
FORCE_INLINE inttype cxx::io::MsgBundle::
writer::open() {
	if (0 == m_piBuffer) {
		m_piBuffer = new ubytetype[BUNDLE_SIZE];
		if (0 == m_piBuffer) {
			throw MsgBundleException("Unable to allocate the BUNDLE Buffer.");
		}
	}

	/**
	 * Reserve space to shim the data with a message header and a
	 * field header for the encapsulated data.
	 */
	m_iWritten   = BUNDLE_PREAMBLE;
	m_iBufferLen = BUNDLE_SIZE;

	return 0;
}

/**
 * close
 * The method closes out the bundle writer and returns a
 * reference to a encode protocol writer that can then be used to send
 * the bundle.
 * @param pcWriter [out] - returns encode protocol writer
 * instance. The caller is expected to deal with deleting this
 * instance.
 * @return 0 on success, can throw exceptions on errors.
 */
FORCE_INLINE inttype cxx::io::MsgBundle::
writer::close(cxx::io::encodeProtocol::writer*& pcWriter,
	      const uinttype                    iMsgId) {
	if (m_iWritten > m_iBufferLen) {
		throw MsgBundleException("Mutable buffer error discovered.");
	}

	if (0 == m_iWritten) {
		return -1;
	}

	if (cxx::io::encodeProtocol::header::HEADER_SIZE > m_iWritten) {
		throw MsgBundleException("Written length is incorrect.");
	}

	uinttype iPayloadLen = m_iWritten - cxx::io::encodeProtocol::header::HEADER_SIZE;

	pcWriter = new cxx::io::encodeProtocol::writer(m_piBuffer,
						       m_iBufferLen,
						       iPayloadLen,
						       iMsgId,
						       true);
	if (0 == pcWriter) {
		throw MsgBundleException("Unable to create a encode protocol writer.");
	}

	m_piBuffer   = 0;
	m_iBufferLen = 0;

	return 0;
}

/** MsgBundle::reader */

/**
 * reader
 * Bundle reader constructor
 * @param pBuffer - buffer with one or more messages to decode and
 * read.
 * @param iSize   - the complete length of this buffer
 */
FORCE_INLINE cxx::io::MsgBundle::
reader::reader(const ubytetype* pBuffer, uinttype iSize) :
	m_pBuffer(pBuffer),
	m_piPos(const_cast<ubytetype* >(pBuffer)),
	m_iSize(iSize) {
}

/**
 * ~reader
 * Destructor. The buffer is not owned by us so just null out.
 */

FORCE_INLINE cxx::io::MsgBundle::
reader::~reader() {
	m_pBuffer = 0;
	m_piPos   = 0;
	m_iSize   = 0;
}

/**
 * getNextMsg
 * Returns an instance of a encode protocol reader with the next
 * message fields. If there are no more messages, then NULL is
 * returned.
 * @return encode protocol reader instance with the next message
 * fields. If nothing to get, returns NULL.
 */
FORCE_INLINE cxx::io::encodeProtocol::reader* cxx::io::MsgBundle::
reader::getNextMsg() {
	if (0 == m_iSize) {
		return 0;
	}

	cxx::io::encodeProtocol::reader* pcReader = new cxx::io::encodeProtocol::reader(m_piPos, m_iSize, true);
	if (0 == pcReader) {
		throw MsgBundleException("Unable to create a encode protocol reader.");
	}

	if (0 != pcReader->decodeFields()) {
		throw MsgBundleException("Unable to decode the message.");
	}

	uinttype iSize = pcReader->getEncodedLength();

	if (iSize > m_iSize) {
		throw MsgBundleException("Bad Message payload length decoded.");
	}

	m_piPos += iSize;
	m_iSize -= iSize;

	return pcReader;
}

#endif /** CXX_LANG_IO_MSGBUNDLE_H_ */
