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
#ifndef COM_DEEPIS_DATASTORE_API_DEEP_SCHEMA_H_
#define COM_DEEPIS_DATASTORE_API_DEEP_SCHEMA_H_

#include "com/deepis/datastore/api/deep/KeyBuilder.h"
#include "com/deepis/datastore/api/deep/DeepTypes.h"
#include "com/deepis/datastore/api/deep/Store.h"

#include "com/deepis/db/store/relative/core/RealTimeSchema.h"

#include "cxx/util/Collections.h"

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace core {

using namespace cxx::util;

template<typename K>
struct DeepKeySchema_v1 {
	public:
		static const String KEY_PART_LIST;
		static const String KEY_PART;
		static const String KEY_PART_FIELD_INDEX;
		static const String KEY_PART_TYPE;
		static const String KEY_PART_LENGTH;
		static const String KEY_PART_VALUE_OFFSET;
		static const String KEY_PART_NULL_OFFSET;
		static const String KEY_PART_NULL_BIT;
		static const String KEY_PART_RESERVED;
		static const String KEY_PART_IGNORED;
		static const String KEY_PART_VARIABLE_POSITION;
		static const String KEY_PART_PRIMARY_POSITION;
	public:
		static void read(org::w3c::dom::Element& schemaElem, SchemaBuilder<K>* schemaBuilder) {
			org::w3c::dom::Element elem;
			schemaElem.getFirstChild(&elem);
			elem.getFirstChild(&elem);

			org::w3c::dom::Element keyPartListElem;
			elem.getFirstChild(&keyPartListElem);

			org::w3c::dom::Element keyPartElem;
			keyPartListElem.getFirstChild(&keyPartElem);

			do {
				uinttype fieldIndex = keyPartElem.getAttributeUint(KEY_PART_FIELD_INDEX);
				inttype type = keyPartElem.getAttributeInt(KEY_PART_TYPE);
				inttype length = keyPartElem.getAttributeInt(KEY_PART_LENGTH);
				inttype valueOffset = keyPartElem.getAttributeInt(KEY_PART_VALUE_OFFSET);
				inttype nullOffset = keyPartElem.getAttributeInt(KEY_PART_NULL_OFFSET);
				inttype nullBit = keyPartElem.getAttributeInt(KEY_PART_NULL_BIT);
				inttype variablePosition = keyPartElem.getAttributeInt(KEY_PART_VARIABLE_POSITION);
				inttype primaryPosition = keyPartElem.getAttributeInt(KEY_PART_PRIMARY_POSITION);
				boolean isIgnored = keyPartElem.getAttributeBool(KEY_PART_IGNORED);
				boolean isReserved = keyPartElem.getAttributeBool(KEY_PART_RESERVED);

				schemaBuilder->addKeyPart(createDeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition));

			} while (keyPartElem.getNextSibling(&keyPartElem) != null);
		}

		static void write(org::w3c::dom::Document* doc, const SchemaBuilder<K>* schemaBuilder) {
			org::w3c::dom::Element elem;
			doc->getFirstChild(&elem);
			elem.getFirstChild(&elem);
			elem.getFirstChild(&elem);

			org::w3c::dom::Element keyPartListElem;
			doc->createElement(KEY_PART_LIST, &keyPartListElem);
			elem.appendChild(&keyPartListElem, &keyPartListElem);

			Iterator<DeepKeyPart*>* iter = Collections::unmodifiableIteratorFrom< DeepKeyPart*,BasicArray<DeepKeyPart*>::BasicArrayIterator,BasicArray<DeepKeyPart*> >(schemaBuilder->getKeyParts());
			while(iter->hasNext()) {
				DeepKeyPart* keyPart = iter->next();

				org::w3c::dom::Element keyPartElem;
				doc->createElement(KEY_PART, &keyPartElem);

				keyPartElem.setAttributeUint(KEY_PART_FIELD_INDEX, keyPart->getFieldIndex());
				keyPartElem.setAttributeInt(KEY_PART_TYPE, keyPart->getType());
				keyPartElem.setAttributeInt(KEY_PART_LENGTH, keyPart->getLength());
				keyPartElem.setAttributeInt(KEY_PART_VALUE_OFFSET, keyPart->getValueOffset());
				keyPartElem.setAttributeInt(KEY_PART_NULL_OFFSET, keyPart->getNullOffset());
				keyPartElem.setAttributeInt(KEY_PART_NULL_BIT, keyPart->getNullBit());
				keyPartElem.setAttributeInt(KEY_PART_VARIABLE_POSITION, keyPart->getVariablePosition());
				keyPartElem.setAttributeInt(KEY_PART_PRIMARY_POSITION, keyPart->getPrimaryPosition());
				keyPartElem.setAttributeBool(KEY_PART_IGNORED, keyPart->isIgnored());
				keyPartElem.setAttributeBool(KEY_PART_RESERVED, keyPart->isReserved());

				keyPartListElem.appendChild(&keyPartElem, &keyPartElem);
			}
			delete iter;
		}
};

