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
#include "cxx/util/Logger.h"

#include "cxx/util/Comparator.h"
#include "cxx/util/concurrent/locks/Lock.h"
#include "com/deepis/datastore/api/DeepStore.h"

#include "com/deepis/datastore/api/deep/Protocol.h"
#include "com/deepis/datastore/api/deep/Converter.h"
#include "com/deepis/datastore/api/deep/Comparator.h"
#include "com/deepis/datastore/api/deep/KeyBuilder.h"
#include "com/deepis/datastore/api/deep/Schema.h"
#include "com/deepis/datastore/api/deep/Store.h"
#include "com/deepis/datastore/api/deep/Condition.h"

#include "cxx/util/HashMap.h"
#include "cxx/util/HashSet.h"

#include "cxx/util/concurrent/atomic/AtomicLong.h"
#include "cxx/util/concurrent/locks/ReentrantLock.h"

#include "cxx/nio/file/Path.h"

#include "cxx/util/regex/Pattern.h"

#include "com/deepis/db/store/relative/util/License.h"
#include "com/deepis/db/store/relative/util/Versions.h"
#include "com/deepis/db/store/relative/util/MapFileUtil.h"
#include "com/deepis/db/store/relative/util/DynamicUtils.h"

#include "com/deepis/db/store/relative/core/Properties.h"
#include "com/deepis/db/store/relative/core/RealTimeMap.h"
#include "com/deepis/db/store/relative/core/RealTimeMap.cxx"

#ifdef DEEP_DISTRIBUTED
#include "com/deepis/db/store/relative/distributed/VirtualKeySpace.cxx"
#include "com/deepis/db/store/relative/distributed/SegmentDataManager.cxx"
#include "com/deepis/db/store/relative/distributed/DataLogManager.cxx"
#include "com/deepis/db/store/relative/distributed/MapFacilitator.cxx"
#include "com/deepis/db/store/relative/distributed/ObjectMessageService.cxx"
#endif

#include "cxx/lang/Runtime.h"

using namespace cxx::lang;
using namespace cxx::util;
using namespace cxx::util::concurrent::locks;
using namespace cxx::util::concurrent::atomic;
using namespace cxx::util::regex;
using namespace com::deepis::db::store::relative::util;
using namespace com::deepis::db::store::relative::core;
using namespace com::deepis::db::store::relative::distributed;

typedef HashSet<longtype>::KeySetIterator HashSetIterator;

namespace com { namespace deepis { namespace datastore { namespace api {

static const double GB = 1024 * 1024 * 1024;

template<typename K>
class Datastore : public DeepStore {
	private:
		static Pattern OPT_INDEX_ORIENTATION;

		String m_path;

		long long m_options;
		String m_keyName;
		int m_keyLength;
		int m_recordLength;
		#ifdef DEEP_DISTRIBUTED
		MapBehavior m_mapBehavior;
		#endif
		bool m_isReservedKey;

		RealTimeMap<K>* m_map;

		AtomicLong* m_uniqueKey;

		KeyBuilder<K>* m_keyBuilder;
		Comparator<K>* m_comparator;
		SchemaBuilder<K>* m_schemaBuilder;

	private:
		int get(DeepThreadContext* context, const K key, unsigned char** record, unsigned int* length, K* retkey, int type, int lockOption, bool keyOnly, DeepCursor* cursor, Condition* condition);
		int put(DeepThreadContext* context, const K key, const unsigned char* record, unsigned int length);
		int update(DeepThreadContext* context, const K key, const unsigned char* record, unsigned int length, bool lock);
		int update(DeepThreadContext* context, const K oldKey, const unsigned char* oldRecord, unsigned int oldLength, const K newKey, const unsigned char* newRecord, unsigned int length, bool lock);
		int remove(DeepThreadContext* context, const K key, const unsigned char* value, unsigned int length, bool lock);

		int getPutOption(DeepThreadContext* context, const K key);

		void printKey(DeepThreadContext* context, const K key);

	public:
		Datastore(const char* filePath,
				long long options,
				const char* keyName,
				int keyLength,
				int recordLength,
				#ifdef DEEP_DISTRIBUTED
				MapBehavior mapBehavior,
				#endif
				bool isReservedKey);
		~Datastore();

		int get(DeepThreadContext* context, const unsigned char* key, unsigned char** record, unsigned int* length, unsigned char* retkey, int type, int lockOption, bool keyOnly, unsigned int keyPartMask, DeepCursor* cursor = NULL, Condition* condition = NULL, boolean packKey = true);
		int get(DeepThreadContext* context, const unsigned char* key, unsigned char** record, unsigned int* length, unsigned char* retkey, int type, int lockOption = CT_DATASTORE_LOCK_NONE) {
			return get(context, key, record, length, retkey, type, lockOption, false, ~0, NULL, NULL, true);
		}

		int put(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length);
		int put(DeepThreadContext* context, const unsigned char* record, unsigned int length);

		int update(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock = true);
		int update(DeepThreadContext* context, const unsigned char* oldRecord, unsigned int oldLength, const unsigned char* newRecord, unsigned int newLength, bool lock = true);

		int remove(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock = true);
		int remove(DeepThreadContext* context, const unsigned char* record, unsigned int length, bool lock = true);

		long long size(DeepThreadContext* context);
		long long length(DeepThreadContext* context, int option);
		long long meanRecLength(DeepThreadContext*);
		long long range(DeepThreadContext* context, const unsigned char* startKey, int startType, unsigned int startMask, const unsigned char* endKey, int endType, unsigned int endMask);
		void cardinality(DeepThreadContext* context, long long* stats, bool recalculate);
		void acquireCursor(DeepThreadContext* context, DeepCursor* cursor);
		void releaseCursor(DeepThreadContext* context, DeepCursor* cursor);
		DeepCursor* createCursor(DeepThreadContext* context);
		void unlockRow(DeepThreadContext* context);

		#ifdef DEEP_DISTRIBUTED
		int initialize(IMessageService* mesageService = 0x0, PeerInfo* peerInfo = 0x0);
		#else
		int initialize();
		#endif

		int open(DeepThreadContext* context);
		int close(DeepThreadContext* context = null);

		int clear(DeepThreadContext* context = null, boolean final = false);
		int recover(DeepThreadContext* context, bool rebuild);
		int optimize(DeepThreadContext* context, int type);

		int associate(DeepStore* datastore, bool hasPrimary, bool dynamic = false);
		int disassociate(DeepStore* datastore, bool dynamic = false);

		int associateTransaction(void* transaction);

		const char* getName();
		int getErrorCode();
		void getErrorKey(unsigned char* retkey);

		unsigned char* getHiddenKey(unsigned char* key);

		unsigned long long generateUniqueKey();

		int getReservedKeyBlock(unsigned long long offset, unsigned long long block, unsigned long long* first, unsigned long long* reserved, DeepThreadContext* context);
		unsigned long long getNextReservedKey(DeepThreadContext* context);

		int addKeyPart(unsigned int fieldIndex, unsigned char type, unsigned short length, unsigned int valueOffset, unsigned int nullOffset,
				unsigned char nullBit, bool isIgnored = false, bool isReservedKey = false, short variablePosition = -1, short primaryPosition = -1);
		
		int addField(unsigned char type, unsigned char realType, unsigned int packLength, unsigned int rowPackLength, unsigned int keyLength, 
			     unsigned int lengthBytes, unsigned short index, unsigned char nullBit, unsigned int nullOffset, unsigned int valueOffset,
			     int characterSet, bool gcolVirtual, const char* fieldName = "");

		void populateKey(const unsigned char* record, unsigned char* key);

		int compareKey(DeepThreadContext* context, const unsigned char* key1, const unsigned char* key2);

		void* getMap() {
			return m_map;
		}

		inline void markSchema() {
			m_map->markSchema();
		}

		inline void commitSchema() {
			m_map->commitSchema();
		}
		
		inline void rollbackSchema() {
			m_map->rollbackSchema();
		}

		inline void setHasVariableLengthFields(bool hasVariableLengthFields) {
			m_schemaBuilder->setHasVariableLengthFields(hasVariableLengthFields);
		}

		inline void setHasVirtualFields(bool hasVirtualFields) {
			m_schemaBuilder->setHasVirtualFields(hasVirtualFields);
		}

		inline void setNullBytes(unsigned int nullBytes) {
			m_schemaBuilder->setNullBytes(nullBytes);
		}
		
		inline void setBlobStartOffset(unsigned int blobStartOffset) {
			m_schemaBuilder->setBlobStartOffset(blobStartOffset);
		}
		
		inline void setBlobPtrSize(unsigned int blobPtrSize) {
			m_schemaBuilder->setBlobPtrSize(blobPtrSize);
		}
		
