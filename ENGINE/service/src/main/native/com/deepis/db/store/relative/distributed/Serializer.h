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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_SERIALIZER_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_SERIALIZER_H_

#include "cxx/lang/types.h"
#include "cxx/util/Converter.h"
#include "cxx/io/EncodeProtocol.h"
#include "com/deepis/db/store/relative/util/InvalidException.h"
#include "com/deepis/db/store/relative/distributed/BaseMessageData.h"
#include "com/deepis/db/store/relative/distributed/StatusRequest.h"
#include "com/deepis/db/store/relative/distributed/DataRequest.h"
#include "com/deepis/db/store/relative/distributed/VirtualInfo.h"
#include "com/deepis/db/store/relative/distributed/LogEntry.h"
#include "com/deepis/db/store/relative/distributed/QuickList.h"
#include "com/deepis/db/store/relative/distributed/PeerInfo.h"
#include "com/deepis/db/store/relative/distributed/MapBehavior.h"
#include "com/deepis/db/store/relative/distributed/IMessageService.h"

using namespace cxx::lang;
using namespace cxx::util;
using namespace cxx::io;
using namespace com::deepis::db::store::relative::util;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

template<typename K>
class Serializer {
	public:

		// XXX: Base
		const static uinttype SERVER_ID = 1;
		const static uinttype THREAD_ID = 2;
		const static uinttype TABLE_ID = 3;

		FORCE_INLINE static encodeProtocol::writer* serializeBaseMessage(IMessageService* messageService, uinttype messageId, uinttype serverId, uinttype tableId, longtype threadId) {
			BaseMessageData baseMessageData(serverId, tableId, threadId);
			return serializeBaseMessage(messageService, messageId, &baseMessageData);
		}

		FORCE_INLINE static encodeProtocol::writer* serializeBaseMessage(IMessageService* messageService, uinttype messageId, BaseMessageData* baseMessageData = null) {
			uinttype size = messageService->getBaseMessageSize(baseMessageData);

			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(messageId), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData); 	