template<typename K>
struct DeepFieldSchema_v1 {
	public:
		static const String FIELD_LIST;
		static const String FIELD;
		static const String FIELD_TYPE;
		static const String FIELD_REAL_TYPE;
		static const String FIELD_PACK_LENGTH;
		static const String FIELD_ROW_PACK_LENGTH;
		static const String FIELD_KEY_LENGTH;
		static const String FIELD_LENGTH_BYTES;
		static const String FIELD_INDEX;
		static const String FIELD_NULL_BIT;
		static const String FIELD_NULL_OFFSET;
		static const String FIELD_VALUE_OFFSET;
		static const String FIELD_CHARACTER_SET;
		static const String FIELD_GCOL_VIRTUAL;
		static const String FIELD_NAME;
	
	public:
		static void read(org::w3c::dom::Element& schemaElem, SchemaBuilder<K>* schemaBuilder) {
			org::w3c::dom::Element elem;
			schemaElem.getFirstChild(&elem);
			elem.getFirstChild(&elem);
			elem.getNextSibling(&elem);

			org::w3c::dom::Element fieldListElem;
			elem.getFirstChild(&fieldListElem);
			//fieldListElem.getNextSibling(&fieldListElem);

			org::w3c::dom::Element fieldElem;
			if (fieldListElem.getFirstChild(&fieldElem) == null) {
				return;
			}

			do {
				uchartype  type = fieldElem.getAttributeUint(FIELD_TYPE);
				uchartype  realType = fieldElem.getAttributeUint(FIELD_REAL_TYPE);
				uinttype   packLength = fieldElem.getAttributeUint(FIELD_PACK_LENGTH);
				uinttype   rowPackLength = fieldElem.getAttributeUint(FIELD_ROW_PACK_LENGTH);
				uinttype   keyLength = fieldElem.getAttributeUint(FIELD_KEY_LENGTH);
				uinttype   lengthBytes = fieldElem.getAttributeUint(FIELD_LENGTH_BYTES);
				ushorttype index = fieldElem.getAttributeUint(FIELD_INDEX);
				uchartype  nullBit = fieldElem.getAttributeUint(FIELD_NULL_BIT);
				uinttype   nullOffset = fieldElem.getAttributeUint(FIELD_NULL_OFFSET);
				uinttype   valueOffset = fieldElem.getAttributeUint(FIELD_VALUE_OFFSET);
				inttype    characterSet = fieldElem.getAttributeInt(FIELD_CHARACTER_SET);
				bool       gcolVirtual = fieldElem.getAttributeBool(FIELD_GCOL_VIRTUAL);
				const char* fieldName = fieldElem.getAttribute(FIELD_NAME);
				schemaBuilder->addField(new DeepField(type, realType, packLength, rowPackLength,
								       keyLength, lengthBytes, index, nullBit, nullOffset, valueOffset,
								       characterSet, gcolVirtual, fieldName));

			} while (fieldElem.getNextSibling(&fieldElem) != null);
		}

		static void write(org::w3c::dom::Document* doc, const SchemaBuilder<K>* schemaBuilder) {
			org::w3c::dom::Element elem;
			doc->getFirstChild(&elem);
			elem.getFirstChild(&elem);
			elem.getFirstChild(&elem);
			elem.getNextSibling(&elem);

			org::w3c::dom::Element fieldListElem;
			doc->createElement(FIELD_LIST, &fieldListElem);
			elem.appendChild(&fieldListElem, &fieldListElem);

			Iterator<DeepField*>* iter = Collections::unmodifiableIteratorFrom< DeepField*,BasicArray<DeepField*>::BasicArrayIterator,BasicArray<DeepField*> >(schemaBuilder->getFields());
			while(iter->hasNext()) {
				DeepField* field = iter->next();

				org::w3c::dom::Element fieldElem;
				doc->createElement(FIELD, &fieldElem);

				fieldElem.setAttributeUint(FIELD_TYPE, (uinttype)field->getType());
				fieldElem.setAttributeUint(FIELD_REAL_TYPE, (uinttype)field->getRealType());
				fieldElem.setAttributeUint(FIELD_PACK_LENGTH, field->getPackLength());
				fieldElem.setAttributeUint(FIELD_ROW_PACK_LENGTH, field->getRowPackLength());
				fieldElem.setAttributeUint(FIELD_KEY_LENGTH, field->getKeyLength());
				fieldElem.setAttributeUint(FIELD_LENGTH_BYTES, field->getLengthBytes());
				fieldElem.setAttributeUint(FIELD_INDEX, (uinttype)field->getIndex());
				fieldElem.setAttributeUint(FIELD_NULL_BIT, (uinttype)field->getNullBit());
				fieldElem.setAttributeUint(FIELD_NULL_OFFSET, field->getNullOffset());
				fieldElem.setAttributeUint(FIELD_VALUE_OFFSET, field->getValueOffset());
				fieldElem.setAttributeInt(FIELD_CHARACTER_SET, field->getCharacterSet());
				fieldElem.setAttributeBool(FIELD_GCOL_VIRTUAL, field->isGcVirtual());
				fieldElem.setAttribute(FIELD_NAME, field->getFieldName());
				fieldListElem.appendChild(&fieldElem, &fieldElem);
			}
			delete iter;
		}
};

