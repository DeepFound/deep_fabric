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
#ifndef COM_DEEPIS_DATASTORE_API_DEEP_PROTOCOL_H_
#define COM_DEEPIS_DATASTORE_API_DEEP_PROTOCOL_H_

#include "com/deepis/datastore/api/deep/DeepTypes.h"
#include "com/deepis/datastore/api/deep/Store.h"

#include "com/deepis/db/store/relative/core/RealTimeVersion.h"

#define FIXED_VARCHAR_LENGTH 10

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace core {

template<>
class KeyProtocol_v1<DeepNByte*> {

	public:
		FORCE_INLINE static DeepNByte* readKey(BufferedRandomAccessFile* keyFile, shorttype keySize, boolean* eof = null) {
			if (keySize == -1) {
				keySize = keyFile->readShort(eof);
			}

			DeepNByte* key = new DeepNByte(keySize);
			keyFile->BufferedRandomAccessFile::readFully(key, 0, keySize, eof);

			return key;
		}

		FORCE_INLINE static void skipKey(BufferedRandomAccessFile* keyFile, shorttype keySize, boolean* eof = null) {
			if (keySize == -1) {
				keySize = keyFile->readShort(eof);
			}

			keyFile->BufferedRandomAccessFile::skipBytes(keySize, eof);
		}

		FORCE_INLINE static void writeKey(DeepNByte* key, BufferedRandomAccessFile* keyFile, shorttype keySize) {
			#ifdef DEEP_DEBUG
			// TODO: DATABASE-1234: support null key in key-value store
			if (key == null) {
				DEEP_LOG(ERROR, OTHER, "Invalid key in writeKey");

				throw InvalidException("Invalid key in writeKey");
			}
			#endif

			if (keySize == -1) {
				keySize = key->length;
				keyFile->writeShort(keySize);
			}

			keyFile->BufferedRandomAccessFile::write(key, 0, keySize);
		}
};

template<>
class KeyProtocol_v1<DeepComposite*> {

	public:
		FORCE_INLINE static DeepComposite* readKey(BufferedRandomAccessFile* keyFile, shorttype keySize, boolean* eof = null) {
			if (keySize == -1) {
				keySize = keyFile->readShort(eof);
			}

			DeepComposite* key = new DeepComposite(keySize);
			keyFile->BufferedRandomAccessFile::readFully(key, 0, keySize, eof);

			return key;
		}

		FORCE_INLINE static void skipKey(BufferedRandomAccessFile* keyFile, shorttype keySize, boolean* eof = null) {
			if (keySize == -1) {
				keySize = keyFile->readShort(eof);
			}

			keyFile->BufferedRandomAccessFile::skipBytes(keySize, eof);
		}

		FORCE_INLINE static void writeKey(DeepComposite* key, BufferedRandomAccessFile* keyFile, shorttype keySize) {
			#ifdef DEEP_DEBUG
			// TODO: DATABASE-1234: support null key in key-value store
			if (key == null) {
				DEEP_LOG(ERROR, OTHER, "Invalid key in writeKey");

				throw InvalidException("Invalid key in writeKey");
			}
			#endif

			if (keySize == -1) {
				keySize = key->length;
				keyFile->writeShort(keySize);
			}

			keyFile->BufferedRandomAccessFile::write(key, 0, keySize);
		}
};

class RowProtocol_v1 {
	public:
		FORCE_INLINE static bytearray packRow(const Store* schemaBuilder, const bytearray unpackedRow, uinttype length, bytearray* buffer, uinttype* bufferLength) {
			if ((schemaBuilder->hasVariableLengthFields() == false) &&
			    (schemaBuilder->hasVirtualFields() == false)) { // TODO:
				return (bytearray) unpackedRow;
			}

			bytearray packedRow = null;
			updateBufferLength(buffer, bufferLength, length);
			packedRow = *buffer;

			bytearray unpackedFixedPtr = null;
			bytearray packedFixedPtr = null;
			uinttype fixedLength = 0;

			uinttype packedRowOffset = 0;

			if (schemaBuilder->getNullBytes()) {
				memcpy(packedRow, unpackedRow, schemaBuilder->getNullBytes());
				packedRowOffset += schemaBuilder->getNullBytes();
			}

			bytearray blobDataPtr = packedRow + schemaBuilder->getBlobStartOffset();
			uinttype  blobOffset  = 0;

			const BasicArray<DeepField*>* fields = schemaBuilder->getFields();
			inttype iMaxFields  = fields->size();
			inttype maxJ        = (false == schemaBuilder->hasVirtualFields()) ? 1 : 2;

			for (inttype j = 0; j < maxJ; j++) {

				for (inttype i = 0; i < iMaxFields; i++) {
					const DeepField* field = fields->get(i);
					bytearray ptr = (bytearray) unpackedRow + field->getValueOffset();

					if (true == field->isGcVirtual()) {
						if (0 == j) {
							if (fixedLength != 0) {
								memcpy(packedFixedPtr, unpackedFixedPtr, fixedLength);
								fixedLength = 0;
							}

							continue;
						}

					} else if (1 == j) {
						continue;
					}

					if (isVariableLengthVarchar(field)) {
						if (fixedLength != 0) {
							memcpy(packedFixedPtr, unpackedFixedPtr, fixedLength);
							fixedLength = 0;
						}

						uinttype blobLen = 0;
						uinttype blobLenBytes = field->getLengthBytes();

						// if field is not null get length of variable data
						if (field->isNull(unpackedRow) == false) {
							blobLen = blobLenBytes == 1 ? ((uinttype) *ptr) & 0x000000FF : dp_uint2korr(ptr);
						}

						// pack the length of the variable data, 4 bytes
						memcpy(blobDataPtr + blobOffset, &blobLen, sizeof(uinttype));
						blobOffset += sizeof(uinttype);

						// pack variable length data onto end (i.e. blob data)
						if (blobLen > 0) {
							memcpy(blobDataPtr + blobOffset, ptr + blobLenBytes, blobLen);
							blobOffset += blobLen;
						}

					} else if (field->getType() == CT_DATASTORE_BLOB) { // XXX: DEEP_TYPE_BLOB
						if (fixedLength != 0) {
							memcpy(packedFixedPtr, unpackedFixedPtr, fixedLength);
							fixedLength = 0;
						}

						uinttype blobLen = 0;
						uinttype blobLenBytes = field->getRowPackLength();

						// if field is not null get length of variable data
						if (field->isNull(unpackedRow) == false) {
							blobLen = field->getLength<CT_DATASTORE_BLOB>(unpackedRow); // XXX: DEEP_TYPE_BLOB
						}

						// pack the length of the variable data, 4 bytes
						memcpy(blobDataPtr + blobOffset, &blobLen, sizeof(uinttype));
						blobOffset += sizeof(uinttype);

						// pack variable length data onto end (i.e. blob data)
						if (blobLen > 0) {
							bytearray deepBlobPtr = null;
							memcpy((bytearray)(&deepBlobPtr), ptr + blobLenBytes, schemaBuilder->getBlobPtrSize());
							memcpy(blobDataPtr + blobOffset, deepBlobPtr, blobLen);
							blobOffset += blobLen;
						}

					} else if (field->getType() == CT_DATASTORE_TEXT_MULTIBYTE) {
						if (fixedLength != 0) {
							memcpy(packedFixedPtr, unpackedFixedPtr, fixedLength);
							fixedLength = 0;
						}

						uinttype blobLen = 0;

						// if field is not null get length of variable data
						if (field->isNull(unpackedRow) == false) {
							blobLen = getMultiByteTextLength(field->getPackLength(), field->getCharacterSet(), ptr);
						}

						// pack the length of the variable data, 4 bytes
						memcpy(blobDataPtr + blobOffset, &blobLen, sizeof(uinttype));
						blobOffset += sizeof(uinttype);

						// pack variable length data onto end (i.e. blob data)
						if (blobLen > 0) {
							memcpy(blobDataPtr + blobOffset, ptr, blobLen);
							blobOffset += blobLen;
						}

					} else {
						// collapse consecutive fixed length columns into a single memcpy
						if (fixedLength == 0) {
							unpackedFixedPtr = ptr;
							packedFixedPtr = packedRow + packedRowOffset;
						}

						uinttype packLength = field->getPackLength();
						fixedLength += packLength;
						packedRowOffset += packLength;
					}
				}

				if (fixedLength != 0) {
					memcpy(packedFixedPtr, unpackedFixedPtr, fixedLength);
					fixedLength = 0;
				}
			}

			return packedRow;
		}

		FORCE_INLINE static bytearray unpackRow(const Store* schemaBuilder, bytearray unpackedRow, const bytearray packedRow, bytearray* blobBuffer, uinttype* blobBufferLength) {
			if (schemaBuilder->hasVariableLengthFields() == false) {
				memcpy(unpackedRow, packedRow, unpackLength(schemaBuilder));
				return unpackedRow;
			}
			
			// get length of blob only data, varchar will be unpacked in place
			uinttype blobLengthRequired = unpackBlobLength(schemaBuilder, packedRow);
			updateBufferLength(blobBuffer, blobBufferLength, blobLengthRequired);

			bytearray unpackedFixedPtr = null;
			bytearray packedFixedPtr = null;
			uinttype fixedLength = 0;

			uinttype deepBlobOffset = 0;
			uinttype packedRowOffset = 0;
			uinttype blobField = 0;

			if (schemaBuilder->getNullBytes()) {
				memcpy(unpackedRow, packedRow, schemaBuilder->getNullBytes());
				packedRowOffset += schemaBuilder->getNullBytes();
			}

			const BasicArray<DeepField*>* fields     = schemaBuilder->getFields();
			inttype                        iMaxFields = fields->size();
			inttype                        maxJ       = (false == schemaBuilder->hasVirtualFields()) ? 1 : 2;

			for (inttype j = 0; j < maxJ; j++) {

				for (inttype i = 0; i < iMaxFields; i++) {
					const DeepField* field = fields->get(i);
					uinttype fieldOffset = field->getValueOffset(); // XXX: getFieldOffset(field)

					if (true == field->isGcVirtual()) {
						if (0 == j) {
							if (fixedLength != 0) {
								memcpy(unpackedFixedPtr, packedFixedPtr, fixedLength);
								fixedLength = 0;
							}

							continue;
						}

					} else if (1 == j) {
						continue;
					}

					if (isVariableLengthVarchar(field)) {
						if (fixedLength != 0) {
							memcpy(unpackedFixedPtr, packedFixedPtr, fixedLength);
							fixedLength = 0;
						}

						const uinttype packedRowOffset = getPackedRowOffset(schemaBuilder, packedRow, blobField);
						uinttype blobLen = getPackedBlobLength(packedRow, packedRowOffset);

						uinttype blobLenBytes = field->getLengthBytes();
						switch(blobLenBytes) {
							case 2:
								dp_int2store(unpackedRow + fieldOffset, blobLen);
								break;
							default:
								unpackedRow[fieldOffset] = (uchartype) blobLen;
						}

						if (blobLen > 0) {
							memcpy(unpackedRow + fieldOffset + blobLenBytes, getPackedBlob(packedRow, packedRowOffset), blobLen);
						}

						++blobField;

					} else if (field->getType() == CT_DATASTORE_BLOB) { // XXX: DEEP_TYPE_BLOB
						if (fixedLength != 0) {
							memcpy(unpackedFixedPtr, packedFixedPtr, fixedLength);
							fixedLength = 0;
						}

						const uinttype packedRowOffset = getPackedRowOffset(schemaBuilder, packedRow, blobField);
						uinttype blobLen = getPackedBlobLength(packedRow, packedRowOffset);

						uinttype blobLenBytes = field->getRowPackLength();
						switch(blobLenBytes) {
							case 4:
								dp_int4store(unpackedRow + fieldOffset, blobLen);
								break;
							case 3:
								dp_int3store(unpackedRow + fieldOffset, blobLen);
								break;
							case 2:
								dp_int2store(unpackedRow + fieldOffset, blobLen);
								break;
							default:
								unpackedRow[fieldOffset] = (uchartype) blobLen;
						}

						if (blobLen > 0) {
							bytearray deepBlobPtr = (*blobBuffer) + deepBlobOffset;

							memcpy(deepBlobPtr, getPackedBlob(packedRow, packedRowOffset), blobLen);

							memcpy(unpackedRow + fieldOffset + blobLenBytes, (bytearray) (&deepBlobPtr), schemaBuilder->getBlobPtrSize());

							deepBlobOffset += blobLen;
						}

						++blobField;

					} else if (field->getType() == CT_DATASTORE_TEXT_MULTIBYTE) {
						if (fixedLength != 0) {
							memcpy(unpackedFixedPtr, packedFixedPtr, fixedLength);
							fixedLength = 0;
						}

						const uinttype packedRowOffset = getPackedRowOffset(schemaBuilder, packedRow, blobField);
						uinttype blobLen = getPackedBlobLength(packedRow, packedRowOffset);

						if (blobLen > 0) {
							memcpy(unpackedRow + fieldOffset, getPackedBlob(packedRow, packedRowOffset), blobLen);

							padMultiByteText(field->getPackLength(), blobLen, unpackedRow + fieldOffset);
						}

						++blobField;

					} else {
						// collapse consecutive fixed length columns into a single memcpy
						if (fixedLength == 0) {
							unpackedFixedPtr = unpackedRow + fieldOffset;
							packedFixedPtr = packedRow + packedRowOffset;
						}

						uinttype packLength = field->getPackLength();
						fixedLength += packLength;
						packedRowOffset += packLength;
					}
				}

				if (fixedLength != 0) {
					memcpy(unpackedFixedPtr, packedFixedPtr, fixedLength);
					fixedLength = 0;
				}
			}
			return unpackedRow;
		}

		FORCE_INLINE static bytearray unpackRowFromKey(const Store* schemaBuilder, bytearray unpackedRow, bytearray packedKey, bytearray* blobBuffer, uinttype* blobBufferLength, bool preserve) {
			uint deepBlobOffset = 0;

			updateBufferLength(blobBuffer, blobBufferLength, 2 * CT_DATASTORE_MAX_KEY_LENGTH);

			if (preserve == false) {
				memset(unpackedRow, 0, unpackLength(schemaBuilder));
			}

			const BasicArray<DeepKeyPart*>* keyParts = schemaBuilder->getKeyParts();
			for (inttype i=0; i<keyParts->size(); i++) {
				const DeepKeyPart* keyPart = keyParts->get(i);

				packedKey = unpackRowFromKeyPart(schemaBuilder, keyPart, unpackedRow, packedKey, &deepBlobOffset, blobBuffer, blobBufferLength);
			}

			return unpackedRow;
		}

	private:
		FORCE_INLINE static boolean isVariableLengthVarchar(const DeepField* field) {
			#if 1
			return field->getRealType() == CT_DATASTORE_VARTEXT1 && field->getKeyLength() > FIXED_VARCHAR_LENGTH; // XXX: DEEP_TYPE_VARCHAR
			#else
			return field->getRealType() == CT_DATASTORE_VARTEXT1; // XXX: DEEP_TYPE_VARCHAR
			#endif
		}
		
		/* unpack length is just the length deep expects for the record */
		FORCE_INLINE static uinttype unpackLength(const Store* schemaBuilder) {
			return schemaBuilder->getUnpackedRowLength(); //table_share->reclength;
		}
		
		/* calculate the total length of the blob only data to unpack for deep */
		FORCE_INLINE static uint unpackBlobLength(const Store* schemaBuilder, const bytearray packedRow) {
			const BasicArray<DeepField*>* fields = schemaBuilder->getFields();
			if (fields->size() == 0) {
				return 0;
			}
			
			uinttype blobLength = 0;

			// just care about blobs when unpacking, varchars are expanded in record
			uinttype blobPosition = 0;
			inttype iMaxFields    = fields->size();
			inttype maxJ          = (false == schemaBuilder->hasVirtualFields()) ? 1 : 2;

			for (inttype j = 0; j < maxJ; j++) {

				for (inttype i = 0; i < iMaxFields; i++) {
					const DeepField* field = fields->get(i);

					if (true == field->isGcVirtual()) {
						if (0 == j) {
							continue;
						}
					} else if (1 == j) {
						continue;
					}
					
					if (field->getType() == CT_DATASTORE_BLOB) { // XXX: DEEP_TYPE_BLOB
						const uinttype packedRowOffset = getPackedRowOffset(schemaBuilder, packedRow, blobPosition);
						blobLength += getPackedBlobLength(packedRow, packedRowOffset);
						++blobPosition;

					} else if (field->getType() == CT_DATASTORE_TEXT_MULTIBYTE) {
						++blobPosition;

					} else if (isVariableLengthVarchar(field)) {
						++blobPosition;
					}
				}
			}
			
			return blobLength;
		}
		
		FORCE_INLINE static uinttype getPackedRowOffset(const Store* schemaBuilder, const bytearray packedRow, uinttype blobPosition) {
			uinttype packedRowOffset = schemaBuilder->getBlobStartOffset();
			
			for (uinttype i=0; i<blobPosition; ++i) {
				packedRowOffset += (dp_uint4korr(packedRow + packedRowOffset) + sizeof(uinttype));
			}
			
			return packedRowOffset;
		}

		FORCE_INLINE static uinttype getPackedBlobLength(const bytearray packedRow, const uinttype packedRowOffset) {
			return dp_uint4korr(packedRow + packedRowOffset);
		}

		FORCE_INLINE static bytearray getPackedBlob(const bytearray packedRow, const uinttype packedRowOffset) {
			return (bytearray) (packedRow + packedRowOffset + sizeof(uinttype));
		}

		FORCE_INLINE static bool updateBufferLength(bytearray* buffer, uinttype* bufferLength, uinttype newLength) {
			if (newLength > 0) {
				if ((*buffer == null) || (newLength > *bufferLength)) {
					bytearray newBuffer;
					newBuffer = (bytearray) realloc((void*) *buffer, newLength);

					*buffer = newBuffer;
					*bufferLength = newLength;
				}
			}

			return true;
		}

		FORCE_INLINE static bytearray unpackRowFromKeyPart(const Store* schemaBuilder, const DeepKeyPart* keyPart, bytearray unpackedRow, bytearray packedKey, uinttype* deepBlobOffset, bytearray* blobBuffer, uinttype* blobBufferLength) {
			uinttype packedKeyOffset = 0;

			const DeepField* field = keyPart->getField();
			if (field == null) { // XXX: hidden key case
				return packedKey;
			}

			if (keyPart->getNullBit()) {
				if (packedKey[0]) {
					unpackedRow[keyPart->getNullOffset()] = unpackedRow[keyPart->getNullOffset()] | keyPart->getNullBit();
					// key part is null, return
					return (packedKey + keyPart->getPackLength(packedKey));

				} else {
					unpackedRow[keyPart->getNullOffset()] = unpackedRow[keyPart->getNullOffset()] & ~(keyPart->getNullBit());

					packedKeyOffset += 1;
				}
			}

			uinttype fieldOffset = field->getValueOffset();
			if (field->getType() == CT_DATASTORE_VARTEXT1) { /* XXX: DEEP_TYPE_VARCHAR; is_variable_length_varchar(field) not needed here */
				uinttype blobLenBytes = field->getLengthBytes();
				uinttype blobLen = dp_uint2korr(packedKey + packedKeyOffset);

				packedKeyOffset += 2;

				switch(blobLenBytes) {
					case 2:
						dp_int2store(unpackedRow + fieldOffset, blobLen);
						break;
					default:
						*(unpackedRow + fieldOffset) = (char) (blobLen & 0x000000FF);
				}

				if (blobLen > 0) {
					memcpy(unpackedRow + fieldOffset + blobLenBytes, packedKey + packedKeyOffset, blobLen);
				}

			} else if (field->getType() == CT_DATASTORE_BLOB) { // XXX: DEEP_TYPE_BLOB
				uinttype blobLenBytes = field->getRowPackLength();
				uinttype blobLen = dp_uint2korr(packedKey + packedKeyOffset);

				packedKeyOffset += 2;

				switch(blobLenBytes) {
					case 4:
						dp_int4store(unpackedRow + fieldOffset, blobLen);
						break;
					case 3:
						dp_int3store(unpackedRow + fieldOffset, blobLen);
						break;
					case 2:
						dp_int2store(unpackedRow + fieldOffset, blobLen);
						break;
					default:
						*(unpackedRow + fieldOffset) = (char) (blobLen & 0x000000FF);
				}

				if (blobLen > 0) {
					bytearray deepBlobPtr = *blobBuffer + *deepBlobOffset;

					memcpy(deepBlobPtr, packedKey + packedKeyOffset, blobLen);

					memcpy(unpackedRow + fieldOffset + blobLenBytes, (bytearray) (&deepBlobPtr), schemaBuilder->getBlobPtrSize());

					*deepBlobOffset += blobLen;
				}

			} else if (field->getType() == CT_DATASTORE_TEXT_MULTIBYTE) {
				// XXX: only packed in value, not keys, so memcpy is fine here
				memcpy(unpackedRow + fieldOffset, packedKey + packedKeyOffset, keyPart->getLength());

			} else {
				memcpy(unpackedRow + fieldOffset, packedKey + packedKeyOffset, keyPart->getLength());
			}

			return (packedKey + keyPart->getPackLength(packedKey));
		}
};

} } } } } } // namespace

#endif //COM_DEEPIS_DATASTORE_API_DEEP_PROTOCOL_H_