			return encodeWriter;
		}
		
		// XXX: StatusRequest
		const static uinttype MSG_ID_STATUS_REQUEST = 0x00000001;
		const static uinttype STATUS_REQUEST_KEY = 4;
		const static uinttype STATUS_REQUEST_MAX_VIEWPOINT = 5;

		FORCE_INLINE static encodeProtocol::writer* serialize(IMessageService* messageService, StatusRequest<K>* statusRequest, BaseMessageData* baseMessageData = null) {
			uinttype size = messageService->getBaseMessageSize(baseMessageData) + 
			Converter<K>::encodedSize(statusRequest->getKey()) +
			encodeProtocol::writer::sizeOfUint32(statusRequest->getMaxViewpoint());

			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(MSG_ID_STATUS_REQUEST), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData);
			Converter<K>::encode(encodeWriter, STATUS_REQUEST_KEY, statusRequest->getKey());
			encodeWriter->setUint32Field(STATUS_REQUEST_MAX_VIEWPOINT, statusRequest->getMaxViewpoint());

			return encodeWriter;
		}

		FORCE_INLINE static void deserialize(StatusRequest<K>* statusRequest, ubytetype* data, uinttype dataSize) {
			encodeProtocol::reader encodeReader(data, dataSize);
			
			encodeReader.decodeFields();

			K key = Converter<K>::decode(&encodeReader, STATUS_REQUEST_KEY);

			uinttype maxViewpoint = 0;
			encodeReader.getUint32Field(STATUS_REQUEST_MAX_VIEWPOINT, maxViewpoint);

			statusRequest->setKey(key);
			statusRequest->setMaxViewpoint(maxViewpoint);
		}

		FORCE_INLINE static void deserialize(encodeProtocol::reader* encodeReader, StatusRequest<K>* statusRequest) {
			K key = Converter<K>::decode(encodeReader, STATUS_REQUEST_KEY);

			uinttype maxViewpoint = 0;
			encodeReader->getUint32Field(STATUS_REQUEST_MAX_VIEWPOINT, maxViewpoint);

			statusRequest->setKey(key);
			statusRequest->setMaxViewpoint(maxViewpoint);
		}


		// XXX: StatusResponse
		const static uinttype MSG_ID_STATUS_RESPONSE = 0x00000002;
		const static uinttype STATUS_RESPONSE_MAX_VIEWPOINT = 4;

		FORCE_INLINE static encodeProtocol::writer* serialize(IMessageService* messageService, StatusResponse* statusResponse, BaseMessageData* baseMessageData = null) {
			uinttype size = messageService->getBaseMessageSize(baseMessageData) + 
			encodeProtocol::writer::sizeOfUint32(statusResponse->getMaxViewpoint());

			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(MSG_ID_STATUS_RESPONSE), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData);
			encodeWriter->setUint32Field(STATUS_RESPONSE_MAX_VIEWPOINT, statusResponse->getMaxViewpoint());

			return encodeWriter;
		}

		FORCE_INLINE static void deserialize(StatusResponse* statusResponse, ubytetype* data, uinttype dataSize) {
			encodeProtocol::reader encodeReader(data, dataSize);
			
			encodeReader.decodeFields();

			uinttype maxViewpoint = 0;
			encodeReader.getUint32Field(STATUS_RESPONSE_MAX_VIEWPOINT, maxViewpoint);
			statusResponse->setMaxViewpoint(maxViewpoint);
		}

		FORCE_INLINE static void deserialize(encodeProtocol::reader* encodeReader, StatusResponse* statusResponse) {
			uinttype maxViewpoint = 0;
			encodeReader->getUint32Field(STATUS_RESPONSE_MAX_VIEWPOINT, maxViewpoint);
			statusResponse->setMaxViewpoint(maxViewpoint);
		}

		// XXX: DataReqest
		const static uinttype MSG_ID_DATA_REQUEST = 0x00000003;
		const static uinttype DATA_REQUEST_KEY = 4;
		const static uinttype DATA_REQUEST_REQUEST_TYPE = 5;
		const static uinttype DATA_REQUEST_VIRTUAL_SIZE = 6;
		const static uinttype DATA_REQUEST_MAX_VIRTUAL_VIEWPOINT = 7;
		const static uinttype DATA_REQUEST_MAX_REAL_VIEWPOINT = 8;

		FORCE_INLINE static encodeProtocol::writer* serialize(IMessageService* messageService, DataRequest<K>* dataRequest, BaseMessageData* baseMessageData = null) {
			uinttype size = messageService->getBaseMessageSize(baseMessageData) + 
			Converter<K>::encodedSize(dataRequest->getKey()) +
			encodeProtocol::writer::sizeOfUint32((uinttype)(dataRequest->getRequestType())) +
			encodeProtocol::writer::sizeOfInt32(dataRequest->getVirtualSize()) +
			encodeProtocol::writer::sizeOfUint32(dataRequest->getMaxVirtualViewpoint()) +
			encodeProtocol::writer::sizeOfUint32(dataRequest->getMaxRealViewpoint());
			
			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(MSG_ID_DATA_REQUEST), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData);
			Converter<K>::encode(encodeWriter, DATA_REQUEST_KEY, dataRequest->getKey());
			encodeWriter->setUint32Field(DATA_REQUEST_REQUEST_TYPE, (uinttype)(dataRequest->getRequestType()));
			encodeWriter->setInt32Field(DATA_REQUEST_VIRTUAL_SIZE, dataRequest->getVirtualSize());
			encodeWriter->setUint32Field(DATA_REQUEST_MAX_VIRTUAL_VIEWPOINT, dataRequest->getMaxVirtualViewpoint());
			encodeWriter->setUint32Field(DATA_REQUEST_MAX_REAL_VIEWPOINT, dataRequest->getMaxRealViewpoint());

			return encodeWriter;
		}

		FORCE_INLINE static void deserialize(DataRequest<K>* dataRequest, ubytetype* data, uinttype dataSize) {
			encodeProtocol::reader encodeReader(data, dataSize);
			
			encodeReader.decodeFields();

			K key = Converter<K>::decode(&encodeReader, DATA_REQUEST_KEY);

			uinttype requestType = 0;
			inttype virtualSize = 0;
			uinttype maxVirtualViewpoint = 0;
			uinttype maxRealViewpoint = 0;

			encodeReader.getUint32Field(DATA_REQUEST_REQUEST_TYPE, requestType);
			encodeReader.getInt32Field(DATA_REQUEST_VIRTUAL_SIZE, virtualSize);
			encodeReader.getUint32Field(DATA_REQUEST_MAX_VIRTUAL_VIEWPOINT, maxVirtualViewpoint);
			encodeReader.getUint32Field(DATA_REQUEST_MAX_REAL_VIEWPOINT, maxRealViewpoint);

			dataRequest->setKey(key);
			dataRequest->setRequestType((DataRequestType)requestType);
			dataRequest->setVirtualSize(virtualSize);
			dataRequest->setMaxVirtualViewpoint(maxVirtualViewpoint);
			dataRequest->setMaxRealViewpoint(maxRealViewpoint);
		}

		FORCE_INLINE static void deserialize(encodeProtocol::reader* encodeReader, DataRequest<K>* dataRequest) {
			K key = Converter<K>::decode(encodeReader, DATA_REQUEST_KEY);

			uinttype requestType = 0;
			inttype virtualSize = 0;
			uinttype maxVirtualViewpoint = 0;
			uinttype maxRealViewpoint = 0;

			encodeReader->getUint32Field(DATA_REQUEST_REQUEST_TYPE, requestType);
			encodeReader->getInt32Field(DATA_REQUEST_VIRTUAL_SIZE, virtualSize);
			encodeReader->getUint32Field(DATA_REQUEST_MAX_VIRTUAL_VIEWPOINT, maxVirtualViewpoint);
			encodeReader->getUint32Field(DATA_REQUEST_MAX_REAL_VIEWPOINT, maxRealViewpoint);

			dataRequest->setKey(key);
			dataRequest->setRequestType((DataRequestType)requestType);
			dataRequest->setVirtualSize(virtualSize);
			dataRequest->setMaxVirtualViewpoint(maxVirtualViewpoint);
			dataRequest->setMaxRealViewpoint(maxRealViewpoint);
		}

		// XXX: VirtualInfo
		const static uinttype MSG_ID_VIRTUAL_INFO = 0x00000004;
		const static uinttype VIRTUAL_INFO_VIRTUAL_SIZE = 4;
		const static uinttype VIRTUAL_INFO_MIN_VIEWPOINT = 5;
		const static uinttype VIRTUAL_INFO_MAX_VIEWPOINT = 6;
		const static uinttype VIRTUAL_INFO_VERSION = 7;

		FORCE_INLINE static encodeProtocol::writer* serialize(IMessageService* messageService, VirtualInfo* virtualInfo, BaseMessageData* baseMessageData = null) {
			uinttype size = messageService->getBaseMessageSize(baseMessageData) + 
			encodeProtocol::writer::sizeOfInt32(virtualInfo->getVirtualSize()) +
			encodeProtocol::writer::sizeOfUint32(virtualInfo->getMinViewpoint()) +
			encodeProtocol::writer::sizeOfUint32(virtualInfo->getMaxViewpoint()) +
			encodeProtocol::writer::sizeOfInt64(virtualInfo->getVersion());
			
			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(MSG_ID_VIRTUAL_INFO), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData);
			encodeWriter->setInt32Field(VIRTUAL_INFO_VIRTUAL_SIZE, virtualInfo->getVirtualSize());
			encodeWriter->setUint32Field(VIRTUAL_INFO_MIN_VIEWPOINT, virtualInfo->getMinViewpoint());
			encodeWriter->setUint32Field(VIRTUAL_INFO_MAX_VIEWPOINT, virtualInfo->getMaxViewpoint());
			encodeWriter->setInt64Field(VIRTUAL_INFO_VERSION, virtualInfo->getVersion());

			return encodeWriter;
		}

		FORCE_INLINE static void deserialize(VirtualInfo* virtualInfo, ubytetype* data, uinttype dataSize) {
			encodeProtocol::reader encodeReader(data, dataSize);
			
			encodeReader.decodeFields();

			inttype virtualSize = 0;
			uinttype minViewpoint = 0;
			uinttype maxViewpoint = 0;
			longtype version = 0;

			encodeReader.getInt32Field(VIRTUAL_INFO_VIRTUAL_SIZE, virtualSize);
			encodeReader.getUint32Field(VIRTUAL_INFO_MIN_VIEWPOINT, minViewpoint);
			encodeReader.getUint32Field(VIRTUAL_INFO_MAX_VIEWPOINT, maxViewpoint);
			encodeReader.getInt64Field(VIRTUAL_INFO_VERSION, version);

			virtualInfo->setVirtualSize(virtualSize);
			virtualInfo->setMinViewpoint(minViewpoint);
			virtualInfo->setMaxViewpoint(maxViewpoint);
			virtualInfo->setVersion(version);
		}

		FORCE_INLINE static void deserialize(encodeProtocol::reader* encodeReader, VirtualInfo* virtualInfo) {
			inttype virtualSize = 0;
			uinttype minViewpoint = 0;
			uinttype maxViewpoint = 0;
			longtype version = 0;

			encodeReader->getInt32Field(VIRTUAL_INFO_VIRTUAL_SIZE, virtualSize);
			encodeReader->getUint32Field(VIRTUAL_INFO_MIN_VIEWPOINT, minViewpoint);
			encodeReader->getUint32Field(VIRTUAL_INFO_MAX_VIEWPOINT, maxViewpoint);
			encodeReader->getInt64Field(VIRTUAL_INFO_VERSION, version);

			virtualInfo->setVirtualSize(virtualSize);
			virtualInfo->setMinViewpoint(minViewpoint);
			virtualInfo->setMaxViewpoint(maxViewpoint);
			virtualInfo->setVersion(version);
		}

		// XXX: DataLogEntry
		const static uinttype MSG_ID_DATA_LOG_ENTRY = 0x00000005;
		const static uinttype DATA_LOG_ENTRY_KEY = 4;
		const static uinttype DATA_LOG_ENTRY_ACTION = 5;
		const static uinttype DATA_LOG_ENTRY_VIEWPOINT = 6;
		const static uinttype DATA_LOG_ENTRY_DATA = 7;
		
		FORCE_INLINE static encodeProtocol::writer* serialize(IMessageService* messageService, DataLogEntry<K>* dataLogEntry, BaseMessageData* baseMessageData = null) {
			uinttype size = messageService->getBaseMessageSize(baseMessageData) + 
			Converter<K>::encodedSize(dataLogEntry->getKey()) +
			encodeProtocol::writer::sizeOfUint32((uinttype)(dataLogEntry->getAction())) +
			encodeProtocol::writer::sizeOfUint32(dataLogEntry->getViewpoint()) +
			encodeProtocol::writer::sizeOfRaw(dataLogEntry->getSize());
			
			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(MSG_ID_DATA_LOG_ENTRY), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData);
			Converter<K>::encode(encodeWriter, DATA_LOG_ENTRY_KEY, dataLogEntry->getKey());
			encodeWriter->setUint32Field(DATA_LOG_ENTRY_ACTION, (uinttype)(dataLogEntry->getAction()));
			encodeWriter->setUint32Field(DATA_LOG_ENTRY_VIEWPOINT, dataLogEntry->getViewpoint());
			encodeWriter->setRawField(DATA_LOG_ENTRY_DATA, (ubytetype*) dataLogEntry->getData(), dataLogEntry->getSize());

			return encodeWriter;
		}

		FORCE_INLINE static void deserialize(DataLogEntry<K>* dataLogEntry, ubytetype* data, uinttype dataSize) {
			encodeProtocol::reader encodeReader(data, dataSize);
			
			encodeReader.decodeFields();

			K key = Converter<K>::decode(&encodeReader, DATA_LOG_ENTRY_KEY);

			uinttype action = 0;
			uinttype viewpoint = 0;
			const ubytetype* dataField = null;
			uinttype dataFieldSize = 0;

			encodeReader.getUint32Field(DATA_LOG_ENTRY_ACTION, action);
			encodeReader.getUint32Field(DATA_LOG_ENTRY_VIEWPOINT, viewpoint);
			encodeReader.getRawFieldRef(DATA_LOG_ENTRY_DATA, dataField, dataFieldSize);

			ubytetype* dataValue = (ubytetype*) malloc(dataFieldSize);
			memcpy(dataValue, dataField, dataFieldSize);

			dataLogEntry->setKey(key);
			dataLogEntry->setAction((LogEntryAction)action);
			dataLogEntry->setViewpoint(viewpoint);
			dataLogEntry->setData((bytearray)dataValue);
			dataLogEntry->setSize(dataFieldSize);
		}

		FORCE_INLINE static void deserialize(encodeProtocol::reader* encodeReader, DataLogEntry<K>* dataLogEntry) {
			K key = Converter<K>::decode(encodeReader, DATA_LOG_ENTRY_KEY);

			uinttype action = 0;
			uinttype viewpoint = 0;
			const ubytetype* dataField = null;
			uinttype dataFieldSize = 0;

			encodeReader->getUint32Field(DATA_LOG_ENTRY_ACTION, action);
			encodeReader->getUint32Field(DATA_LOG_ENTRY_VIEWPOINT, viewpoint);
			encodeReader->getRawFieldRef(DATA_LOG_ENTRY_DATA, dataField, dataFieldSize);

			ubytetype* dataValue = (ubytetype*) malloc(dataFieldSize);
			memcpy(dataValue, dataField, dataFieldSize);

			dataLogEntry->setKey(key);
			dataLogEntry->setAction((LogEntryAction)action);
			dataLogEntry->setViewpoint(viewpoint);
			dataLogEntry->setData((bytearray)dataValue);
			dataLogEntry->setSize(dataFieldSize);
		}

		// XXX: DataLogEntryBlock
		const static uinttype MSG_ID_DATA_LOG_ENTRY_BLOCK = 0x00000006;
		const static uinttype DATA_LOG_ENTRY_BLOCK_ENTRY_COUNT = 4;
		const static uinttype DATA_LOG_ENTRY_BLOCK = 5;

		FORCE_INLINE static encodeProtocol::writer* serialize(IMessageService* messageService, QuickList<DataLogEntry<K>*>* dataLogEntries, BaseMessageData* baseMessageData = null) {
			uinttype entryCount = dataLogEntries->getSize();
			QuickListEntry<DataLogEntry<K>*>* listEntry = dataLogEntries->getHead();
			DataLogEntry<K>* entry = null;
			uinttype entrySize = 0;
			ubytetype* bytes = null;
			ubytetype* block = null;
			uinttype pointer = 0;

			do {
				entry = listEntry->getEntry();
				bytes = entry->getBytes(&entrySize);

				if (block == null) {
					block = (ubytetype*) malloc(entryCount * entrySize);
				}

				// TODO: handle resize

				memcpy((block + pointer), bytes, entrySize);
				pointer += entrySize;

				free(bytes);

				listEntry = listEntry->getNext();

			} while (listEntry != null);

			uinttype size = messageService->getBaseMessageSize(baseMessageData) + 
			encodeProtocol::writer::sizeOfUint32(entryCount) +
			encodeProtocol::writer::sizeOfRaw(pointer);
			
			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(MSG_ID_DATA_LOG_ENTRY_BLOCK), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData);
			encodeWriter->setUint32Field(DATA_LOG_ENTRY_BLOCK_ENTRY_COUNT, entryCount);
			encodeWriter->setRawField(DATA_LOG_ENTRY_BLOCK, block, pointer);

			free(block);

			return encodeWriter;
		}

		FORCE_INLINE static void deserialize(QuickList<DataLogEntry<K>*>* dataLogEntries, ubytetype* data, uinttype dataSize) {
			encodeProtocol::reader encodeReader(data, dataSize);
			
			encodeReader.decodeFields();

			uinttype entryCount = 0;
			const ubytetype* block = null;
			uinttype blockSize = 0;

			encodeReader.getUint32Field(DATA_LOG_ENTRY_BLOCK_ENTRY_COUNT, entryCount);
			encodeReader.getRawFieldRef(DATA_LOG_ENTRY_BLOCK, block, blockSize);

			uinttype pointer = 0;
			for (uinttype x = 0; x < entryCount; x++) {
				DataLogEntry<K>* logEntry = new DataLogEntry<K>();
				pointer += logEntry->setBytes((block + pointer));
				
				dataLogEntries->addEntry(logEntry);
			}

			delete block;
		}

		FORCE_INLINE static void deserialize(encodeProtocol::reader* encodeReader, QuickList<DataLogEntry<K>*>* dataLogEntries) {
			uinttype entryCount = 0;
			const ubytetype* block = null;
			uinttype blockSize = 0;

			encodeReader->getUint32Field(DATA_LOG_ENTRY_BLOCK_ENTRY_COUNT, entryCount);
			encodeReader->getRawFieldRef(DATA_LOG_ENTRY_BLOCK, block, blockSize);

			uinttype pointer = 0;
			for (uinttype x = 0; x < entryCount; x++) {
				DataLogEntry<K>* logEntry = new DataLogEntry<K>();
				pointer += logEntry->setBytes((block + pointer));
				
				dataLogEntries->addEntry(logEntry);
			}

			// XXX: this is deleted as part of the message
			//delete block;
		}

		// XXX: VirtualLogEntry
		const static uinttype MSG_ID_VIRTUAL_LOG_ENTRY = 0x00000007;
		const static uinttype VIRTUAL_LOG_ENTRY_KEY = 4;
		const static uinttype VIRTUAL_LOG_ENTRY_ACTION = 5;
		// XXX: processed does not need to be encoded
		#if 0
		const static uinttype VIRTUAL_LOG_ENTRY_PROCESSED = 5;
		#endif
		const static uinttype VIRTUAL_LOG_ENTRY_VIRTUAL_SIZE = 6;
		const static uinttype VIRTUAL_LOG_ENTRY_MIN_VIEWPOINT = 7;
		const static uinttype VIRTUAL_LOG_ENTRY_MAX_VIEWPOINT = 8;
		
		FORCE_INLINE static encodeProtocol::writer* serialize(IMessageService* messageService, VirtualLogEntry<K>* virtualLogEntry, BaseMessageData* baseMessageData = null) {
			uinttype size = messageService->getBaseMessageSize(baseMessageData) + 
			Converter<K>::encodedSize(virtualLogEntry->getKey()) +
			encodeProtocol::writer::sizeOfUint32((uinttype)(virtualLogEntry->getAction())) +
			encodeProtocol::writer::sizeOfInt32(virtualLogEntry->getVirtualSize()) +
			encodeProtocol::writer::sizeOfUint32(virtualLogEntry->getMinViewpoint()) +
			encodeProtocol::writer::sizeOfUint32(virtualLogEntry->getMaxViewpoint());
			
			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(MSG_ID_VIRTUAL_LOG_ENTRY), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData);
			Converter<K>::encode(encodeWriter, VIRTUAL_LOG_ENTRY_KEY, virtualLogEntry->getKey());
			encodeWriter->setUint32Field(VIRTUAL_LOG_ENTRY_ACTION, (uinttype)(virtualLogEntry->getAction()));
			encodeWriter->setInt32Field(VIRTUAL_LOG_ENTRY_VIRTUAL_SIZE, virtualLogEntry->getVirtualSize());
			encodeWriter->setUint32Field(VIRTUAL_LOG_ENTRY_MIN_VIEWPOINT, virtualLogEntry->getMinViewpoint());
			encodeWriter->setUint32Field(VIRTUAL_LOG_ENTRY_MAX_VIEWPOINT, virtualLogEntry->getMaxViewpoint());

			return encodeWriter;
		}

		FORCE_INLINE static void deserialize(VirtualLogEntry<K>* virtualLogEntry, ubytetype* data, uinttype dataSize) {
			encodeProtocol::reader encodeReader(data, dataSize);
			
			encodeReader.decodeFields();

			K key = Converter<K>::decode(&encodeReader, VIRTUAL_LOG_ENTRY_KEY);

			uinttype action = 0;
			inttype virtualSize = 0;
			uinttype minViewpoint = 0;
			uinttype maxViewpoint = 0;

			encodeReader.getUint32Field(VIRTUAL_LOG_ENTRY_ACTION, action);
			encodeReader.getInt32Field(VIRTUAL_LOG_ENTRY_VIRTUAL_SIZE, virtualSize);
			encodeReader.getUint32Field(VIRTUAL_LOG_ENTRY_MIN_VIEWPOINT, minViewpoint);
			encodeReader.getUint32Field(VIRTUAL_LOG_ENTRY_MAX_VIEWPOINT, maxViewpoint);

			virtualLogEntry->setKey(key);
			virtualLogEntry->setAction((LogEntryAction)action);
			virtualLogEntry->setVirtualSize(virtualSize);
			virtualLogEntry->setMinViewpoint(minViewpoint);
			virtualLogEntry->setMaxViewpoint(maxViewpoint);
		}

		FORCE_INLINE static void deserialize(encodeProtocol::reader* encodeReader, VirtualLogEntry<K>* virtualLogEntry) {
			K key = Converter<K>::decode(encodeReader, VIRTUAL_LOG_ENTRY_KEY);

			uinttype action = 0;
			inttype virtualSize = 0;
			uinttype minViewpoint = 0;
			uinttype maxViewpoint = 0;

			encodeReader->getUint32Field(VIRTUAL_LOG_ENTRY_ACTION, action);
			encodeReader->getInt32Field(VIRTUAL_LOG_ENTRY_VIRTUAL_SIZE, virtualSize);
			encodeReader->getUint32Field(VIRTUAL_LOG_ENTRY_MIN_VIEWPOINT, minViewpoint);
			encodeReader->getUint32Field(VIRTUAL_LOG_ENTRY_MAX_VIEWPOINT, maxViewpoint);

			virtualLogEntry->setKey(key);
			virtualLogEntry->setAction((LogEntryAction)action);
			virtualLogEntry->setVirtualSize(virtualSize);
			virtualLogEntry->setMinViewpoint(minViewpoint);
			virtualLogEntry->setMaxViewpoint(maxViewpoint);
		}

		// XXX: PeerInfo
		const static uinttype MSG_ID_PEER_INFO = 0x00000008;
		const static uinttype PEER_INFO_SERVER_ID = 4;
		const static uinttype PEER_INFO_ADDRESS = 5;
		const static uinttype PEER_INFO_MAP_BEHAVIOR = 6;
		const static uinttype PEER_INFO_FACILITATOR = 7;
		
		FORCE_INLINE static encodeProtocol::writer* serialize(IMessageService* messageService, PeerInfo* peerInfo, BaseMessageData* baseMessageData = null) {
			uinttype size = messageService->getBaseMessageSize(baseMessageData) + 
			encodeProtocol::writer::sizeOfUint32(peerInfo->getServerId()) +
			encodeProtocol::writer::sizeOfString(*(peerInfo->getAddressString())) +
			encodeProtocol::writer::sizeOfUint32(peerInfo->getMapBehavior()) +
			encodeProtocol::writer::sizeOfUint64((ulongtype)(peerInfo->getFacilitator()));
			
			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(MSG_ID_PEER_INFO), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData);
			encodeWriter->setUint32Field(PEER_INFO_SERVER_ID, peerInfo->getServerId());
			encodeWriter->setStringField(PEER_INFO_ADDRESS, *(peerInfo->getAddressString()));
			encodeWriter->setUint32Field(PEER_INFO_MAP_BEHAVIOR, (uinttype)(peerInfo->getMapBehavior()));
			encodeWriter->setUint64Field(PEER_INFO_FACILITATOR, (ulongtype)(peerInfo->getFacilitator()));

			return encodeWriter;
		}

		FORCE_INLINE static void deserialize(PeerInfo* peerInfo, ubytetype* data, uinttype dataSize) {
			encodeProtocol::reader encodeReader(data, dataSize);
			
			encodeReader.decodeFields();

			uinttype serverId = 0;
			String address;
			uinttype mapBehavior = 0;
			ulongtype facilitator = 0;

			encodeReader.getUint32Field(PEER_INFO_SERVER_ID, serverId);
			encodeReader.getStringField(PEER_INFO_ADDRESS, address);
			encodeReader.getUint32Field(PEER_INFO_MAP_BEHAVIOR, mapBehavior);
			encodeReader.getUint64Field(PEER_INFO_FACILITATOR, facilitator);

			peerInfo->setServerId(serverId);
			peerInfo->setAddress(address.c_str());
			peerInfo->setMapBehavior((MapBehavior)mapBehavior);
			peerInfo->setFacilitator((Facilitator<K>*)(facilitator));
		}

		FORCE_INLINE static void deserialize(encodeProtocol::reader* encodeReader, PeerInfo* peerInfo) {
			uinttype serverId = 0;
			String address;
			uinttype mapBehavior = 0;
			ulongtype facilitator = 0;

			encodeReader->getUint32Field(PEER_INFO_SERVER_ID, serverId);
			encodeReader->getStringField(PEER_INFO_ADDRESS, address);
			encodeReader->getUint32Field(PEER_INFO_MAP_BEHAVIOR, mapBehavior);
			encodeReader->getUint64Field(PEER_INFO_FACILITATOR, facilitator);

			peerInfo->setServerId(serverId);
			peerInfo->setAddress(address.c_str());
			peerInfo->setMapBehavior((MapBehavior)mapBehavior);
			peerInfo->setFacilitator((Facilitator<K>*)(facilitator));
		}

		// XXX: virtual key space version
		const static uinttype MSG_ID_VIRTUAL_KEY_SPACE_VERSION_RESPONSE = 0x00000009;
		const static uinttype VIRTUAL_KEY_SPACE_VERSION = 4;
		
		FORCE_INLINE static encodeProtocol::writer* serialize(IMessageService* messageService, longtype version, BaseMessageData* baseMessageData = null) {
			uinttype size = messageService->getBaseMessageSize(baseMessageData) + 
			encodeProtocol::writer::sizeOfInt64(version);
			
			encodeProtocol::writer* encodeWriter = messageService->createWriter(size, messageService->getMessageId(MSG_ID_VIRTUAL_KEY_SPACE_VERSION_RESPONSE), baseMessageData);

			messageService->encodeBaseMessage(encodeWriter, baseMessageData);
			encodeWriter->setInt64Field(VIRTUAL_KEY_SPACE_VERSION, version);

			return encodeWriter;
		}

		FORCE_INLINE static void deserialize(longtype* version, ubytetype* data, uinttype dataSize) {
			encodeProtocol::reader encodeReader(data, dataSize);
			
			encodeReader.decodeFields();

			encodeReader.getInt64Field(VIRTUAL_KEY_SPACE_VERSION, (*version));
		}

		FORCE_INLINE static void deserialize(encodeProtocol::reader* encodeReader, longtype* version) {
			encodeReader->getInt64Field(VIRTUAL_KEY_SPACE_VERSION, (*version));
		}

		// XXX: empty messages
		const static uinttype MSG_ID_VIRTUAL_REPRESENTATION_REQUEST = 0x00000020;
		const static uinttype MSG_ID_VIRTUAL_COMMIT = 0x00000021;
		const static uinttype MSG_ID_VIRTUAL_KEY_SPACE_VERSION_REQUEST = 0x00000022;

		// XXX: response codes
		const static uinttype MSG_ID_CODE_SUCCESS = 0x00000100;
		const static uinttype MSG_ID_CODE_FAIL = 0x00000101;

		FORCE_INLINE static uinttype* getMessageIds(IMessageService* messageService) {
			static uinttype messageIds[15];

			messageIds[0] = messageService->getMessageId(MSG_ID_STATUS_REQUEST);
			messageIds[1] = messageService->getMessageId(MSG_ID_STATUS_RESPONSE);
			messageIds[2] = messageService->getMessageId(MSG_ID_DATA_REQUEST);
			messageIds[3] = messageService->getMessageId(MSG_ID_VIRTUAL_INFO);
			messageIds[4] = messageService->getMessageId(MSG_ID_DATA_LOG_ENTRY);
			messageIds[5] = messageService->getMessageId(MSG_ID_DATA_LOG_ENTRY_BLOCK);
			messageIds[6] = messageService->getMessageId(MSG_ID_VIRTUAL_LOG_ENTRY);
			messageIds[7] = messageService->getMessageId(MSG_ID_VIRTUAL_REPRESENTATION_REQUEST);
			messageIds[8] = messageService->getMessageId(MSG_ID_VIRTUAL_COMMIT);
			messageIds[9] = messageService->getMessageId(MSG_ID_PEER_INFO);
			messageIds[10] = messageService->getMessageId(MSG_ID_VIRTUAL_KEY_SPACE_VERSION_REQUEST);
			messageIds[11] = messageService->getMessageId(MSG_ID_VIRTUAL_KEY_SPACE_VERSION_RESPONSE);

			messageIds[12] = messageService->getMessageId(MSG_ID_CODE_SUCCESS);
			messageIds[13] = messageService->getMessageId(MSG_ID_CODE_FAIL);
			messageIds[14] = 0;

			return messageIds;
		}

		FORCE_INLINE static void registerMessageIds(IMessageService* messageService) {
			boolean success = true;
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_STATUS_REQUEST));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_STATUS_RESPONSE));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_DATA_REQUEST));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_VIRTUAL_INFO));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_DATA_LOG_ENTRY));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_DATA_LOG_ENTRY_BLOCK));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_VIRTUAL_LOG_ENTRY));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_VIRTUAL_REPRESENTATION_REQUEST));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_VIRTUAL_COMMIT));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_PEER_INFO));

			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_CODE_SUCCESS));
			success &= messageService->registerMessageId(messageService->getMessageId(MSG_ID_CODE_FAIL));

			if (success == false ) {
				throw InvalidException("Invalid Serializer<K>::registerMessageIds: register message failed");	
			}
		}

		FORCE_INLINE static void printMessageIds(IMessageService* messageService) {
			fprintf(stderr, " \nMSG_ID_STATUS_REQUEST %d ", messageService->getMessageId(MSG_ID_STATUS_REQUEST));
			fprintf(stderr, " \nMSG_ID_STATUS_RESPONSE %d ", messageService->getMessageId(MSG_ID_STATUS_RESPONSE));
			fprintf(stderr, " \nMSG_ID_DATA_REQUEST %d ", messageService->getMessageId(MSG_ID_DATA_REQUEST));
			fprintf(stderr, " \nMSG_ID_VIRTUAL_INFO %d ", messageService->getMessageId(MSG_ID_VIRTUAL_INFO));
			fprintf(stderr, " \nMSG_ID_DATA_LOG_ENTRY %d ", messageService->getMessageId(MSG_ID_DATA_LOG_ENTRY));
			fprintf(stderr, " \nMSG_ID_DATA_LOG_ENTRY %d ", messageService->getMessageId(MSG_ID_DATA_LOG_ENTRY_BLOCK));
			fprintf(stderr, " \nMSG_ID_VIRTUAL_LOG_ENTRY %d ", messageService->getMessageId(MSG_ID_VIRTUAL_LOG_ENTRY));
			fprintf(stderr, " \nMSG_ID_VIRTUAL_REPRESENTATION_REQUEST %d ", messageService->getMessageId(MSG_ID_VIRTUAL_REPRESENTATION_REQUEST));
			fprintf(stderr, " \nMSG_ID_VIRTUAL_COMMIT %d ", messageService->getMessageId(MSG_ID_VIRTUAL_COMMIT));
			fprintf(stderr, " \nMSG_ID_PEER_INFO %d ", messageService->getMessageId(MSG_ID_PEER_INFO));

			fprintf(stderr, " \nMSG_ID_CODE_SUCCESS %d ", messageService->getMessageId(MSG_ID_CODE_SUCCESS));
			fprintf(stderr, " \nMSG_ID_CODE_FAIL %d ", messageService->getMessageId(MSG_ID_CODE_FAIL));
		}

};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_SERIALIZER_H_ */