template<typename K> 
struct DeepFragmentationSchema_v1 {
	public:
		static const String FRAGMENTATION;
		static const String TOTAL_COUNT;
		static const String DEAD_COUNT;

	public:
		static void read(org::w3c::dom::Element& fragmentElem, SchemaBuilder<K>* schemaBuilder, ushorttype fileIndex, boolean value) {

			longtype totalCount = fragmentElem.getAttributeUint(TOTAL_COUNT);
			longtype deadCount  = fragmentElem.getAttributeUint(DEAD_COUNT);
		
			if (value == true) {
				schemaBuilder->setValueFragmentationTotalCount(fileIndex, totalCount);
				schemaBuilder->setValueFragmentationDeadCount(fileIndex, deadCount);
			} else {
				schemaBuilder->setKeyFragmentationTotalCount(fileIndex, totalCount);
				schemaBuilder->setKeyFragmentationDeadCount(fileIndex, deadCount);
			}
		}

		static void write(org::w3c::dom::Document* doc, org::w3c::dom::Element& statElem, const SchemaBuilder<K>* schemaBuilder, ushorttype fileIndex, boolean value) {
			org::w3c::dom::Element fragmentElem;
			doc->createElement(FRAGMENTATION, &fragmentElem);

			if (value == true) {
				fragmentElem.setAttributeUint(TOTAL_COUNT, schemaBuilder->getValueFragmentationTotalCount(fileIndex));
				fragmentElem.setAttributeUint(DEAD_COUNT, schemaBuilder->getValueFragmentationDeadCount(fileIndex));
			} else {
				fragmentElem.setAttributeUint(TOTAL_COUNT, schemaBuilder->getKeyFragmentationTotalCount(fileIndex));
				fragmentElem.setAttributeUint(DEAD_COUNT, schemaBuilder->getKeyFragmentationDeadCount(fileIndex));
			}
		
			statElem.appendChild(&fragmentElem, &fragmentElem);
		}
};

template<typename K>
struct DeepSchema_v1 {
	public:
		static const String SCHEMA;
		static const String SCHEMA_PROTOCOL;
		static const String SCHEMA_VERSION;

		static const String MAP;
		static const String MAP_INDEX;
		static const String MAP_KEY_NAME;
		static const String MAP_LRT_INDEX;
		static const String MAP_LRT_POSITION;
		static const String MAP_KEY_SIZE;
		static const String MAP_VALUE_SIZE;
		static const String MAP_CHARACTER_SET;
		static const String MAP_IS_TEMPORARY;
		static const String MAP_INDEX_ORIENTATION;
		static const String MAP_KEY_COMPRESSION;
		static const String MAP_VALUE_COMPRESSION;
		static const String MAP_DATA_DIRECTORY;
		static const String MAP_INDEX_DIRECTORY;

		static const String KEY;
		static const String KEY_HAS_PRIMARY;

		static const String FIELD;
		static const String FIELD_NULL_BYTES;
		static const String FIELD_HAS_VARLEN_FIELDS;
		static const String FIELD_BLOB_START_OFFSET;
		static const String FIELD_BLOB_PTR_SIZE;
		static const String FIELD_BLOB_UNPACKED_ROW_LENGTH;
		static const String FIELD_AUTO_INCREMENT_VALUE;

		static const String TERM;
		static const String AUTOINC;
		static const String REMOVE_KEY;
		