		inline void setUnpackedRowLength(unsigned long long unpackedRowLength) {
			m_schemaBuilder->setUnpackedRowLength(unpackedRowLength);
		}
		
		inline unsigned long long getAutoIncrementValue() {
			return m_schemaBuilder->getAutoIncrementValue();
		}
		
		inline void setAutoIncrementValue(unsigned long long autoIncrementValue) {
			m_schemaBuilder->setAutoIncrementValue(autoIncrementValue);
		}

		inline void setDirectoryPaths(const char* dataDirPath, const char* indexDirPath) {
			File dataDir("");
			File indexDir("");

			if (dataDirPath != null) {
				dataDir = File(String(File(dataDirPath).getParentFile()) + File::separator);
			}
			if (indexDirPath != null) {
				indexDir = File(String(File(indexDirPath).getParentFile()) + File::separator);
			}

			m_schemaBuilder->setDirectoryPaths(dataDir, indexDir);
		}
		
		inline void setCharacterSet(int characterSet) {
			m_schemaBuilder->setCharacterSet(characterSet);
		}
		
		inline void setIsTemporary(bool isTemporary) {
			m_schemaBuilder->setIsTemporary(isTemporary);
		}
		
		inline void setIndexOrientation(unsigned int orientation) {
			m_schemaBuilder->setIndexOrientation((RealTime::IndexOrientation)orientation);
		}

		inline void setComment(const char* comment, unsigned int length) {
			static const String row("row");
			static const String column("column");
			
			String input(comment, length);
			
			Matcher m = OPT_INDEX_ORIENTATION.matcher(input);
			if (m.groups() >= 3) {
				String orientation = m.group(2);
				if(orientation.equals(&row)) {
					setIndexOrientation(CT_DATASTORE_INDEX_ORIENTATION_ROW);
				} else if (orientation.equals(&column)) {
					setIndexOrientation(CT_DATASTORE_INDEX_ORIENTATION_COLUMN);
				}
			}
		}
		
		inline char* packRow(const char* unpackedRow, unsigned int length, char** buffer, unsigned int* bufferLength) {
			return RowProtocol_v1::packRow(m_schemaBuilder, const_cast<char*>(unpackedRow), length, buffer, bufferLength); // TODO: remove const_cast(...)
		}
		
		inline char* unpackRow(char* unpackedRow, const char* packedRow, char** blobBuffer, unsigned int* blobBufferLength) {
			return RowProtocol_v1::unpackRow(m_schemaBuilder, unpackedRow, const_cast<char*>(packedRow), blobBuffer, blobBufferLength);
		}
		
		inline char* unpackRowFromKey(char* unpackedRow, const char* unpackedKey, char** blobBuffer, unsigned int* blobBufferLength, bool preserve) {
			return RowProtocol_v1::unpackRowFromKey(m_schemaBuilder, unpackedRow, const_cast<char*>(unpackedKey), blobBuffer, blobBufferLength, preserve);
		}
};

struct DeepThreadInners {

	public:
		BasicArray<inttype, true> m_transactionTypes;
		Transaction* m_transaction;

		DeepThreadInners(void):
			m_transactionTypes(Properties::DEFAULT_TRANSACTION_DEPTH, false),
			m_transaction(null) {
		}
};

template<typename K>
class Cursor : public DeepCursor {
	public:
		typedef typename TreeMap<K,Information*,bytetype>::TreeMapEntryIterator MapIterator;

		MapIterator* m_iterator;

		Cursor() {
			m_iterator = new MapIterator();
		}

