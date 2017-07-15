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
#include "cxx/io/BinaryWireFormat.h"
#include "cxx/util/Logger.h"
#include "cxx/util/PrettyPrint.h"
#include "cxx/util/Time.h"
#include "cxx/io/EncodeProtocol.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <limits.h>
#include <float.h>

using namespace cxx::lang;
using namespace cxx::util;

#define DEEP_INT32_MIN  (-DEEP_INT32_MAX - 1)
#define DEEP_INT32_MAX  (2147483647L)

#define DEEP_INT64_MIN  (-DEEP_INT64_MAX - 1)
#define DEEP_INT64_MAX  (9223372036854775807LL)

void testInt32WriteRead() {
	static const uinttype iSize  = 4096;
	static const inttype  iStart = 1024;

	ubytetype  iBuffer[iSize];
	ubytetype* piPtr = iBuffer;

	uinttype iNum   = 2048;
	inttype  iVal   = iStart;
	uinttype iBytes = 0;

	for (uinttype i=0; i<iNum; i++) {
		uinttype iWritten = cxx::io::binaryWireFormat::WriteInt32ToBuffer(piPtr,
										  iSize - iBytes,
										  iVal);
		if (0 == iWritten) {
			DEEP_LOG_ERROR(OTHER, "Unable to write the Int32\n");
		}

		iBytes += iWritten;
		piPtr  += iWritten;
		iVal--;
	}

	piPtr = iBuffer;

	inttype iFind = iStart;

	for (uinttype i=0; i<iNum; i++) {
		inttype  iVal  = 0;
		uinttype iRead = cxx::io::binaryWireFormat::ReadInt32FromBuffer(piPtr,
										iBytes,
										iVal);

		if (0 == iRead) {
			DEEP_LOG_ERROR(OTHER, "Unable to read in Int32\n");
			exit(-1);
		}
	
		iBytes -= iRead;
		piPtr  += iRead;

		if (iVal != iFind) {
			DEEP_LOG_ERROR(OTHER, "mismatch in values: expected %i, got %i\n", iFind, iVal);
			exit(-1);
		}

		//DEEP_LOG_DEBUG(OTHER, "INT32, found: %ld\n", iFind);

		iFind--;
	}

	DEEP_LOG_INFO(OTHER, "INT32: %i to %i sequence verified.\n", iStart, iFind);

	{
		inttype iCheck = DEEP_INT32_MAX;
		iVal = iCheck;

		uinttype iWritten = cxx::io::binaryWireFormat::WriteInt32ToBuffer(iBuffer,
										  iSize,
										  iVal);
		if (0 == iWritten) {
			DEEP_LOG_ERROR(OTHER, "Failed to write MAX INT32\n");
			exit(-1);
		}

		uinttype iRead = cxx::io::binaryWireFormat::ReadInt32FromBuffer(iBuffer,
										iSize,
										iVal);
		if (0 == iRead) {
			DEEP_LOG_ERROR(OTHER, "Failed to read MAX INT32\n");
			exit(-1);
		}

		if (iCheck != iVal) {
			DEEP_LOG_ERROR(OTHER, "Failed to read MAX INT32: %i not %i\n", iVal, iCheck);
			exit(-1);
		}

		DEEP_LOG_INFO(OTHER, "INT32 MAX verified: %i\n", iVal);
		
	}

	{
		inttype iCheck = DEEP_INT32_MIN;
		iVal = iCheck;

		uinttype iWritten = cxx::io::binaryWireFormat::WriteInt32ToBuffer(iBuffer,
										  iSize,
										  iVal);
		if (0 == iWritten) {
			DEEP_LOG_ERROR(OTHER, "Failed to write MIN INT32\n");
			exit(-1);
		}

		uinttype iRead = cxx::io::binaryWireFormat::ReadInt32FromBuffer(iBuffer,
										iSize,
										iVal);
		if (0 == iRead) {
			DEEP_LOG_ERROR(OTHER, "Failed to read MIN INT32\n");
			exit(-1);
		}

		if (iCheck != iVal) {
			DEEP_LOG_ERROR(OTHER, "Failed to read MIN INT32: %i not %i\n", iVal, iCheck);
			exit(-1);
		}

		DEEP_LOG_INFO(OTHER, "INT32 MIN verified: %i\n", iVal);
	}
}