	private:
		static boolean read(org::w3c::dom::Element& schemaElem, SchemaBuilder<K>* schemaBuilder, const char* mapKeyName) {

			inttype protocol = schemaElem.getAttributeInt(SCHEMA_PROTOCOL);
			// const char* version = schemaElem.getAttribute(SCHEMA_VERSION);

			org::w3c::dom::Element mapElem;
			schemaElem.getFirstChild(&mapElem);

			const char* keyName = mapElem.getAttribute(MAP_KEY_NAME);
			inttype lrtIndex = mapElem.getAttributeInt(MAP_LRT_INDEX);
			uinttype lrtPosition = mapElem.getAttributeUint(MAP_LRT_POSITION);
			inttype keySize = mapElem.getAttributeInt(MAP_KEY_SIZE);
			inttype valueSize = mapElem.getAttributeInt(MAP_VALUE_SIZE);
			inttype characterSet = mapElem.getAttributeInt(MAP_CHARACTER_SET);
			boolean isTemporary = mapElem.getAttributeBool(MAP_IS_TEMPORARY);
			uinttype indexOrientation = mapElem.getAttributeUint(MAP_INDEX_ORIENTATION);
			boolean keyCompression = mapElem.getAttributeBool(MAP_KEY_COMPRESSION);
			boolean valueCompression = mapElem.getAttributeBool(MAP_VALUE_COMPRESSION);
			const char* dataDir = mapElem.getAttribute(MAP_DATA_DIRECTORY);
			const char* indexDir = mapElem.getAttribute(MAP_INDEX_DIRECTORY);
			if (dataDir == null) {
				dataDir = "";
			}
			if (indexDir == null) {
				indexDir = "";
			}
			
			boolean keyMatches = strcmp(mapKeyName, keyName) == 0;
			
			org::w3c::dom::Element keyElem;
			mapElem.getFirstChild(&keyElem);

			boolean hasPrimary = keyElem.getAttributeBool(KEY_HAS_PRIMARY);
			
			org::w3c::dom::Element fieldElem;
			boolean hasFieldEntries = keyElem.getNextSibling(&fieldElem) != null;
			
			if (hasFieldEntries) {
				uinttype nullBytes = fieldElem.getAttributeUint(FIELD_NULL_BYTES);
				boolean hasVariableLengthFields = fieldElem.getAttributeBool(FIELD_HAS_VARLEN_FIELDS);
				uinttype blobStartOffset = fieldElem.getAttributeUint(FIELD_BLOB_START_OFFSET);
				uinttype blobPtrSize = fieldElem.getAttributeUint(FIELD_BLOB_PTR_SIZE);
				ulongtype unpackedRowLength = fieldElem.getAttribute(FIELD_BLOB_UNPACKED_ROW_LENGTH, ulongFromString);
				ulongtype autoIncrementValue = fieldElem.getAttribute(FIELD_AUTO_INCREMENT_VALUE, ulongFromString);
				
				schemaBuilder->setNullBytes(nullBytes);
				schemaBuilder->setHasVariableLengthFields(hasVariableLengthFields);
				schemaBuilder->setBlobStartOffset(blobStartOffset);
				schemaBuilder->setBlobPtrSize(blobPtrSize);
				schemaBuilder->setUnpackedRowLength(unpackedRowLength);
				schemaBuilder->setAutoIncrementValue(autoIncrementValue);
			}
			
			schemaBuilder->setKeyName(keyName);
			schemaBuilder->setLrtIndex(lrtIndex);
			schemaBuilder->setLrtPosition(lrtPosition);
			schemaBuilder->setCharacterSet(characterSet);
			schemaBuilder->setIsTemporary(isTemporary);
			schemaBuilder->setIndexOrientation((RealTime::IndexOrientation)indexOrientation);
			schemaBuilder->setKeyCompression(keyCompression);
			schemaBuilder->setValueCompression(valueCompression);
			schemaBuilder->setDirectoryPaths(File(dataDir), File(indexDir));
			schemaBuilder->setMapParams(protocol, keySize, valueSize, hasPrimary);
			

			if (hasFieldEntries) {
				FieldSchema_v1<K>::read(schemaElem, schemaBuilder);
			}
			
			if (keyMatches) {
				KeySchema_v1<K>::read(schemaElem, schemaBuilder);
			}
			
			return keyMatches;
		}

		FORCE_INLINE static void terminate(org::w3c::dom::Document* doc) {
			org::w3c::dom::Element termElem;
			doc->createElement(TERM, &termElem);
			doc->appendChild(&termElem, &termElem);
		}

	public:
		static longtype validate(RandomAccessFile* file) {
			static const String marker("<mark/>\n", 8);

			longtype pos = file->length();
			String str;
			nbyte buf(marker.length());
			while (pos > 0) {
				longtype seek = pos - buf.length;
				if (seek < 0) {
					seek = 0;
				}
				file->seek(seek);
				inttype count = file->read(&buf, 0, (inttype)(pos - seek));
				if (count > 0) {
					inttype prevlen = str.length();
					String read(&buf, 0, count);
					str = read+str;

					inttype match = str.indexOf(marker);
					if (match >= 0) {
						pos = pos + prevlen - str.length() + match + marker.length();
						break;
					}

					str = String(&buf, 0, count);
				}
				pos = seek;
			}

			if (pos < 0) {
				pos = 0;
			}
			return pos-1;
		}