		~Cursor() {
			delete m_iterator;
		}
};

DeepStore* DeepStore::create(const char* filePath,
		long long options,
		const char* keyName,
		int keyType,
		int keyLength,
		int recordLength,
		#ifdef DEEP_DISTRIBUTED
		int mapBehaviorValue,
		#endif
		bool isReservedKey) {

	DeepStore* datastore = null;
	#ifdef DEEP_DISTRIBUTED
	MapBehavior mapBehavior = (MapBehavior) mapBehaviorValue;
	#endif

	switch(keyType) {
		case CT_DATASTORE_LONGLONG:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepLongLong>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepLongLong>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		case CT_DATASTORE_ULONGLONG:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepULongLong>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepULongLong>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		case CT_DATASTORE_LONG_INT:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepLongInt>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepLongInt>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		case CT_DATASTORE_ULONG_INT:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepULongInt>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepULongInt>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		#ifndef DEEP_REDUCE_TEMPLATES
		case CT_DATASTORE_SHORT_INT:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepShortInt>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepShortInt>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		case CT_DATASTORE_USHORT_INT:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepUShortInt>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepUShortInt>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		case CT_DATASTORE_INT8:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepTinyInt>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepTinyInt>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		case CT_DATASTORE_UINT8:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepUTinyInt>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepUTinyInt>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		case CT_DATASTORE_FLOAT:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepFloat>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepFloat>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		case CT_DATASTORE_DOUBLE:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepDouble>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepDouble>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		#endif
		case CT_DATASTORE_INT24:
		case CT_DATASTORE_UINT24:
		case CT_DATASTORE_NUMBER_NULL:
		case CT_DATASTORE_BINARY:
		case CT_DATASTORE_VARBINARY1:
		case CT_DATASTORE_VARBINARY2:
		case CT_DATASTORE_FIXED_VARBINARY1:
		case CT_DATASTORE_TEXT:
		case CT_DATASTORE_TEXT_MULTIBYTE:
		case CT_DATASTORE_VARTEXT1:
		case CT_DATASTORE_VARTEXT2:
		case CT_DATASTORE_FIXED_VARTEXT1:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepNByte*>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepNByte*>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		case CT_DATASTORE_COMPOSITE:
			#ifdef DEEP_DISTRIBUTED
			datastore = new Datastore<DeepComposite*>(filePath, options, keyName, keyLength, recordLength, mapBehavior, isReservedKey);
			#else
			datastore = new Datastore<DeepComposite*>(filePath, options, keyName, keyLength, recordLength, isReservedKey);
			#endif
			break;
		default:
			DEEP_LOG(ERROR, OTHER, "(Invalid key): type %d)\n", keyType);
			throw UnsupportedOperationException("Invalid key type");
	}

	return datastore;
}

bool DeepStore::validatePaths(const char* filePath, const char* dataDir, const char* indexDir) {
	struct Util {
		static boolean validate(const Path& dname, const char* dir) {
			if (dir == null) {
				return true;
			}

			Path path(String(dir, strnlen(dir, PATH_MAX)));
			inttype index = path.lastIndexOf(File::separator);
			if (index == -1) {
				return false;
			}
			path = Path(path.substring(0, index));

			if (Path::toRealPath(path, path) == false) {
				return false;
			}
			if (path.startsWith(dname) == true) {
				return false;
			}

			return true;
		}
	};

	Path dname(String("."));
	{
		Path sname(filePath);
		inttype index = sname.lastIndexOf(File::separator);
		if (index != -1) {
			dname = Path(sname.substring(0, index));
		}
		index = dname.lastIndexOf(File::separator);
		if (index != -1) {
			dname = Path(dname.substring(0, index));
		}
	}
	Path::toRealPath(dname, dname);

	if (Util::validate(dname, dataDir) == false) {
		DEEP_LOG(WARN, OTHER, "Invalid data directory specified: %s, %s\n", dataDir, (const char*)dname);
		return false;
	}

	if (Util::validate(dname, indexDir) == false) {
		DEEP_LOG(WARN, OTHER, "Invalid data directory specified: %s, %s\n", dataDir, (const char*)dname);
		return false;
	}

	return true;
}

int DeepStore::clobber(const char* filePath) {
	int status = CT_DATASTORE_SUCCESS;

	String path(filePath);
	String store(path + CT_DATASTORE_BASE_EXT);

	DEEP_LOG(DEBUG, DROP_, "%s\n", store.data());

	if (MapFileUtil::clobber(store) == false) {
		status = CT_DATASTORE_ERROR;
	}

	return status;
}

int DeepStore::rename(const char* from, const char* to) {
	int status = CT_DATASTORE_SUCCESS;

	String sourcePath(from);
	String sourceName(sourcePath + CT_DATASTORE_BASE_EXT);

	String targetPath(to);
	String targetName(targetPath + CT_DATASTORE_BASE_EXT);

	DEEP_LOG(DEBUG, RENAM, "%s to %s\n", sourceName.data(), targetName.data());

	if (MapFileUtil::move(sourceName, targetName) == false) {
		status = CT_DATASTORE_ERROR;
	}

	return status;
}

void DeepStore::setBuildNumber(int bld) {
	Versions::setBuildNumber(bld);
}

void DeepStore::setMajorVersion(int maj) {
	Versions::setMajorVersion(maj);
}

void DeepStore::setMinorVersion(int min) {
	Versions::setMinorVersion(min);
}

void DeepStore::setRevisionNumber(int rev) {
	Versions::setRevisionNumber(rev);
}

void DeepStore::setDebugEnabled(bool enabled) {
	if (enabled == true) {
		Logger::enableLevel(Logger::DEBUG);
	} else {
		Logger::disableLevel(Logger::DEBUG);
	}
}

void DeepStore::setInfoEnabled(bool enabled) {
	if (enabled == true) {
		Logger::enableLevel(Logger::INFO);
	} else {
		Logger::disableLevel(Logger::INFO);
	}
}

void DeepStore::setWarnEnabled(bool enabled) {
	if (enabled == true) {
		Logger::enableLevel(Logger::WARN);
	} else {
		Logger::disableLevel(Logger::WARN);
	}
}

void DeepStore::setErrorEnabled(bool enabled) {
	if (enabled == true) {
		Logger::enableLevel(Logger::ERROR);
	} else {
		Logger::disableLevel(Logger::ERROR);
	}
}

void DeepStore::setFileSize(long long fileSize) {
	Properties::setFileSize(fileSize);
}

void DeepStore::setTrtFileSize(long long fileSize) {
	Properties::setTrtFileSize(fileSize);
}

void DeepStore::setThreadLimit(int threadLimit) {
	Properties::setThreadLimit(threadLimit);
}

void DeepStore::setCacheSize(long long cacheSize) {
	Properties::setCacheSize(cacheSize);
}

void DeepStore::setLockTimeout(int timeout) {
	Properties::setTransactionTimeout(timeout * 1000 /* msecs */);
}

void DeepStore::setFileDescriptorLimit(int limit) {
	Properties::setFileLimit(limit);
}

void DeepStore::setWorkThreads(int threads) {

	if (true == Properties::getDynamicResources()) {
		return;
	}

	Properties::setWorkThreads(threads);
}

void DeepStore::setReorgThreads(int threads) {

	if (true == Properties::getDynamicResources()) {
		return;
	}

	Properties::setReorgThreads(threads);
}

void DeepStore::setIsolationLevel(int isolation) {
	Transaction::setGlobalIsolation((Transaction::Isolation) isolation);
}

void DeepStore::setInfinitelimit(bool enabled) {
	RealTimeResource::setInfinitelimit(enabled);
}

void DeepStore::setTransactionChunk(int chunk) {
	Properties::setTransactionChunk(chunk);
}

void DeepStore::setTransactionStream(bool enabled) {
	Properties::setTransactionStream(enabled);
}

void DeepStore::setSegmentSize(int size) {
	Properties::setSegmentSize(size);
}

void DeepStore::setDynamicResources(bool dynamic) {
	DynamicUtils::setDynamicResources(dynamic);
}

bool DeepStore::getDynamicResources(void) {
	return DynamicUtils::getDynamicResources();
}

void DeepStore::setDurable(bool enabled) {
	Properties::setDurable(enabled);
}

void DeepStore::setDurableSyncInterval(long long interval) {
	Properties::setDurableSyncInterval(interval);
}

#ifdef DEEP_SYNCHRONIZATION_GROUPING
void DeepStore::setDurableHoldDownTime(long long time) {
	Properties::setDurableHoldDownTime(time);
}

void DeepStore::setDurableHoldDownThreshold(long long threshold) {
	Properties::setDurableHoldDownThreshold(threshold);
}
#endif

void DeepStore::setValueCompressPercent(double percent) {
	Properties::setValueCompressPercent(percent);
}

void DeepStore::setDebugEnabled(int option, bool enabled) {
	Properties::setDebugEnabled((Properties::DebugOption) option, enabled);
}

void DeepStore::setProfilingEnabled(int option, bool enabled) {
	Properties::setProfilingEnabled((Properties::ProfilingOption) option, enabled);
}

void DeepStore::setProfilingTriggerInterval(int interval) {
	Properties::setProfilingTriggerInterval(interval);
}

void DeepStore::setStatisticsFlushInterval(int interval) {
	Properties::setStatisticsFlushInterval(interval);
}

const char** DeepStore::getLoggingTopicNames(void) {
	return Logger::topicStrings;
}

void DeepStore::setLoggingTopicOptions(unsigned long long options) {
	Logger::setTopicFlags(options);
}

void DeepStore::setLogOption(int option, bool enabled) {
	Properties::setLogOption((Properties::LogOption) option, enabled);
}

void DeepStore::setMemoryFragment(bool enabled) {
	Properties::setMemoryFragment(enabled);
}

void DeepStore::setMemoryAnalytics(bool enabled) {
	Properties::setMemoryAnalytics(enabled);
}

void DeepStore::setCheckpointMode(unsigned int mode) {
	Properties::setCheckpointMode((Properties::CheckpointMode)mode);
}

void DeepStore::setAutomaticCheckpointInterval(unsigned int interval) {
	Properties::setAutomaticCheckpointInterval(interval);
}

bool DeepStore::performManualCheckpoint(void) {
	DEEP_LOG(INFO, CHECK, "initiating manual checkpoint...\n");

	return RealTime::requestManualCheckpoint();
}

void DeepStore::setRecoveryReplay(bool enabled) {
	Properties::setRecoveryReplay(enabled);
}

void DeepStore::setRecoveryRealign(bool enabled) {
	Properties::setRecoveryRealign(enabled);
}

void DeepStore::setRecoverySafe(bool enabled) {
	Properties::setRecoverySafe(enabled);
}

void DeepStore::setVirtualized(bool enabled) {
	if (enabled == true) {
		Lock::setPause(Lock::VIRTUAL_PAUSE);
		Lock::setYield(Lock::VIRTUAL_YIELD);

	} else {
		Lock::setPause(Lock::PHYSICAL_PAUSE);
		Lock::setYield(Lock::PHYSICAL_YIELD);
	}
}

void DeepStore::setSeekStatistics(bool enabled) {
	Properties::setSeekStatistics(enabled);
}

void DeepStore::setSeekStatisticsResetInterval(int interval) {
	Properties::setSeekStatisticsResetInterval(interval * 1000 /* msecs */);
}

void DeepStore::setSeekStatisticsDisplayInterval(int interval) {
	Properties::setSeekStatisticsDisplayInterval(interval * 1000 /* msecs */);
}

void DeepStore::setReorgWorkPercent(int percent) {
	Properties::setReorgWorkPercent((doubletype)percent / 100.0);
}

void DeepStore::setFragmentationPercent(int percent) {
	Properties::setFragmentationPercent((doubletype)percent / 100.0);
}

void DeepStore::setSemiPurge(bool enabled) {
	Properties::setSemiPurge(enabled);
}

void DeepStore::setDynamicSummarization(bool enabled) {
	Properties::setDynamicSummarization(enabled);
}

void DeepStore::setRangeSync(bool enabled) {
	Properties::setRangeSync(enabled);
}

int DeepStore::generateSystemUid() {
	int id = License::generateSystemUid();
	Properties::setSystemUid( id );
	return id;
}

void DeepStore::setActivationKey(const char* activation_key) {
	Properties::setActivationKey(activation_key);
	License::logActivationStatus(activation_key); //Log whenever the key changes
}

const char* DeepStore::getActivationKey(void) {
	return Properties::getActivationKey();
}

int DeepStore::validateActivationKey(const char* activation_key) {
	return License::validateActivationKey(activation_key);
}

void DeepStore::setAllowLrtVrtMismatch(bool enabled) {
	Properties::setAllowLrtVrtMismatch(enabled);
}

void DeepStore::setCardinalityRecalculateRecovery(bool enabled) {
	Properties::setCardinalityRecalculateRecovery(enabled);
}

void DeepStore::setFileRefCheckMod(inttype mod) {
	Properties::setFileRefCheckMod(mod);
}

void DeepStore::shutdownCacheManagement(void) {
	RealTimeResource::immediateShutdown();
}

void DeepStore::runtimeGarbageCollection(bool flag) {
	RealTimeResource::gcRuntime(flag);
}

void DeepStore::shutdownGarbageCollection(bool flag) {
	RealTimeResource::gcShutdown(flag);
}

bool DeepStore::checkMemoryRequirements() {
	return RealTimeResource::checkMemoryRequirements();
}

void DeepStore::setTimer(DEEP_TIMER_CALLBACK callback, void* context) {
	#ifdef COM_DEEPIS_DB_TIMER
	RealTimeResource::setTimer(callback, context);
	#endif
}

template<typename K>
FORCE_INLINE KeyBuilder<K>* createKeyBuilder(SchemaBuilder<K>* schemaBuilder) {
	KeyBuilder<K>* keyBuilder = new KeyBuilder<K>();
	keyBuilder->setSchemaBuilder(schemaBuilder);
	return keyBuilder;
}

template<typename K>
FORCE_INLINE Comparator<K>* createComparator(SchemaBuilder<K>* schemaBuilder) {
	Comparator<K>* comparator = new Comparator<K>();
	return comparator;
}

template<>
FORCE_INLINE Comparator<DeepNByte*>* createComparator(SchemaBuilder<DeepNByte*>* schemaBuilder) {
	Comparator<DeepNByte*>* comparator = new Comparator<DeepNByte*>();
	comparator->setSchemaBuilder(schemaBuilder);
	return comparator;
}

template<>
FORCE_INLINE Comparator<DeepComposite*>* createComparator(SchemaBuilder<DeepComposite*>* schemaBuilder) {
	Comparator<DeepComposite*>* comparator = new Comparator<DeepComposite*>();
	comparator->setSchemaBuilder(schemaBuilder);
	return comparator;
}


template<typename K>
Datastore<K>::Datastore(const char* filePath,
		long long options,
		const char* keyName,
		int keyLength,
		int recordLength,
		#ifdef DEEP_DISTRIBUTED
		MapBehavior mapBehavior,
		#endif
		bool isReservedKey) :