void testInt64WriteRead() {
	static const uinttype iSize  = 4096;
	static const longtype iStart = 1024;

	ubytetype  iBuffer[iSize];
	ubytetype* piPtr = iBuffer;

	uinttype iNum   = 2048;
	longtype iVal   = iStart;
	uinttype iBytes = 0;

	for (uinttype i=0; i<iNum; i++) {
		uinttype iWritten = cxx::io::binaryWireFormat::WriteInt64ToBuffer(piPtr,
										  iSize - iBytes,
										  iVal);
		if (0 == iWritten) {
			DEEP_LOG_ERROR(OTHER, "Unable to write the Int64\n");
		}

		iBytes += iWritten;
		piPtr  += iWritten;
		iVal--;
	}

	piPtr = iBuffer;

	longtype iFind = iStart;

	for (uinttype i=0; i<iNum; i++) {
		longtype iVal  = 0;
		uinttype iRead = cxx::io::binaryWireFormat::ReadInt64FromBuffer(piPtr,
										iBytes,
										iVal);

		if (0 == iRead) {
			DEEP_LOG_ERROR(OTHER, "Unable to read in Int32\n");
			exit(-1);
		}
	
		iBytes -= iRead;
		piPtr  += iRead;

		if (iVal != iFind) {
			DEEP_LOG_ERROR(OTHER, "INT64: mismatch in values: expected %lli, got %lli\n", iFind, iVal);
			exit(-1);
		}

		//DEEP_LOG_DEBUG(OTHER, "INT64, found: %i\n", iFind);

		iFind--;
	}

	DEEP_LOG_INFO(OTHER, "INT64: %lli to %lli sequence verified.\n", iStart, iFind);

	{
		longtype iCheck = DEEP_INT64_MAX;
		iVal = iCheck;

		uinttype iWritten = cxx::io::binaryWireFormat::WriteInt64ToBuffer(iBuffer,
										  iSize,
										  iVal);
		if (0 == iWritten) {
			DEEP_LOG_ERROR(OTHER, "Failed to write MAX INT64\n");
			exit(-1);
		}

		uinttype iRead = cxx::io::binaryWireFormat::ReadInt64FromBuffer(iBuffer,
										iSize,
										iVal);
		if (0 == iRead) {
			DEEP_LOG_ERROR(OTHER, "Failed to read MAX INT64\n");
			exit(-1);
		}

		if (iCheck != iVal) {
			DEEP_LOG_ERROR(OTHER, "Failed to read MAX INT64: %lli not %lli\n", iVal, DEEP_INT64_MIN);
			exit(-1);
		}

		DEEP_LOG_INFO(OTHER, "INT64 MAX verified: %lli\n", iVal);
		
	}

	{
		longtype iCheck = DEEP_INT64_MIN;
		iVal = iCheck;

		uinttype iWritten = cxx::io::binaryWireFormat::WriteInt64ToBuffer(iBuffer,
										  iSize,
										  iVal);
		if (0 == iWritten) {
			DEEP_LOG_ERROR(OTHER, "Failed to write MIN INT64\n");
			exit(-1);
		}

		uinttype iRead = cxx::io::binaryWireFormat::ReadInt64FromBuffer(iBuffer,
										iSize,
										iVal);
		if (0 == iRead) {
			DEEP_LOG_ERROR(OTHER, "Failed to read MIN INT64\n");
			exit(-1);
		}

		if (iCheck != iVal) {
			DEEP_LOG_ERROR(OTHER, "Failed to read MIN INT64: %lli not %lli\n", iVal, iCheck);
			exit(-1);
		}

		DEEP_LOG_INFO(OTHER, "INT64 MIN verified: %lli\n", iCheck);
		
	}

	{
		ulongtype iSeed = time::GetNanos<time::SteadyClock>();
	        srand((uinttype)(iSeed & 0xffffffff));

		for (longtype i=0; i<10; i++) {
			longtype j = (longtype)(rand()) << 32 | (longtype)rand(); //6439905433781496523LL;

			uinttype iWritten = cxx::io::binaryWireFormat::WriteInt64ToBuffer(iBuffer,
											  iSize,
											  j);
			if (0 == iWritten) {
				DEEP_LOG_ERROR(OTHER, "Failed to write INT64\n");
				exit(-1);
			}

			uinttype iRead = cxx::io::binaryWireFormat::ReadInt64FromBuffer(iBuffer,
											iSize,
											iVal);
			if (0 == iRead) {
				DEEP_LOG_ERROR(OTHER, "Failed to read INT64\n");
				exit(-1);
			}

			if (j != iVal) {
				DEEP_LOG_ERROR(OTHER, "Failed to read INT64: %lli not %lli\n", iVal, j);
				exit(-1);
			}

			DEEP_LOG_INFO(OTHER, "RANDOM INT64 verified: %lli (encode size: %i - %i)\n",
				      iVal,
				      cxx::io::binaryWireFormat::SizeofInt64(iVal),
				      iWritten);
		}

	}

	{
		ulongtype iSeed = time::GetNanos<time::SteadyClock>();
	        srand((uinttype)(iSeed & 0xffffffff));

		for (longtype i=0; i<10; i++) {
			doubletype j   = (double)rand() / (double)rand();
			doubletype val = 0;

			uinttype iWritten = cxx::io::binaryWireFormat::WriteDoubleToBuffer(iBuffer,
											   iSize,
											   j);
			if (0 == iWritten) {
				DEEP_LOG_ERROR(OTHER, "Failed to write Double\n");
				exit(-1);
			}

			uinttype iRead = cxx::io::binaryWireFormat::ReadDoubleFromBuffer(iBuffer,
											 iSize,
											 val);
			if (0 == iRead) {
				DEEP_LOG_ERROR(OTHER, "Failed to read DOUBLE\n");
				exit(-1);
			}

			if (j != val) {
				DEEP_LOG_ERROR(OTHER, "Failed to read double: %f not %f\n", val, j);
				exit(-1);
			}

			DEEP_LOG_INFO(OTHER, "RANDOM doubletype verified: %f\n", val);
		}

	}
}