		static SchemaBuilder<K>* read(RandomAccessFile* file, const char* keyName, boolean primary) {
			SchemaBuilder<K>* schemaBuilder = new SchemaBuilder<K>();
			
			org::w3c::dom::Document* doc = org::w3c::dom::DomUtil::readFile(file);
			org::w3c::dom::Element schemaElem;
			doc->getFirstChild(&schemaElem);

			boolean success = false;
			ulongtype autoinc = 1;
			do {
				if (strcmp(AUTOINC, schemaElem.getNodeName()) == 0) {
					autoinc = schemaElem.getAttribute(FIELD_AUTO_INCREMENT_VALUE, ulongFromString);
					continue;

				} else if (strcmp(TERM, schemaElem.getNodeName()) == 0) {
					continue;

				} else if (strcmp(REMOVE_KEY, schemaElem.getNodeName()) == 0) {
					const char* removeKeyName = schemaElem.getAttribute(MAP_KEY_NAME);
			
					if (strcmp(removeKeyName, keyName) == 0) {
						success = false;

						SchemaBuilder<K>* sb = new SchemaBuilder<K>();
						schemaBuilder->cloneTo(sb);

						delete schemaBuilder;
						schemaBuilder = sb;
					}

					continue;
				}

				if (success == false) {
					success = read(schemaElem, schemaBuilder, keyName);
				}

			} while (schemaElem.getNextSibling(&schemaElem) != null);
			
			delete doc;
			
			if (success == false) {
				delete schemaBuilder;
				schemaBuilder = null;

			} else if (primary == true) {
				schemaBuilder->setAutoIncrementValue(autoinc);
			}
			
			return schemaBuilder;
		}

		static void write(const SchemaBuilder<K>* schemaBuilder, RandomAccessFile* file, boolean primary) {

			if (primary) {
				if (file->length() != 0) {
					DEEP_LOG(ERROR, OTHER, "Invalid length for writing schema information: %s\n", file->getPath());

					throw InvalidException("Invalid length for writing schema information");
				}

				if (file->getFilePointer() != 0) {
					DEEP_LOG(ERROR, OTHER, "Invalid position for writing schema information: %s\n", file->getPath());

					throw InvalidException("Invalid position for writing schema information");
				}
			} else {
				if (file->length() == 0) {
					DEEP_LOG(ERROR, OTHER, "Invalid length for writing schema information: %s\n", file->getPath());

					throw InvalidException("Invalid length for writing schema information");
				}
				file->seek(0);
				
				SchemaBuilder<K>* existingSchema = RealTimeSchema_v1<K>::read(file, schemaBuilder->getKeyName(), primary);
				if (existingSchema != null) {
					inttype cmp = schemaBuilder->compare(existingSchema);
					delete existingSchema;
					if(cmp != 0) {
						DEEP_LOG(ERROR, OTHER, "Schema definition mismatch: %s\n", file->getPath());
						throw InvalidException("Schema definition mismatch");
					}
					
					return;
				}
				
				file->seek(file->length());
			}

			org::w3c::dom::Document* doc = org::w3c::dom::DomUtil::newDocument();

			org::w3c::dom::Element schemaElem;
			doc->createElement(SCHEMA, &schemaElem);

			schemaElem.setAttributeInt(SCHEMA_PROTOCOL, schemaBuilder->getKeyProtocol());
			// schema.setAttribute(SCHEMA_VERSION, "v1");

			doc->appendChild(&schemaElem, &schemaElem);

			org::w3c::dom::Element mapElem;
			doc->createElement(MAP, &mapElem);

			mapElem.setAttribute(MAP_KEY_NAME, schemaBuilder->getKeyName());
			mapElem.setAttributeInt(MAP_LRT_INDEX, schemaBuilder->getLrtIndex());
			mapElem.setAttributeUint(MAP_LRT_POSITION, schemaBuilder->getLrtPosition());
			mapElem.setAttributeInt(MAP_KEY_SIZE, schemaBuilder->getKeySize());
			mapElem.setAttributeInt(MAP_VALUE_SIZE, schemaBuilder->getValueSize());
			mapElem.setAttributeInt(MAP_CHARACTER_SET, schemaBuilder->getCharacterSet());
			mapElem.setAttributeBool(MAP_IS_TEMPORARY, schemaBuilder->isTemporary());
			mapElem.setAttributeUint(MAP_INDEX_ORIENTATION, schemaBuilder->getIndexOrientation());
			mapElem.setAttributeBool(MAP_KEY_COMPRESSION, schemaBuilder->getKeyCompression());
			mapElem.setAttributeBool(MAP_VALUE_COMPRESSION, schemaBuilder->getValueCompression());
			if (primary) {
				if (schemaBuilder->getDataDirectory().String::length() > 0) {
					mapElem.setAttribute(MAP_DATA_DIRECTORY, (const char*)schemaBuilder->getDataDirectory());
				}
				if (schemaBuilder->getIndexDirectory().String::length() > 0) {
					mapElem.setAttribute(MAP_INDEX_DIRECTORY, (const char*)schemaBuilder->getIndexDirectory());
				}
			}

			schemaElem.appendChild(&mapElem, &mapElem);

			org::w3c::dom::Element keyElem;
			doc->createElement(KEY, &keyElem);

			keyElem.setAttributeBool(KEY_HAS_PRIMARY, schemaBuilder->getHasPrimary());

			mapElem.appendChild(&keyElem, &keyElem);

			if (primary) {
				org::w3c::dom::Element fieldElem;
				doc->createElement(FIELD, &fieldElem);

				fieldElem.setAttributeUint(FIELD_NULL_BYTES, schemaBuilder->getNullBytes());
				fieldElem.setAttributeBool(FIELD_HAS_VARLEN_FIELDS, schemaBuilder->hasVariableLengthFields());
				fieldElem.setAttributeUint(FIELD_BLOB_START_OFFSET, schemaBuilder->getBlobStartOffset());
				fieldElem.setAttributeUint(FIELD_BLOB_PTR_SIZE, schemaBuilder->getBlobPtrSize());
				fieldElem.setAttribute<ulongtype>(FIELD_BLOB_UNPACKED_ROW_LENGTH, schemaBuilder->getUnpackedRowLength(), ulongToString);
				fieldElem.setAttribute<ulongtype>(FIELD_AUTO_INCREMENT_VALUE, schemaBuilder->getAutoIncrementValue(), ulongToString);

				mapElem.appendChild(&fieldElem, &fieldElem);
			}

			KeySchema_v1<K>::write(doc, schemaBuilder);
			if (primary) {
				FieldSchema_v1<K>::write(doc, schemaBuilder);
			}
		

			terminate(doc);

			org::w3c::dom::DomUtil::writeFile(doc, file);
			file->syncFilePointer();

			delete doc;
		}