		m_path(filePath),
		m_options(options),
		m_keyName(keyName),
		m_keyLength(keyLength),
		m_recordLength(recordLength),
		#ifdef DEEP_DISTRIBUTED
		m_mapBehavior(mapBehavior),
		#endif
		m_isReservedKey(isReservedKey),
		m_map(null),
		m_uniqueKey(null) {

	m_schemaBuilder = new SchemaBuilder<K>();
	m_comparator = createComparator<K>(m_schemaBuilder);
	m_keyBuilder = createKeyBuilder<K>(m_schemaBuilder);

	m_options |= (m_keyBuilder->isPrimitive() == true) ? RealTimeMap<K>::O_FIXEDKEY : 0;
}

template<typename K>
Datastore<K>::~Datastore() {
	delete m_comparator;
	// XXX: for debugging reasons
	m_comparator  = null;

	delete m_keyBuilder;
	// XXX: for debugging reasons
	m_keyBuilder = null;
	
	delete m_schemaBuilder;
	// XXX: for debugging reasons
	m_schemaBuilder = null;

	delete m_uniqueKey;
	// XXX: for debugging reasons
	m_uniqueKey = null;

	#ifdef DEEP_DISTRIBUTED
	if (m_map->getMapFacilitator() != null) {
		delete m_map->getMapFacilitator();
	}
	#endif

	delete m_map;
	// XXX: for debugging reasons
	m_map = null;
}

template<typename K>
#ifdef DEEP_DISTRIBUTED
int Datastore<K>::initialize(IMessageService* messageService, PeerInfo* peerInfo) {
#else
int Datastore<K>::initialize() {
#endif
	if (m_map == null) {
		String store(m_path + CT_DATASTORE_BASE_EXT);

		DEEP_LOG(DEBUG, AFFIX, "store: %s, key length is %d, row length is %d\n", store.data(), m_keyLength, m_recordLength);

		m_map = new RealTimeMap<K>(store, m_options, m_keyBuilder->getUnpackLength(), m_recordLength, m_comparator, m_keyBuilder, m_schemaBuilder);
		setIdentifier(m_map->getIdentifier());
		m_schemaBuilder->setKeyName(getName());
		m_schemaBuilder->setMapParams(m_map);

		m_schemaBuilder->setKeyCompression((m_options & CT_DATASTORE_OPTION_KEY_COMPRESS) == CT_DATASTORE_OPTION_KEY_COMPRESS);
		m_schemaBuilder->setValueCompression((m_options & CT_DATASTORE_OPTION_VALUE_COMPRESS) == CT_DATASTORE_OPTION_VALUE_COMPRESS);

		#ifdef DEEP_DISTRIBUTED
		if (m_mapBehavior != DISTRIBUTED_STANDALONE) {
			MapFacilitator<K>* mapFacilitator = new MapFacilitator<K>(m_map, m_mapBehavior, messageService, peerInfo);
			m_map->setMapFacilitator(mapFacilitator);

			// XXX: start the message service / thread
			messageService->initialize(mapFacilitator);
		}
		#endif
	}

	return CT_DATASTORE_SUCCESS;
}

template<>
int Datastore<DeepULongLong>::open(DeepThreadContext* context) {
	Transaction* transaction = (Transaction*) context->getTransactionInline();

	boolean success = m_map->mount(m_isReservedKey, transaction);

	return (success == true) ? CT_DATASTORE_SUCCESS : CT_DATASTORE_ERROR;
}

template<typename K>
int Datastore<K>::open(DeepThreadContext* context) {
	Transaction* transaction = (Transaction*) context->getTransactionInline();

	boolean success = m_map->mount(m_isReservedKey, transaction);

	// XXX: note that if unique key is necessary it is obtained in recover

	return (success == true) ? CT_DATASTORE_SUCCESS : CT_DATASTORE_ERROR;
}

template<typename K>
int Datastore<K>::close(DeepThreadContext* context) {
	//Transaction* transaction = (Transaction*) context->getTransactionInline();

	m_map->unmount(false /* don't delete cache/resources until map delete */);

	return CT_DATASTORE_SUCCESS;
}

template<typename K>
int Datastore<K>::clear(DeepThreadContext* context, boolean final) {
	//Transaction* transaction = (Transaction*) context->getTransactionInline();

	m_map->clear(final);

	return CT_DATASTORE_SUCCESS;
}

template<typename K>
int Datastore<K>::recover(DeepThreadContext* context, bool rebuild) {
	//Transaction* transaction = (Transaction*) context->getTransactionInline();

	boolean success = m_map->recover(rebuild);

	return (success == true) ? CT_DATASTORE_SUCCESS : CT_DATASTORE_ERROR;
}

template<typename K>
int Datastore<K>::optimize(DeepThreadContext* context, int type) {
	//Transaction* transaction = (Transaction*) context->getTransactionInline();

	boolean success = m_map->optimize((RealTime::OptimizeOption) type);

	return (success == true) ? CT_DATASTORE_SUCCESS : CT_DATASTORE_ERROR;
}

template<>
int Datastore<DeepULongLong>::recover(DeepThreadContext* context, bool rebuild) {
	// TODO: pass trancation into the get below
	//Transaction* transaction = (Transaction*) context->getTransactionInline();

	boolean success = m_map->recover(rebuild);

	if ((success == true) && (m_keyBuilder->hasHiddenKey() == true)) {
		DeepULongLong rKey = 0;

		if (m_map->get(Converter<DeepULongLong>::NULL_VALUE, (nbyte*) null, RealTime::LAST, &rKey, (Transaction*) null) == true) {
			m_uniqueKey = new AtomicLong(rKey);

		} else {
			m_uniqueKey = new AtomicLong(0);
		}
	}

	return (success == true) ? CT_DATASTORE_SUCCESS : CT_DATASTORE_ERROR;
}

template<typename K>
int Datastore<K>::associate(DeepStore* datastore, bool hasPrimary, bool dynamic) {
	m_map->associate( (RealTime*) datastore->getMap(), hasPrimary, dynamic);
	datastore->setPrimary(this);

	// XXX: datastore is now a secondary (secondary inherents primary table identifier);
	datastore->setIdentifier(((RealTime*) datastore->getMap())->getIdentifier());

	return CT_DATASTORE_SUCCESS;
}

template<typename K>
int Datastore<K>::disassociate(DeepStore* datastore, bool dynamic) {
	m_map->disassociate( (RealTime*) datastore->getMap(), dynamic);

	return CT_DATASTORE_SUCCESS;
}

template<typename K>
int Datastore<K>::put(DeepThreadContext* context, const unsigned char* record, unsigned int length) {
	return put(context, m_keyBuilder->getKey((bytearray) record), record, length);
}

template<>
int Datastore<DeepComposite*>::put(DeepThreadContext* context, const unsigned char* record, unsigned int length) {
	DeepComposite key((const inttype) m_keyBuilder->getUnpackLength());

	return put(context, m_keyBuilder->fillKey(null, (bytearray) record, &key), record, length);
}

template<>
int Datastore<DeepNByte*>::put(DeepThreadContext* context, const unsigned char* record, unsigned int length) {
	DeepNByte key((const inttype) m_keyBuilder->getUnpackLength());

	return put(context, m_keyBuilder->fillKey(null, (bytearray) record, &key), record, length);
}

template<typename K>
int Datastore<K>::put(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length) {
	return put(context, *((K*) key), record, length);
}

template<>
int Datastore<DeepComposite*>::put(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length) {
	DeepComposite bKey((const inttype) m_keyBuilder->getUnpackLength());

	m_keyBuilder->packKey(&bKey, (const bytearray) key);

	return put(context, &bKey, record, length);
}

template<>
int Datastore<DeepNByte*>::put(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length) {
	DeepNByte bKey((const bytearray) key, m_keyBuilder->getPackLength((const bytearray) key));

	return put(context, &bKey, record, length);
}

template<typename K>
int Datastore<K>::put(DeepThreadContext* context, const K key, const unsigned char* record, unsigned int length) {
	bool status = true;

	nbyte* value = ((nbyte*) context->value)->reassign((const bytearray) record, length);

	Transaction* transaction = (Transaction*) context->getTransactionInline();

	status = m_map->put(key, value, (RealTime::WriteOption) getPutOption(context, key), transaction);

	if ((status == true) && (transaction != null)) {
		bool loadData = context->inLoadDataTransactionInline(getPrimary());
		bool uniqueChecks = context->getUniqueChecksInline(getPrimary());

		// perform chunking if:
		//   - a single statement exceeds transaction count
		//   - if we're ignoring unique checks (used during deep dump/load)
		if ((loadData == true) || (uniqueChecks == false)) {
			Conductor* conductor = transaction->getConductor(getIdentifier());
			if (conductor->m_createStats >= Properties::getTransactionChunk()) {
				bool reserved = context->inReservedTransactionInline(getPrimary());

				BasicArray<inttype, true> transactionTypes(Properties::DEFAULT_TRANSACTION_DEPTH, false);

				shorttype level = context->getTransactionLevel();
				for (shorttype i = 0; i <= level; i++) {
					transactionTypes.set(i, context->getTransactionType(i), true);
				}

				context->commitTransaction();

				for (shorttype i = 0; i <= level; i++) {
					context->beginTransaction((DeepThreadContext::TransactionType) transactionTypes.get(i));
				}

				associateTransaction(transaction);

				context->setReserved(getPrimary(), reserved);
				context->setLoadData(getPrimary(), loadData);
				context->setUniqueChecks(getPrimary(), uniqueChecks);

				#ifdef XXX_DEBUG
				if (cxx::util::Logger::isDebugEnabled() == true) {
					DEEP_LOG(DEBUG, OTHER, "(LOAD DATA): transaction count exceeded, commit/begin new transaction, store: %s\n", m_map->getFilePath());
				}
				#endif
			}
		}
	}

	return status ? CT_DATASTORE_SUCCESS : CT_DATASTORE_ERROR;
}

template<typename K>
int Datastore<K>::update(DeepThreadContext* context, const unsigned char* oldRecord, unsigned int oldLength, const unsigned char* newRecord, unsigned int newLength, bool lock) {
	return update(context, m_keyBuilder->getKey((bytearray) oldRecord), oldRecord, oldLength, m_keyBuilder->getKey((bytearray) newRecord), newRecord, newLength, lock);
}

template<>
int Datastore<DeepComposite*>::update(DeepThreadContext* context, const unsigned char* oldRecord, unsigned int oldLength, const unsigned char* newRecord, unsigned int newLength, bool lock) {
	DeepComposite oldKey((const inttype) m_keyBuilder->getUnpackLength());
	DeepComposite newKey((const inttype) m_keyBuilder->getUnpackLength());

	return update(context, m_keyBuilder->fillKey(null, (bytearray) oldRecord, &oldKey), oldRecord, oldLength, m_keyBuilder->fillKey(null, (bytearray) newRecord, &newKey), newRecord, newLength, lock);
}

template<>
int Datastore<DeepNByte*>::update(DeepThreadContext* context, const unsigned char* oldRecord, unsigned int oldLength, const unsigned char* newRecord, unsigned int newLength, bool lock) {
	DeepNByte oldKey((const inttype) m_keyBuilder->getUnpackLength());
	DeepNByte newKey((const inttype) m_keyBuilder->getUnpackLength());

	return update(context, m_keyBuilder->fillKey(null, (bytearray) oldRecord, &oldKey), oldRecord, oldLength, m_keyBuilder->fillKey(null, (bytearray) newRecord, &newKey), newRecord, newLength, lock);
}

template<typename K>
int Datastore<K>::update(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock) {
	return update(context, *((K*) key), record, length, lock);
}

template<>
int Datastore<DeepComposite*>::update(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock) {
	DeepComposite bKey((const inttype) m_keyBuilder->getUnpackLength());

	m_keyBuilder->packKey(&bKey, (const bytearray) key);

	return update(context, &bKey, record, length, lock);
}

template<>
int Datastore<DeepNByte*>::update(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock) {
	DeepNByte bKey((const bytearray) key, m_keyBuilder->getPackLength((const bytearray) key));

	return update(context, &bKey, record, length, lock);
}

template<typename K>
int Datastore<K>::update(DeepThreadContext* context, const K oldKey, const unsigned char* oldRecord, unsigned int oldLength, const K newKey, const unsigned char* newRecord, unsigned int newLength, bool lock) {

	int status = CT_DATASTORE_SUCCESS;

	// XXX: use quick equality check here instead of comparator
	if (m_keyBuilder->isEqual(oldKey, newKey) == true) {
		status = update(context, newKey, newRecord, newLength, lock);

	} else {
		// XXX: set savepoint so we can roll all the way back if necessary
		short savepoint = context->beginTransaction(DeepThreadContext::SAVEPOINT);

		// XXX: model this remove/put after equivalent transaction to maintain consistency (DATABASE-1909 rekey memory leak on RealTimeMap::commitCacheMemory)
		short level = context->beginTransaction(DeepThreadContext::STATEMENT);

		nbyte* oldValue = ((nbyte*) context->value)->reassign((const bytearray) oldRecord, oldLength);
		if (m_map->remove(oldKey, oldValue, RealTime::DELETE_POPULATED, (Transaction*) context->getTransactionInline()) == true) {
			nbyte* newValue = ((nbyte*) context->value)->reassign((const bytearray) newRecord, newLength);

			context->commitTransaction(level);

			context->beginTransaction(DeepThreadContext::STATEMENT);
			
			if (m_map->put(newKey, newValue, RealTime::UNIQUE, (Transaction*) context->getTransactionInline()) == true) {
				// XXX: commit savepoint
				context->commitTransaction(savepoint);

			} else {
				// XXX: rollback to savepoint
				context->rollbackTransaction(savepoint);

				status = CT_DATASTORE_ERROR;
			}

		} else {
			// XXX: rollback to savepoint
			context->rollbackTransaction(savepoint);

			status = CT_DATASTORE_ERROR;
		}
	}

	return status;
}

template<typename K>
int Datastore<K>::update(DeepThreadContext* context, const K key, const unsigned char* record, unsigned int length, bool lock) {
	bool status = true;

	RealTime::LockOption lockOption = (RealTime::LockOption) ((lock == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE);

	nbyte* value = ((nbyte*) context->value)->reassign((const bytearray) record, length);

	Transaction* transaction = (Transaction*) context->getTransactionInline();

	status = m_map->put(key, value, RealTime::EXISTING, transaction, lockOption);

	return status ? CT_DATASTORE_SUCCESS : CT_DATASTORE_ERROR;
}

template<typename K>
int Datastore<K>::remove(DeepThreadContext* context, const unsigned char* record, unsigned int length, bool lock) {
	return remove(context, m_keyBuilder->getKey((bytearray) record), record, length, lock);
}

template<>
int Datastore<DeepComposite*>::remove(DeepThreadContext* context, const unsigned char* record, unsigned int length, bool lock) {
	DeepComposite key((const inttype) m_keyBuilder->getUnpackLength());

	return remove(context, m_keyBuilder->fillKey(null, (bytearray) record, &key), record, length, lock);
}

template<>
int Datastore<DeepNByte*>::remove(DeepThreadContext* context, const unsigned char* record, unsigned int length, bool lock) {
	DeepNByte key((const inttype) m_keyBuilder->getUnpackLength());

	return remove(context, m_keyBuilder->fillKey(null, (bytearray) record, &key), record, length, lock);
}

template<typename K>
int Datastore<K>::remove(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock) {
	return remove(context, *((K*) key), record, length, lock);
}

template<>
int Datastore<DeepComposite*>::remove(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock) {
	DeepComposite bKey((const inttype) m_keyBuilder->getUnpackLength());

	m_keyBuilder->packKey(&bKey, (const bytearray) key);

	return remove(context, &bKey, record, length, lock);
}

template<>
int Datastore<DeepNByte*>::remove(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock) {
	DeepNByte bKey((const bytearray) key, m_keyBuilder->getPackLength((const bytearray) key));

	return remove(context, &bKey, record, length, lock);
}

template<typename K>
int Datastore<K>::remove(DeepThreadContext* context, const K key, const unsigned char* record, unsigned int length, bool lock) {
	bool status = true;

	RealTime::LockOption lockOption = (RealTime::LockOption) ((lock == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE);

	nbyte* value = ((nbyte*) context->value)->reassign((const bytearray) record, length);

	Transaction* transaction = (Transaction*) context->getTransactionInline();

	status = m_map->remove(key, value, RealTime::DELETE_POPULATED, transaction, lockOption);

	return status ? CT_DATASTORE_SUCCESS : CT_DATASTORE_ERROR;
}

template<typename K>
int Datastore<K>::get(DeepThreadContext* context, const unsigned char* key, unsigned char** record, unsigned int* length, unsigned char* retkey, int type, int lockOption, bool keyOnly, unsigned int keyPartMask, DeepCursor* cursor, Condition* condition, boolean packKey) {
	K rKey;

	int status = get(context, *((K*)key), record, length, &rKey, type, lockOption, keyOnly, cursor, condition);
	if (status == CT_DATASTORE_SUCCESS) {
		m_keyBuilder->copyKeyBuffer(rKey, (const bytearray) retkey);
	}

	return status;
}

template<>
int Datastore<DeepComposite*>::get(DeepThreadContext* context, const unsigned char* key, unsigned char** record, unsigned int* length, unsigned char* retkey, int type, int lockOption, bool keyOnly, unsigned int keyPartMask, DeepCursor* cursor, Condition* condition, boolean packKey) {
	int status = CT_DATASTORE_ERROR;

	bytetype keyPartWeight = ((type == CT_DATASTORE_GET_PREFIX_LAST) ||
							(type == CT_DATASTORE_GET_PREFIX_LAST_OR_PREV) ||
							(type == CT_DATASTORE_GET_NEXT) ||
							(type == CT_DATASTORE_GET_NEXT_MATCH)) ? 1 : -1;

	DeepComposite tKey((bytearray) null, 0, keyPartMask, keyPartWeight);

	if (type == CT_DATASTORE_GET_NEXT_MATCH) {
		tKey.setKeyPartMask(~0);
		tKey.calculateKeyPartMatch(keyPartMask);
	}

	if ((key != null) && (type != CT_DATASTORE_GET_LAST) && (type != CT_DATASTORE_GET_FIRST)) {
		if (type == CT_DATASTORE_GET_EXACT) {
			m_keyBuilder->setIgnorePrimary(&tKey, true);
		}

		if (packKey == true) {
			tKey.alloc(m_keyBuilder->getUnpackLength());

			m_keyBuilder->packKey(&tKey, (const bytearray) key);

		} else {
			m_keyBuilder->reassignKey(&tKey, (const bytearray) key);
		}
	}

	DeepComposite rKey((const bytearray) retkey, m_keyBuilder->getUnpackLength());
	DeepComposite* bRetKey = &rKey;

	switch(type) {
		case CT_DATASTORE_GET_PREFIX_LAST:
		case CT_DATASTORE_GET_PREFIX_LAST_OR_PREV:
			status = get(context, &tKey, record, length, &bRetKey, CT_DATASTORE_GET_PREVIOUS, lockOption, keyOnly, cursor, condition);
			break;

		default:
			status = get(context, &tKey, record, length, &bRetKey, type, lockOption, keyOnly, cursor, condition);
			if ((status != CT_DATASTORE_SUCCESS) && (type == CT_DATASTORE_GET_EXACT)) {
				if (keyPartMask == (unsigned int) ~0) {
					// full mask should not include ignored primary in this case
					m_keyBuilder->setIgnorePrimary(&tKey, true, false);
				}

				tKey.calculateKeyPartMatch(tKey.getKeyPartMask());

				status = get(context, &tKey, record, length, &bRetKey, CT_DATASTORE_GET_NEXT_MATCH, lockOption, keyOnly, null, condition);
			}
	}

	return status;
}

template<>
int Datastore<DeepNByte*>::get(DeepThreadContext* context, const unsigned char* key, unsigned char** record, unsigned int* length, unsigned char* retkey, int type, int lockOption, bool keyOnly, unsigned int keyPartMask, DeepCursor* cursor, Condition* condition, boolean packKey) {
	DeepNByte* bKey = (DeepNByte*) ((DeepNByte*) context->key)->reassign((const bytearray) key, (key == null) ? 0 : m_keyBuilder->getPackLength((const bytearray) key));
	DeepNByte* rKey = (DeepNByte*) ((DeepNByte*) context->returnKey)->reassign((const bytearray) retkey, (const inttype) m_keyBuilder->getUnpackLength());

	int status = get(context, bKey, record, length, &rKey, type, lockOption, keyOnly, cursor, condition);

	return status;
}

template<typename K>
int Datastore<K>::get(DeepThreadContext* context, const K key, unsigned char** record, unsigned int* length, K* retkey, int type, int lockOption, bool keyOnly, DeepCursor* cursor, Condition* condition) {
	int status = CT_DATASTORE_SUCCESS;

	// XXX: lock will override a key only scan
	nbyte* value = (keyOnly && (lockOption == CT_DATASTORE_LOCK_NONE)) ? null : ((nbyte*) context->value)->reassign((const bytearray) *record, *length);

	typename Cursor<K>::MapIterator* iterator = (cursor != NULL) ? ((Cursor<K>*) cursor)->m_iterator : NULL;

	DeepCondition<K> deepCondition(condition);
	DeepCondition<K>* rtCondition = (condition != NULL) ? &deepCondition : NULL;

	Transaction* transaction = (Transaction*) context->getTransactionInline();

	switch(type) {
		case CT_DATASTORE_GET_PREFIX_LAST:
			if (m_map->get(key, value, (RealTime::ReadOption) CT_DATASTORE_GET_EXACT, retkey, transaction, (RealTime::LockOption) lockOption, iterator, rtCondition) == false) {
				status = CT_DATASTORE_ERROR;
			}
			break;

		case CT_DATASTORE_GET_PREFIX_LAST_OR_PREV:
			if (m_map->get(key, value, (RealTime::ReadOption) CT_DATASTORE_GET_EXACT, retkey, transaction, (RealTime::LockOption) lockOption, iterator, rtCondition) == false) {
				if (m_map->get(key, value, (RealTime::ReadOption) CT_DATASTORE_GET_PREVIOUS, retkey, transaction, (RealTime::LockOption) lockOption, iterator, rtCondition) == false) {
					status = CT_DATASTORE_ERROR;
				}
			}
			break;

		default:
			if (m_map->get(key, value, (RealTime::ReadOption) type, retkey, transaction, (RealTime::LockOption) lockOption, iterator, rtCondition) == false) {
				status = CT_DATASTORE_ERROR;
			}
			break;
	}

	if (value != null) {
		*record = (unsigned char*) ((bytearray) *value);
		*length = value->length;
	}

	return status;
}

template<typename K>
int Datastore<K>::getPutOption(DeepThreadContext* context, const K key) {
	int option = CT_DATASTORE_PUT_UNIQUE;

/*  TODO: implement override
	if (context->getOverrideInline(getPrimary()) == true) {
		// XXX: run regression using this
		//option = CT_DATASTORE_PUT_OVERRIDE;

	} else
*/
	if (context->inReservedTransactionInline(getPrimary()) == true) {
		unsigned long long reservedBlock = context->getReservedBlock(getPrimary());

		if (reservedBlock == 0) {
			option = CT_DATASTORE_PUT_RESERVED;
		} else {
			unsigned long long reservedKey = context->getReservedKey(getPrimary());
			unsigned long long resKey = m_keyBuilder->getReservedKey(key);

			boolean keyWithinBlock = (resKey >= reservedKey) && (resKey < (reservedKey + reservedBlock)); 

			if (keyWithinBlock == true) {
				option = CT_DATASTORE_PUT_RESERVED;
			}
		}

	} else if (m_uniqueKey != null) {
		option = CT_DATASTORE_PUT_RESERVED;
	}

	return option;
}

template<typename K>
int Datastore<K>::getReservedKeyBlock(unsigned long long offset, unsigned long long block, unsigned long long* first, unsigned long long* reserved, DeepThreadContext* context) {
	int status = CT_DATASTORE_SUCCESS;

	/* XXX: allow reservation greater than transaction chunk
	if (block > (ulongtype) Properties::getTransactionChunk()) {
		block = Properties::getTransactionChunk();
	}
	*/

	if (m_map->reserve(offset, block, *first, *reserved, (Transaction*) context->getTransactionInline()) == false) {
		status = CT_DATASTORE_ERROR;
	}

	return status;
}

template<typename K>
unsigned long long Datastore<K>::getNextReservedKey(DeepThreadContext* context) {
	unsigned long long nextKey = 0;

	if (m_isReservedKey == true) {
		K rKey = m_keyBuilder->newKey();

		bool status = m_map->get((K) Converter<K>::NULL_VALUE, (nbyte*) null, RealTime::LAST, &rKey, (Transaction*) context->getTransactionInline());
		if (status == true) {
			nextKey = m_keyBuilder->getReservedKey(rKey) + 1;
		}

		Converter<K>::destroy(rKey);
	}

	return nextKey;
}

template<>
unsigned long long Datastore<DeepULongLong>::generateUniqueKey() {
	return m_uniqueKey->incrementAndGet();
}

template<typename K>
unsigned long long Datastore<K>::generateUniqueKey() {
	DEEP_LOG(ERROR, OTHER, "Invalid hidden key type\n");
	throw UnsupportedOperationException("Invalid hidden key type");
}

template<typename K>
int Datastore<K>::addKeyPart(unsigned int fieldIndex, unsigned char type, unsigned short length, unsigned int valueOffset, unsigned int nullOffset, unsigned char nullBit, bool isIgnored, bool isReserved, short variablePosition, short primaryPosition) {

	m_schemaBuilder->addKeyPart(createDeepKeyPart(fieldIndex, type, length, valueOffset, nullOffset, nullBit, isIgnored, isReserved, variablePosition, primaryPosition));

	return CT_DATASTORE_SUCCESS;
}

template<typename K>
int Datastore<K>::addField(unsigned char type, unsigned char realType, unsigned int packLength, unsigned int rowPackLength, unsigned int keyLength, 
			   unsigned int lengthBytes, unsigned short index, unsigned char nullBit, unsigned int nullOffset, unsigned int valueOffset,
			   int characterSet, bool gcolVirtual, const char* fieldName) {
	m_schemaBuilder->addField(new DeepField(type, realType, packLength, rowPackLength, keyLength, lengthBytes, index, nullBit, nullOffset, valueOffset, characterSet, gcolVirtual, fieldName));
	
	return 0;
}

template<typename K>
void Datastore<K>::populateKey(const unsigned char* record, unsigned char* key) {
	m_keyBuilder->populateKey((const bytearray) record, (bytearray) key);
}

template<>
void Datastore<DeepComposite*>::populateKey(const unsigned char* record, unsigned char* key) {
	DeepComposite rKey((const inttype) m_keyBuilder->getUnpackLength());

	m_keyBuilder->populateKey((const bytearray) record, (bytearray) rKey);
	m_keyBuilder->unpackKey(&rKey, (bytearray) key);
}

template<typename K>
int Datastore<K>::compareKey(DeepThreadContext* context, const unsigned char* key1, const unsigned char* key2) {
	return m_comparator->compare(*((K*)key1), *((K*)key2));
}

template<>
int Datastore<DeepComposite*>::compareKey(DeepThreadContext* context, const unsigned char* key1, const unsigned char* key2) {
	DeepComposite cKey1((const inttype) m_keyBuilder->getUnpackLength());
	DeepComposite cKey2((const inttype) m_keyBuilder->getUnpackLength());

	m_keyBuilder->packKey(&cKey1, (const bytearray) key1);
	m_keyBuilder->packKey(&cKey2, (const bytearray) key2);

	return m_comparator->compare(&cKey1, &cKey2);
}

template<>
int Datastore<DeepNByte*>::compareKey(DeepThreadContext* context, const unsigned char* key1, const unsigned char* key2) {
	DeepNByte* bKey1 = (DeepNByte*) ((DeepNByte*) context->key)->reassign((const bytearray) key1, m_keyBuilder->getPackLength((const bytearray) key1));
	DeepNByte* bKey2 = (DeepNByte*) ((DeepNByte*) context->returnKey)->reassign((const bytearray) key2, m_keyBuilder->getPackLength((const bytearray) key2));

	return m_comparator->compare(bKey1, bKey2);
}

template<typename K>
long long Datastore<K>::size(DeepThreadContext* context) {
	return m_map->size((Transaction*) context->getTransactionInline());
}

template<typename K>
long long Datastore<K>::length(DeepThreadContext* context, int option) {
	return m_map->length((RealTime::FileOption) option);
}

template<typename K>
long long Datastore<K>::meanRecLength(DeepThreadContext* context) {
	long long length = 0;

	long long rows = size(context);
	if (rows > 0) {
		const ExtraStatistics* stats = m_map->getExtraStats();

		length = stats->getUserSpaceSize() / rows;
	}

	return length;
}

template<typename K>
long long Datastore<K>::range(DeepThreadContext* context, const unsigned char* startKey, int startType, unsigned int startMask, const unsigned char* endKey, int endType, unsigned int endMask) {
	return m_map->range((K*)startKey, (K*)endKey);
}

template<>
long long Datastore<DeepComposite*>::range(DeepThreadContext* context, const unsigned char* startKey, int startType, unsigned int startMask, const unsigned char* endKey, int endType, unsigned int endMask) {
	
	DeepComposite* sKey = NULL;
	DeepComposite* eKey = NULL;

	if (startKey != null) {
		bytetype startWeight = ((startType == CT_DATASTORE_GET_EXACT) || (startType == CT_DATASTORE_GET_EXACT_OR_NEXT)) ? -1 : 1;

		sKey = new DeepComposite((const inttype) m_keyBuilder->getUnpackLength(), startMask, startWeight);
		m_keyBuilder->packKey(sKey, (const bytearray) startKey);
	}

	if (endKey != null) {
		bytetype endWeight = ((endType == CT_DATASTORE_GET_NEXT) || (endType == CT_DATASTORE_GET_EXACT_OR_NEXT)) ? 1 : -1;

		eKey = new DeepComposite((const inttype) m_keyBuilder->getUnpackLength(), endMask, endWeight);
		m_keyBuilder->packKey(eKey, (const bytearray) endKey);
	}

	long long recs =  m_map->range(startKey ? &sKey : NULL, endKey ? &eKey : NULL);

	if (startKey != null) {
		delete sKey;
	}

	if (endKey != null) {
		delete eKey;
	}

	return recs;
}

template<>
long long Datastore<DeepNByte*>::range(DeepThreadContext* context, const unsigned char* startKey, int startType, unsigned int startMask, const unsigned char* endKey, int endType, unsigned int endMask) {
	
	DeepNByte* bStartKey = NULL;
	DeepNByte* bEndKey = NULL;

	if (startKey){
		bStartKey = (DeepNByte*) ((DeepNByte*) context->key)->reassign((const bytearray) startKey, m_keyBuilder->getPackLength((const bytearray) startKey));
	}
	if (endKey){
		bEndKey = (DeepNByte*) ((DeepNByte*) context->returnKey)->reassign((const bytearray) endKey, m_keyBuilder->getPackLength((const bytearray) endKey));
	}

	return m_map->range(startKey ? &bStartKey : NULL, endKey ? &bEndKey : NULL);
}

template<typename K>
void Datastore<K>::cardinality(DeepThreadContext* context, long long* stats, bool recalculate) {
	m_map->cardinality(stats, (Transaction*) null, recalculate);
}

template<typename K>
void Datastore<K>::acquireCursor(DeepThreadContext* context, DeepCursor* cursor) {
	m_map->acquire(((Cursor<K>*) cursor)->m_iterator);
}

template<typename K>
void Datastore<K>::releaseCursor(DeepThreadContext* context, DeepCursor* cursor) {
	m_map->release(((Cursor<K>*) cursor)->m_iterator);
}

template<typename K>
DeepCursor* Datastore<K>::createCursor(DeepThreadContext* context) {
	return new Cursor<K>();
}

template<typename K>
void Datastore<K>::unlockRow(DeepThreadContext* context) {
	#ifdef DEEP_LAST_ROW_UNLOCK
	Transaction* transaction = (Transaction*) context->getTransactionInline();

	m_map->unlock(transaction);
	#endif
}

template<typename K>
int Datastore<K>::getErrorCode() {
	//printf("ERROR CODE : %d\n", m_map->getErrorCode());

	switch(m_map->getErrorCode()) {
		case RealTime::ERR_SUCCESS:
			return CT_DATASTORE_SUCCESS;
		case RealTime::ERR_DEADLOCK:
			return CT_DATASTORE_ERROR_DEADLOCK;
		case RealTime::ERR_NOT_FOUND:
			return CT_DATASTORE_ERROR_KEY_NOT_FOUND;
		case RealTime::ERR_DUPLICATE:
			return CT_DATASTORE_ERROR_DUP_KEY;
		case RealTime::ERR_TIMEOUT:
			return CT_DATASTORE_ERROR_LOCK_TIMEOUT;
		case RealTime::ERR_GENERAL:
		default:
			return CT_DATASTORE_ERROR;
	}
}

template <typename K>
void Datastore<K>::getErrorKey(unsigned char* retkey) {
	K key = m_map->getErrorKey();
	m_keyBuilder->copyKeyBuffer(key, (const bytearray) retkey);
}

template <typename K>
unsigned char* Datastore<K>::getHiddenKey(unsigned char* key) {
	LOGGING_ERROR("Unsupported getHiddenKey for key type\n");

	throw UnsupportedOperationException("Unsupported getHiddenKey for key type\n");
}

template<>
unsigned char* Datastore<DeepComposite*>::getHiddenKey(unsigned char* key) {
	DeepComposite rKey((const bytearray) key, m_keyBuilder->getUnpackLength());

	return (unsigned char*) m_keyBuilder->getHiddenKey(&rKey);
}

template<typename K>
const char* Datastore<K>::getName() {
	return m_keyName.c_str();
}

template<typename K>
int Datastore<K>::associateTransaction(void* transaction) {
	#ifdef DEEP_DISTRIBUTED
	if ((m_map->getMapFacilitator() != null) && (m_map->getMapFacilitator()->allowRemoteOwner() == true)) {
		((Transaction*)transaction)->setVirtualKeySpaceVersion(m_map->getMapFacilitator()->getVirtualKeySpaceVersion());
	}
	#endif
	return m_map->associate((Transaction*) transaction) ? CT_DATASTORE_SUCCESS : CT_DATASTORE_ERROR;
}

template<typename K>
void Datastore<K>::printKey(DeepThreadContext* context, const K key) {
	// TODO: reuse buffer on context
	char buffer[4096];

	memset(buffer, 0, 4096);

	m_keyBuilder->toString(key, buffer);

	printf("Key : %s\n", buffer);
}

static String OPT_INDEX_ORIENTATION_str("(^|[ \t\r\n\v\f])deep_index_orientation[ \t\r\n\v\f]*=[ \t\r\n\v\f]*([a-zA-Z]*)");
template<typename K>
Pattern Datastore<K>::OPT_INDEX_ORIENTATION = Pattern::compile(OPT_INDEX_ORIENTATION_str);

DeepCharset DefaultCharset;
DeepCharset* DeepStore::s_charset = &DefaultCharset;

int DeepCharset::compare(int charset, char* key1, unsigned short length1, char* key2, unsigned short length2) {
	int cmp = strncmp(key1, key2, min(length1, length2));

	if ((cmp == 0) && (length1 != length2)) {
		cmp = (length1 > length2) ? 1 : -1;
	}

	return cmp;
}

unsigned short DeepCharset::populate(int charset, char* keyBuffer,
		const char* value, unsigned int valueLengthInBytes,
		unsigned short maxKeyLengthInBytes) {

	unsigned short length = (valueLengthInBytes < maxKeyLengthInBytes) ? valueLengthInBytes : maxKeyLengthInBytes;

	memcpy(keyBuffer, value, length);

	return length;
}

unsigned int DeepCharset::getMultiByteMaxLen(int charset) {
	return 1;
}

bool DeepCharset::getMultiBytePack() {
	return false;
}

DeepThreadContext::DeepThreadContext() {
	DeepThreadInners* inners = new DeepThreadInners();

	inners->m_transaction = Transaction::create();
	m_inners = inners;

	m_lockCount = 0;

	value = new nbyte((const bytearray) null, 0);

	key = new DeepNByte((const bytearray) null, 0);
	returnKey = new DeepNByte((const bytearray) null, 0);

	compositeKey = new DeepComposite((const bytearray) null, 0);
	compositeReturnKey = new DeepComposite((const bytearray) null, 0);
}

DeepThreadContext::~DeepThreadContext() {
	DeepThreadInners* inners = (DeepThreadInners*) m_inners;
	// XXX: for debugging reasons
	m_inners = null;

	Transaction::destroy(inners->m_transaction);
	// XXX: for debugging reasons
	inners->m_transaction = null;

	delete inners;

	delete ((nbyte*) value);

	delete ((DeepNByte*) key);
	delete ((DeepNByte*) returnKey);

	delete ((DeepComposite*) compositeKey);
	delete ((DeepComposite*) compositeReturnKey);
}

void* DeepThreadContext::getTransactionInline() {
	return ((DeepThreadInners*) m_inners)->m_transaction;
}

bool DeepThreadContext::inReservedTransactionInline(DeepStore* datastore) {
	return ((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->getReserved();
}

unsigned long long DeepThreadContext::getReservedKey(DeepStore* datastore) {
	return ((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->getReservedKey();
}

unsigned long long DeepThreadContext::getReservedBlock(DeepStore* datastore) {
	return ((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->getReservedBlock();
}

bool DeepThreadContext::inLoadDataTransactionInline(DeepStore* datastore) {
	return ((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->getLoadData();
}

bool DeepThreadContext::getOverrideInline(DeepStore* datastore) {
	// TODO: implement override
	//return ((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->getOverride();
	return false;
}

bool DeepThreadContext::getUniqueChecksInline(DeepStore* datastore) {
	return ((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->getUniqueChecks();
}

void DeepThreadContext::setReserved(DeepStore* datastore, bool reserved) {
	((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->setReserved(reserved);
}

void DeepThreadContext::setLoadData(DeepStore* datastore, bool loadData) {
	((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->setLoadData(loadData);
}

void DeepThreadContext::setOverride(DeepStore* datastore, bool override) {
	// TODO: implement override
	//((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->setOverride(override);
}

void DeepThreadContext::setUniqueChecks(DeepStore* datastore, bool uniqueChecks) {
	((DeepThreadInners*) m_inners)->m_transaction->getConductor(datastore->getIdentifier())->setUniqueChecks(uniqueChecks);
}

short DeepThreadContext::beginTransaction(TransactionType type, int isolation) {
	DeepThreadInners* inners = (DeepThreadInners*) m_inners;

	if ((inners->m_transaction->getLevel() == -1) && (isolation != CT_DATASTORE_ISO_INHERIT)) {
		inners->m_transaction->setIsolation((Transaction::Isolation) isolation);
	}

	shorttype level = inners->m_transaction->begin();
	if (level >= 0) {
		if ((type == DeepThreadContext::SAVEPOINT) && (level >= Properties::MAXIMUM_TRANSACTION_DEPTH)) {
			LOGGING_ERROR("Exceeded maximum transaction depth: %d\n", level);

			inners->m_transaction->rollback(level);

			level = -1;

		} else {
			inners->m_transactionTypes.set(level, type, true);
		}
	}

	return level;
}

short DeepThreadContext::prepareTransaction(short level) {
	// TODO: DATABASE-190
	//DeepThreadInners* inners = (DeepThreadInners*) m_inners;

	//inners->m_transaction->prepare(level);

	return level;
}

short DeepThreadContext::commitTransaction(short level) {
	DeepThreadInners* inners = (DeepThreadInners*) m_inners;

	for (shorttype i = inners->m_transaction->getLevel(); i >= level; i--) {
		inners->m_transaction->commit(i);
	}

	// XXX (set when size is used): inners->m_transactionTypes.size(0);

	return inners->m_transaction->getLevel();
}

short DeepThreadContext::rollbackTransaction(short level) {
	DeepThreadInners* inners = (DeepThreadInners*) m_inners;

	for (shorttype i = inners->m_transaction->getLevel(); i >= level; i--) {
		inners->m_transaction->rollback(i);
	}

	// XXX (set when size is used): inners->m_transactionTypes.size(0);

	return inners->m_transaction->getLevel();
}

short DeepThreadContext::associateTransaction(DeepStore* datastore) {
	DeepThreadInners* inners = (DeepThreadInners*) m_inners;

	datastore->associateTransaction(inners->m_transaction);

	return inners->m_transaction->getLevel();
}

bool DeepThreadContext::inTransaction(TransactionType type) {
	DeepThreadInners* inners = (DeepThreadInners*) m_inners;

	shorttype level = getTransactionLevel();
	if (level >= 0) {
		if (type == DeepThreadContext::SAVEPOINT) {
			for (shorttype i = level; i >= 0; i--) {
				if (inners->m_transactionTypes.get(i) == type) {
					return true;
				}
			}

		} else {
			return (inners->m_transactionTypes.get(level) == type);
		}
	}

	return false;
}

DeepThreadContext::TransactionType DeepThreadContext::getTransactionType(short level) {
	if (level <= getTransactionLevel()) {
		DeepThreadInners* inners = (DeepThreadInners*) m_inners;

		return (DeepThreadContext::TransactionType) inners->m_transactionTypes.get(level);

	} else {
		return DeepThreadContext::NONE;
	}
}

bool DeepThreadContext::isCacheAllowed(DeepStore* datastore) {
/*
	DeepThreadInners* inners = (DeepThreadInners*) m_inners;

printf("isCacheAllowed() %d\n", (inners->m_transaction->modifiedConductor(datastore->getIdentifier()) == false));

	return (inners->m_transaction->modifiedConductor(datastore->getIdentifier()) == false);
*/
	return false;
}

short DeepThreadContext::getTransactionLevel() {
	return ((DeepThreadInners*) m_inners)->m_transaction->getLevel();
}

} } } } // namespace