void testStringWriteRead() {
	cxx::lang::String cTest = "This is a test of D.E.E.P. Deep Engine Encoding Protocol.";
	static const uinttype iSize  = 4096;

	ubytetype iBuffer[iSize];

	uinttype iWritten = cxx::io::binaryWireFormat::WriteStringToBuffer(iBuffer,
									   iSize,
									   cTest);

	cxx::lang::String cRead;

	uinttype iRead =cxx::io::binaryWireFormat:: ReadStringFromBuffer(iBuffer,
									 iWritten,
									 cRead);

	if (iWritten != iRead) {
		DEEP_LOG_ERROR(OTHER, "FAILED on cxx::lang::String encoding %i != %i lengths.\n",
			       iRead,
			       iWritten);
		exit(-1);
	}

	if (0 == cRead.compare(cTest)) {
		DEEP_LOG_INFO(OTHER, "SUCCESS on cxx::lang::String encoding.\n");
	} else {
		DEEP_LOG_ERROR(OTHER, "FAILED on cxx::lang::String encoding (%s).\n",
			       cRead.c_str());
		exit(-1);
	}
}

void testBufferWriter() {
	inttype     iZero32  = 0;
	longtype    iZero64  = 0;
	inttype     iOne     = 1;
	uinttype    iTwo     = 2;
	longtype    iThree   = 3;
	ulongtype   iFour    = 4;
	floattype       fFive    = 5.5;
	doubletype      fSix     = 6.6;
	cxx::lang::String cSeven   = "This is a test of D.E.E.P. Deep Engine Encoding Protocol.";
	ubytetype   cEight[] = "This is a test of D.E.E.P. Deep Engine Encoding Protocol.";

	ulongtype iStart = time::GetNanos<time::SteadyClock>();

	uinttype iSize =		
		cxx::io::encodeProtocol::writer::sizeOfInt32(iZero32) +
		cxx::io::encodeProtocol::writer::sizeOfInt64(iZero64) +
		cxx::io::encodeProtocol::writer::sizeOfInt32(iOne) +
		cxx::io::encodeProtocol::writer::sizeOfUint32(iTwo) +
		cxx::io::encodeProtocol::writer::sizeOfInt64(iThree) +
		cxx::io::encodeProtocol::writer::sizeOfUint64(iFour) +
		cxx::io::encodeProtocol::writer::sizeOfFloat(fFive) +
		cxx::io::encodeProtocol::writer::sizeOfDouble(fSix) +
		cxx::io::encodeProtocol::writer::sizeOfString(cSeven) +
		cxx::io::encodeProtocol::writer::sizeOfRaw(cSeven.size());

	cxx::io::encodeProtocol::writer testBuffer(iSize,
						   0x80800808);

	DEEP_LOG_INFO(OTHER, "Writer buffer size: %i, created OK.\n", iSize);

	inttype status = testBuffer.setInt32Field(10, iZero32);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write int32 field!\n");
		exit(-1);
	}

	status = testBuffer.setInt64Field(0, iZero64);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write int64 field!\n");
		exit(-1);
	}

	status = testBuffer.setInt32Field(1, iOne);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write int32 field!\n");
		exit(-1);
	}

	status = testBuffer.setUint32Field(2, iTwo);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write uint32 field!\n");
		exit(-1);
	}

	status = testBuffer.setInt64Field(3, iThree);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write int64 field!\n");
		exit(-1);
	}

	status = testBuffer.setUint64Field(4, iFour);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write uint64 field!\n");
		exit(-1);
	}

	status = testBuffer.setFloatField(5, fFive);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write floattype field!\n");
		exit(-1);
	}

	status = testBuffer.setDoubleField(6, fSix);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write doubletype field!\n");
		exit(-1);
	}

	status = testBuffer.setStringField(7, cSeven);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write string field!\n");
		exit(-1);
	}

	status = testBuffer.setRawField(8, cEight, cSeven.size());
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to write raw field!\n");
		exit(-1);
	}

	status = testBuffer.setDoubleField(9, fSix);
	if (0 != status) {
		DEEP_LOG_INFO(OTHER, "Correctly detected buffer overrun.\n");
	} else {
		DEEP_LOG_ERROR(OTHER, "Failed to detected buffer overrun!\n");
		exit(-1);
	}

	ulongtype iEnd   = time::GetNanos<time::SteadyClock>();
	ulongtype iDelta = (iEnd - iStart);

	DEEP_LOG_INFO(OTHER, "Successfully encoded all fields in %llu nanos.\n", iDelta);

	ubytetype* piBuffer  = 0;
	uinttype iReadSize = 0;
	
	testBuffer.getMutableBuffer(piBuffer, iReadSize);
	if (0 == piBuffer) {
		DEEP_LOG_ERROR(OTHER, "Failed to get the encoded buffer!\n");
		exit(-1);
	}

	iStart = time::GetNanos<time::SteadyClock>();

	cxx::io::encodeProtocol::reader testReadBuffer(piBuffer, iReadSize);
	status = testReadBuffer.decodeFields();

	iEnd   = time::GetNanos<time::SteadyClock>();
	iDelta = (iEnd - iStart);

	if (0 == status) {
		DEEP_LOG_INFO(OTHER, "Successfully decoded the buffer in %llu nanos.\n", iDelta);
	} else {
		DEEP_LOG_ERROR(OTHER, "Failed to decode the buffer!\n");
		exit(-1);
	}

	const ubytetype* pcRaw    = 0;
	uinttype       iRawSize = 0;

	inttype  iZero32Fetch = 0;
	longtype iZero64Fetch = 0;

	status = testReadBuffer.getInt32Field(10, iZero32Fetch);
	if (0 == status) {
		DEEP_LOG_INFO(OTHER, "Successfully got the int32 zero field.\n");
	} else {
		DEEP_LOG_ERROR(OTHER, "Failed to get the int32 zero field!\n");
	}

	status = testReadBuffer.getInt64Field(0, iZero64Fetch);
	if (0 == status) {
		DEEP_LOG_INFO(OTHER, "Successfully got the int64 zero field.\n");
	} else {
		DEEP_LOG_ERROR(OTHER, "Failed to get the int64 zero field!\n");
	}

	status = testReadBuffer.getRawFieldRef(8, pcRaw, iRawSize);
	if (0 == status) {
		DEEP_LOG_INFO(OTHER, "Successfully got the raw field.\n");
	} else {
		DEEP_LOG_ERROR(OTHER, "Failed to get the raw field!\n");
		exit(-1);
	}

	cxx::lang::String cFetch;
	status = testReadBuffer.getStringField(7, cFetch);
	if (0 == status) {
		DEEP_LOG_INFO(OTHER, "Successfully got the string field by value.\n");
	} else {
		DEEP_LOG_ERROR(OTHER, "Failed to get the string field by value!\n");
		exit(-1);
	}

	if (0 != cSeven.compare(cFetch)) {
		DEEP_LOG_ERROR(OTHER, "String value mismatch!\n");
		exit(-1);
	}

	const char* pcStr    = 0;
	uinttype    iStrSize = 0;

	status = testReadBuffer.getStringFieldRef(7, pcStr, iStrSize);
	if (0 == status) {
		DEEP_LOG_INFO(OTHER, "Successfully got the string field by ref.\n");
	} else {
		DEEP_LOG_ERROR(OTHER, "Failed to get the string field by ref!\n");
		exit(-1);
	}

	if (0 == memcmp(cSeven.c_str(), pcStr, iStrSize)) {
		DEEP_LOG_INFO(OTHER, "String Value is good\n");
	} else {
		DEEP_LOG_ERROR(OTHER, "String Value mismatched!");
		exit(-1);
	}

	inttype iOneFetch = 0;
	status = testReadBuffer.getInt32Field(1, iOneFetch);
	if ((0 != status) || (iOne != iOneFetch)) {
		DEEP_LOG_ERROR(OTHER, "Value fetch mismatch\n");
		exit(-1);
	}

	uinttype iTwoFetch = 0;
	status = testReadBuffer.getUint32Field(2, iTwoFetch);
	if ((0 != status) || (iTwo != iTwoFetch)) {
		DEEP_LOG_ERROR(OTHER, "Value fetch mismatch\n");
		exit(-1);
	}

	longtype iThreeFetch = 0;
	status = testReadBuffer.getInt64Field(3, iThreeFetch);
	if ((0 != status) || (iThree != iThreeFetch)) {
		DEEP_LOG_ERROR(OTHER, "Value fetch mismatch\n");
		exit(-1);
	}

	ulongtype iFourFetch = 0;
	status = testReadBuffer.getUint64Field(4, iFourFetch);
	if ((0 != status) || (iFour != iFourFetch)) {
		DEEP_LOG_ERROR(OTHER, "Value fetch mismatch\n");
		exit(-1);
	}

	floattype fFiveFetch = 0;
	status = testReadBuffer.getFloatField(5, fFiveFetch);
	if ((0 != status) || (fFive != fFiveFetch)) {
		DEEP_LOG_ERROR(OTHER, "Value fetch mismatch\n");
		exit(-1);
	}

	doubletype fSixFetch = 0;
	status = testReadBuffer.getDoubleField(6, fSixFetch);
	if ((0 != status) || (fSix != fSixFetch)) {
		DEEP_LOG_ERROR(OTHER, "Value fetch mismatch\n");
		exit(-1);
	}

	DEEP_LOG_INFO(OTHER, "All values fetched and match.\n");

	/** Empty payload test. */
	cxx::io::encodeProtocol::writer testEmptyPayload(0,
							 0x80800809);

	testEmptyPayload.getMutableBuffer(piBuffer, iReadSize);
	if (0 == piBuffer) {
		DEEP_LOG_ERROR(OTHER, "Failed to get the encoded buffer!\n");
		exit(-1);
	}

	cxx::io::encodeProtocol::reader testReadEmptyPayload(piBuffer, iReadSize);
	status = testReadBuffer.decodeFields();
	if (0 == status) {
		DEEP_LOG_INFO(OTHER, "Successfully decoded an empty message payload.\n");
	} else {
		DEEP_LOG_ERROR(OTHER, "Failed to decoded an empty message payload.\n");
		exit(-1);
	}
}