		static void remove(const SchemaBuilder<K>* schemaBuilder, RandomAccessFile* file, boolean primary, const inttype lrtIndex, const uinttype lrtPosition) {

			if (primary) {
				DEEP_LOG(ERROR, OTHER, "Schema remove primary not supported: %s\n", file->getPath());

				throw InvalidException("Schema remove primary not supported");
			}

			if (file->length() == 0) {
				DEEP_LOG(ERROR, OTHER, "Invalid length for removing schema information: %s\n", file->getPath());

				throw InvalidException("Invalid length for removing schema information");
			}

			file->seek(0);
				
			SchemaBuilder<K>* existingSchema = RealTimeSchema_v1<K>::read(file, schemaBuilder->getKeyName(), primary);
			if (existingSchema != null) {
				file->seek(file->length());

				org::w3c::dom::Document* doc = org::w3c::dom::DomUtil::newDocument();

				org::w3c::dom::Element removeElem;
				doc->createElement(REMOVE_KEY, &removeElem);

				removeElem.setAttribute(MAP_KEY_NAME, schemaBuilder->getKeyName());
				removeElem.setAttributeInt(MAP_LRT_INDEX, lrtIndex);
				removeElem.setAttributeInt(MAP_LRT_POSITION, lrtPosition);

				doc->appendChild(&removeElem, &removeElem);

				terminate(doc);

				org::w3c::dom::DomUtil::writeFile(doc, file);
				file->syncFilePointer();

				delete doc;
				delete existingSchema;
			}
		}

		static void updateAutoIncrementValue(const SchemaBuilder<K>* schemaBuilder, RandomAccessFile* file) {

			org::w3c::dom::Document* doc = org::w3c::dom::DomUtil::newDocument();

			if (file->length() == 0) {
				DEEP_LOG(ERROR, OTHER, "Invalid length for writing schema information: %s\n", file->getPath());

				throw InvalidException("Invalid length for writing schema information");
			}
			file->seek(file->length());

			org::w3c::dom::Element aiElem;
			doc->createElement(AUTOINC, &aiElem);

			aiElem.setAttribute<ulongtype>(FIELD_AUTO_INCREMENT_VALUE, schemaBuilder->getAutoIncrementValue(), ulongToString);

			doc->appendChild(&aiElem, &aiElem);
			terminate(doc);
			org::w3c::dom::DomUtil::writeFile(doc, file);
			file->syncFilePointer();

			delete doc;
		}

	private:
		static ulongtype ulongFromString(const char* value) {
			ulongtype ret;
			
			if (sscanf(value, "%llu", &ret) != 1) {
				DEEP_LOG(ERROR, OTHER, "Error parsing schema attribute");
				throw IOException("Error parsing schema attribute");
			}
			
			return ret;
		}
		