void testUint32ArrayWriteRead() {

	static const uinttype iArraySize=100;

	ulongtype iStart = time::GetNanos<time::SteadyClock>();

	cxx::lang::array<uinttype> cValues(iArraySize);
	for (uinttype i=0; i<iArraySize; i++) {
		cValues[i] = i;
	}

	uinttype iSize = cxx::io::encodeProtocol::writer::sizeOfUint32Array(cValues);

	cxx::io::encodeProtocol::writer testBuffer(iSize,
						   0x80800808);

	int status = testBuffer.setUint32ArrayField(1, cValues);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Failed to encode an array to the message payload.\n");
		exit(-1);
	}

	ulongtype iEnd   = time::GetNanos<time::SteadyClock>();
	ulongtype iDelta = (iEnd - iStart);

	DEEP_LOG_INFO(OTHER, "Successfully encoded all values in the array field in %llu nanos.\n", iDelta);

	ubytetype* piBuffer  = 0;
	uinttype   iReadSize = 0;
	
	testBuffer.getMutableBuffer(piBuffer, iReadSize);
	if (0 == piBuffer) {
		DEEP_LOG_ERROR(OTHER, "Failed to get the encoded buffer!\n");
		exit(-1);
	}

	cxx::lang::array<uinttype>* pcReadValues = 0;

	iStart = time::GetNanos<time::SteadyClock>();

	cxx::io::encodeProtocol::reader testReadBuffer(piBuffer, iReadSize);

	status = testReadBuffer.decodeFields();

	iEnd   = time::GetNanos<time::SteadyClock>();
	iDelta = (iEnd - iStart);

	if (0 == status) {
		DEEP_LOG_INFO(OTHER, "Successfully decoded the buffer in %llu nanos.\n", iDelta);
	} else {
		DEEP_LOG_ERROR(OTHER, "Failed to decode the buffer!\n");
		exit(-1);
	}

	testReadBuffer.getUint32ArrayField(1, pcReadValues);

	if (cValues.length != pcReadValues->length) {
		DEEP_LOG_ERROR(OTHER, "Failed decoded array length does not match %u != %u.\n",
			       cValues.length,
			       pcReadValues->length);
		exit(-1);
	}

	for (int i=0; i<cValues.length; i++) {
		if (cValues[i] != pcReadValues->operator[](i)) {
			DEEP_LOG_ERROR(OTHER, "Array values mismatch %u != %u.\n",
				       cValues[i],
				       pcReadValues->operator[](i));
			exit(-1);
		}
	}
}

int main(int argc, char **argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	testInt32WriteRead();
	testInt64WriteRead();
	testStringWriteRead();
	testBufferWriter();
	testUint32ArrayWriteRead();

	return 0;
}