		static const char* ulongToString(ulongtype value) {
			char* s = new char[sizeof(uchartype*)*2+1];
			
			if (snprintf(s, sizeof(uchartype*)*2+1, "%llu", value) <= 0) {
				delete[] s;
				DEEP_LOG(ERROR, OTHER, "Error serializing schema attribute");
				throw IOException("Error serializing schema attribute");
			}
			
			return s;
		}
};

template<>
struct RealTimeSchema_v1<DeepLongInt> : public DeepSchema_v1<DeepLongInt> {
};

template<>
struct RealTimeSchema_v1<DeepULongInt> : public DeepSchema_v1<DeepULongInt> {
};

template<>
struct RealTimeSchema_v1<DeepLongLong> : public DeepSchema_v1<DeepLongLong> {
};

template<>
struct RealTimeSchema_v1<DeepULongLong> : public DeepSchema_v1<DeepULongLong> {
};

#ifndef DEEP_REDUCE_TEMPLATES
template<>
struct RealTimeSchema_v1<DeepShortInt> : public DeepSchema_v1<DeepShortInt> {
};

template<>
struct RealTimeSchema_v1<DeepUShortInt> : public DeepSchema_v1<DeepUShortInt> {
};

template<>
struct RealTimeSchema_v1<DeepTinyInt> : public DeepSchema_v1<DeepTinyInt> {
};

template<>
struct RealTimeSchema_v1<DeepUTinyInt> : public DeepSchema_v1<DeepUTinyInt> {
};

template<>
struct RealTimeSchema_v1<DeepDouble> : public DeepSchema_v1<DeepDouble> {
};

template<>
struct RealTimeSchema_v1<DeepFloat> : public DeepSchema_v1<DeepFloat> {
};
#endif

template<>
struct RealTimeSchema_v1<DeepNByte*> : public DeepSchema_v1<DeepNByte*> {
};

template<>
struct RealTimeSchema_v1<DeepComposite*> : public DeepSchema_v1<DeepComposite*> {
};

template<typename K> const String DeepSchema_v1<K>::SCHEMA                 = "schema";
template<typename K> const String DeepSchema_v1<K>::SCHEMA_PROTOCOL        = "protocol";
template<typename K> const String DeepSchema_v1<K>::SCHEMA_VERSION         = "version";

template<typename K> const String DeepSchema_v1<K>::MAP                    = "map";
template<typename K> const String DeepSchema_v1<K>::MAP_KEY_NAME           = "key_name";
template<typename K> const String DeepSchema_v1<K>::MAP_LRT_INDEX	    = "lrt_index";
template<typename K> const String DeepSchema_v1<K>::MAP_LRT_POSITION	    = "lrt_position";
template<typename K> const String DeepSchema_v1<K>::MAP_KEY_SIZE           = "key_size";
template<typename K> const String DeepSchema_v1<K>::MAP_VALUE_SIZE         = "value_size";
template<typename K> const String DeepSchema_v1<K>::MAP_CHARACTER_SET      = "map_charset";
template<typename K> const String DeepSchema_v1<K>::MAP_IS_TEMPORARY       = "is_temp";
template<typename K> const String DeepSchema_v1<K>::MAP_INDEX_ORIENTATION  = "index_orientation";
template<typename K> const String DeepSchema_v1<K>::MAP_KEY_COMPRESSION    = "key_compression";
template<typename K> const String DeepSchema_v1<K>::MAP_VALUE_COMPRESSION  = "value_compression";
template<typename K> const String DeepSchema_v1<K>::MAP_DATA_DIRECTORY     = "data_dir";
template<typename K> const String DeepSchema_v1<K>::MAP_INDEX_DIRECTORY    = "index_dir";

template<typename K> const String DeepSchema_v1<K>::KEY                    = "key";
template<typename K> const String DeepSchema_v1<K>::KEY_HAS_PRIMARY        = "has_primary";

template<typename K> const String DeepSchema_v1<K>::FIELD                          = "field";
template<typename K> const String DeepSchema_v1<K>::FIELD_NULL_BYTES               = "null_bytes";
template<typename K> const String DeepSchema_v1<K>::FIELD_HAS_VARLEN_FIELDS        = "has_varlen_fields";
template<typename K> const String DeepSchema_v1<K>::FIELD_BLOB_START_OFFSET        = "blob_start_offset";
template<typename K> const String DeepSchema_v1<K>::FIELD_BLOB_PTR_SIZE            = "blob_ptr_size";
template<typename K> const String DeepSchema_v1<K>::FIELD_BLOB_UNPACKED_ROW_LENGTH = "blob_unpacked_row_length";
template<typename K> const String DeepSchema_v1<K>::FIELD_AUTO_INCREMENT_VALUE     = "auto_increment_value";

template<typename K> const String DeepSchema_v1<K>::TERM                    = "mark";
template<typename K> const String DeepSchema_v1<K>::AUTOINC                 = "auto_increment";
template<typename K> const String DeepSchema_v1<K>::REMOVE_KEY              = "remove_key";

template<>
struct KeySchema_v1<DeepLongInt> : public DeepKeySchema_v1<DeepLongInt> {
};

template<>
struct KeySchema_v1<DeepULongInt> : public DeepKeySchema_v1<DeepULongInt> {
};

template<>
struct KeySchema_v1<DeepShortInt> : public DeepKeySchema_v1<DeepShortInt> {
};

template<>
struct KeySchema_v1<DeepUShortInt> : public DeepKeySchema_v1<DeepUShortInt> {
};

template<>
struct KeySchema_v1<DeepLongLong> : public DeepKeySchema_v1<DeepLongLong> {
};

template<>
struct KeySchema_v1<DeepULongLong> : public DeepKeySchema_v1<DeepULongLong> {
};

template<>
struct KeySchema_v1<DeepTinyInt> : public DeepKeySchema_v1<DeepTinyInt> {
};

template<>
struct KeySchema_v1<DeepUTinyInt> : public DeepKeySchema_v1<DeepUTinyInt> {
};

template<>
struct KeySchema_v1<DeepDouble> : public DeepKeySchema_v1<DeepDouble> {
};

template<>
struct KeySchema_v1<DeepFloat> : public DeepKeySchema_v1<DeepFloat> {
};

template<>
struct KeySchema_v1<DeepNByte*> : public DeepKeySchema_v1<DeepNByte*> {
};

template<>
struct KeySchema_v1<DeepComposite*> : public DeepKeySchema_v1<DeepComposite*> {
};

template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_LIST          = "key_part_list";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART               = "key_part";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_FIELD_INDEX   = "field_index";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_TYPE          = "type";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_LENGTH        = "length";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_VALUE_OFFSET  = "value_offset";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_NULL_OFFSET   = "null_offset";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_NULL_BIT      = "null_bit";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_RESERVED      = "reserved";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_IGNORED       = "ignored";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_VARIABLE_POSITION = "variable_position";
template<typename K> const String DeepKeySchema_v1<K>::KEY_PART_PRIMARY_POSITION  = "primary_position";

template<>
struct FieldSchema_v1<DeepLongInt> : public DeepFieldSchema_v1<DeepLongInt> {
};

template<>
struct FieldSchema_v1<DeepULongInt> : public DeepFieldSchema_v1<DeepULongInt> {
};

template<>
struct FieldSchema_v1<DeepShortInt> : public DeepFieldSchema_v1<DeepShortInt> {
};

template<>
struct FieldSchema_v1<DeepUShortInt> : public DeepFieldSchema_v1<DeepUShortInt> {
};

template<>
struct FieldSchema_v1<DeepLongLong> : public DeepFieldSchema_v1<DeepLongLong> {
};

template<>
struct FieldSchema_v1<DeepULongLong> : public DeepFieldSchema_v1<DeepULongLong> {
};

template<>
struct FieldSchema_v1<DeepTinyInt> : public DeepFieldSchema_v1<DeepTinyInt> {
};

template<>
struct FieldSchema_v1<DeepUTinyInt> : public DeepFieldSchema_v1<DeepUTinyInt> {
};

template<>
struct FieldSchema_v1<DeepDouble> : public DeepFieldSchema_v1<DeepDouble> {
};

template<>
struct FieldSchema_v1<DeepFloat> : public DeepFieldSchema_v1<DeepFloat> {
};

template<>
struct FieldSchema_v1<DeepNByte*> : public DeepFieldSchema_v1<DeepNByte*> {
};

template<>
struct FieldSchema_v1<DeepComposite*> : public DeepFieldSchema_v1<DeepComposite*> {
};

template<typename K> const String DeepFieldSchema_v1<K>::FIELD_LIST                    = "field_list";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD                         = "field";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_TYPE                    = "field_type";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_REAL_TYPE               = "field_real_type";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_PACK_LENGTH             = "field_pack_length";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_ROW_PACK_LENGTH         = "field_row_pack_length";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_KEY_LENGTH              = "field_key_length";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_LENGTH_BYTES            = "field_length_bytes";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_INDEX                   = "field_index";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_NULL_BIT                = "field_null_bit";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_NULL_OFFSET             = "field_null_offset";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_VALUE_OFFSET            = "field_value_offset";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_CHARACTER_SET           = "field_charset";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_GCOL_VIRTUAL            = "field_gcol_virtual";
template<typename K> const String DeepFieldSchema_v1<K>::FIELD_NAME					   = "field_name";
} } } } } } // namespace

#endif //COM_DEEPIS_DATASTORE_API_DEEP_SCHEMA_H_
