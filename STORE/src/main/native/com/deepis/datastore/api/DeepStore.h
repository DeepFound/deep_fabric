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
#ifndef COM_DEEPIS_DATASTORE_API_DEEPSTORE_H_
#define COM_DEEPIS_DATASTORE_API_DEEPSTORE_H_

#define CT_DATASTORE_BASE_EXT ".deep"

// see: Map::ReadOption
#define CT_DATASTORE_GET_EXACT                1
#define CT_DATASTORE_GET_EXACT_OR_NEXT        2
#define CT_DATASTORE_GET_NEXT                 3
#define CT_DATASTORE_GET_NEXT_MATCH           4
#define CT_DATASTORE_GET_EXACT_OR_PREVIOUS    5
#define CT_DATASTORE_GET_PREVIOUS             6
#define CT_DATASTORE_GET_FIRST                7
#define CT_DATASTORE_GET_LAST                 8
// Datastore extensions for composite keys
#define CT_DATASTORE_GET_PREFIX_LAST          9
#define CT_DATASTORE_GET_PREFIX_LAST_OR_PREV  10

// see: Map::WriteOption
#define CT_DATASTORE_PUT_STANDARD          0
#define CT_DATASTORE_PUT_EXISTING          1
#define CT_DATASTORE_PUT_OVERRIDE          2
#define CT_DATASTORE_PUT_RESERVED          3
#define CT_DATASTORE_PUT_UNIQUE            4

// see: Map::LockOption
#define CT_DATASTORE_LOCK_NONE             0
#define CT_DATASTORE_LOCK_READ             1
#define CT_DATASTORE_LOCK_WRITE            2

// see: Map::FileOption
#define CT_DATASTORE_FILE_KEY              0
#define CT_DATASTORE_FILE_VALUE            1
#define CT_DATASTORE_FILE_PAIRED           2

// see: Map::OptimizeOption
#define CT_DATASTORE_OPTIMIZE_ONLINE_KEY       0
#define CT_DATASTORE_OPTIMIZE_ONLINE_VALUE     1
#define CT_DATASTORE_OPTIMIZE_ONLINE_PAIRED    2
#define CT_DATASTORE_OPTIMIZE_OFFLINE_KEY      3
#define CT_DATASTORE_OPTIMIZE_OFFLINE_VALUE    4
#define CT_DATASTORE_OPTIMIZE_OFFLINE_PAIRED   5

#define CT_DATASTORE_INDEX_ORIENTATION_ROW     0
#define CT_DATASTORE_INDEX_ORIENTATION_COLUMN  1

// see: Transaction::Isolation
#define CT_DATASTORE_ISO_INHERIT          -1
#define CT_DATASTORE_ISO_SERIALIZABLE      0
#define CT_DATASTORE_ISO_REPEATABLE        1
#define CT_DATASTORE_ISO_SNAPSHOT          2
#define CT_DATASTORE_ISO_COMMITTED         3
#define CT_DATASTORE_ISO_UNCOMMITTED       4

#define CT_DATASTORE_DEBUG_OPTIONS_MAX     64
#define CT_DATASTORE_PROFILING_OPTIONS_MAX 64
#define CT_DATASTORE_LOG_OPTIONS_MAX       64

#define CT_DATASTORE_PROFILING_TRIGGER_INTERVAL_DEFAULT 1000
#define CT_DATASTORE_PROFILING_TRIGGER_INTERVAL_MIN 10

#define CT_DATASTORE_STATISTICS_FLUSH_INTERVAL_DEFAULT 3600
#define CT_DATASTORE_STATISTICS_FLUSH_INTERVAL_MIN 60

// map to deep key types
#define CT_DATASTORE_TEXT             1
#define CT_DATASTORE_BINARY           2
#define CT_DATASTORE_SHORT_INT        3
#define CT_DATASTORE_LONG_INT         4
#define CT_DATASTORE_FLOAT            5
#define CT_DATASTORE_DOUBLE           6
#define CT_DATASTORE_NUM              7
#define CT_DATASTORE_USHORT_INT       8
#define CT_DATASTORE_ULONG_INT        9
#define CT_DATASTORE_LONGLONG         10
#define CT_DATASTORE_ULONGLONG        11
#define CT_DATASTORE_INT24            12
#define CT_DATASTORE_UINT24           13
#define CT_DATASTORE_INT8             14
#define CT_DATASTORE_VARTEXT1         15
#define CT_DATASTORE_VARBINARY1       16
#define CT_DATASTORE_VARTEXT2         17
#define CT_DATASTORE_VARBINARY2       18
#define CT_DATASTORE_BIT              19
// deepis extension key types
#define CT_DATASTORE_COMPOSITE        20
#define CT_DATASTORE_NUMBER_NULL      21
#define CT_DATASTORE_HIDDEN           22
#define CT_DATASTORE_UINT8            23
#define CT_DATASTORE_FIXED_VARTEXT1   24
#define CT_DATASTORE_FIXED_VARBINARY1 25

#define CT_DATASTORE_BLOB             26

#define CT_DATASTORE_JSON             27
#define CT_DATASTORE_TEXT_MULTIBYTE   28

#define CT_DATASTORE_OPTION_CREATE          0x01
#define CT_DATASTORE_OPTION_DELETE          0x02
/* #define CT_DATASTORE_OPTION_STREAM       0x04 */
#define CT_DATASTORE_OPTION_MEMORY          0x08
#define CT_DATASTORE_OPTION_MEMORY_COMPRESS 0x10
#define CT_DATASTORE_OPTION_CLEANUP         0x20
/* #define CT_DATASTORE_OPTION_SINGULAR     0x40 */
#define CT_DATASTORE_OPTION_PREFETCH        0x80
#define CT_DATASTORE_OPTION_FIXEDKEY        0x100
#define CT_DATASTORE_OPTION_ROWSTORE        0x200
#define CT_DATASTORE_OPTION_KEY_COMPRESS    0x400
#define CT_DATASTORE_OPTION_VALUE_COMPRESS  0x800
#define CT_DATASTORE_STATIC_CONTEXT         0x1000

#define CT_DATASTORE_CREATE_DEFAULT                  1
#define CT_DATASTORE_DELETE_DEFAULT                  0
#define CT_DATASTORE_MEMORY_DEFAULT                  0
#define CT_DATASTORE_DURABLE_DEFAULT                 1
#define CT_DATASTORE_CLEANUP_DEFAULT                 1
#define CT_DATASTORE_MEMORY_COMPRESS_DEFAULT         1
#define CT_DATASTORE_KEY_COMPRESS_DEFAULT            1
#define CT_DATASTORE_VALUE_COMPRESS_DEFAULT          0
#define CT_DATASTORE_VALUE_COMPRESS_PERCENT_DEFAULT 15
#define CT_DATASTORE_AUTOMATIC_CHECKPOINT_INTERVAL_DEFAULT 900
#define CT_DATASTORE_AUTOMATIC_CHECKPOINT_INTERVAL_MIN     60
#define CT_DATASTORE_PREFETCH_DEFAULT                0

#define CT_DATASTORE_ACTIVATION_KEY_VALID   0
#define CT_DATASTORE_ACTIVATION_KEY_EXPIRED 1
#define CT_DATASTORE_ACTIVATION_KEY_DEFAULT 2
#define CT_DATASTORE_ACTIVATION_KEY_INVALID 3

#define CT_DATASTORE_ACTIVATION_KEY_MAX 128
#define CT_DATASTORE_ACTIVATION_KEY_DEFAULT_STR "xxxxxxxxxxxxxxx"
#define CT_DATASTORE_SYSTEM_UID_DEFAULT 334565

#define CT_DATASTORE_THREAD_LIMIT_DEFAULT          1024 + 1
#define CT_DATASTORE_CACHE_SIZE_MIN                (1024 * 1024 * 100) /* 100MB */
#define CT_DATASTORE_CACHE_SIZE_DEFAULT            0 /* use avaiable            */
#define CT_DATASTORE_CACHE_PERCENT_DEFAULT         0.70 /* use limit            */
#define CT_DATASTORE_FILE_SIZE_DEFAULT             2147483648 /* 2G             */
#if 0
#define CT_DATASTORE_TRT_FILE_SIZE_DEFAULT         104857600 /* 100M           */
#else
#define CT_DATASTORE_TRT_FILE_SIZE_DEFAULT         0 /* OFF                    */
#endif
#define CT_DATASTORE_FILE_SIZE_MIN                 (1024 * 1024 * 100) /* 100MB */
#define CT_DATASTORE_FILE_SIZE_MAX                 4294967295 /* 4G - 1         */
#define CT_DATASTORE_LOCK_TIMEOUT_DEFAULT          50
#define CT_DATASTORE_FILE_DESCRIPTOR_LIMIT_DEFAULT 512
#define CT_DATASTORE_INDEX_THREADS_MIN             4
#define CT_DATASTORE_INDEX_THREADS_MAX             32
#define CT_DATASTORE_INDEX_THREADS_DEFAULT         5    /* relative.core.Properties.h */
#define CT_DATASTORE_REORG_THREADS_MIN             2
#define CT_DATASTORE_REORG_THREADS_MAX             32
#define CT_DATASTORE_REORG_THREADS_DEFAULT         2    /* relative.core.Properties.h */
#define CT_DATASTORE_TRANSACTION_CHUNK_DEFAULT     1500 /* relative.core.Properties.h */
#define CT_DATASTORE_TRANSACTION_STREAM_DEFAULT    0    /* relative.core.Properties.h */
#define CT_DATASTORE_SEGMENT_SIZE_DEFAULT          1500 /* relative.core.Properties.h */

#define CT_DATASTORE_DURABLE_SYNC_INTERVAL_MIN           100 /* msec */
#define CT_DATASTORE_DURABLE_SYNC_INTERVAL_DEFAULT       100 /* msec */
#define CT_DATASTORE_DURABLE_HOLD_DOWN_TIME_DEFAULT      1  /* msec */
#define CT_DATASTORE_DURABLE_HOLD_DOWN_THRESHOLD_DEFAULT 25 /* msec */
#define CT_DATASTORE_REORG_WORK_PERCENT_DEFAULT          10
#define CT_DATASTORE_FRAGMENTATION_PERCENT_DEFAULT       80

#define CT_DATASTORE_SEEK_STATISTICS_DEFAULT                   FALSE
#define CT_DATASTORE_SEEK_STATISTICS_RESET_INTERVAL_DEFAULT    600
#define CT_DATASTORE_SEEK_STATISTICS_DISPLAY_INTERVAL_DEFAULT  30

#define CT_DATASTORE_ERROR                     -1
#define CT_DATASTORE_SUCCESS                   0
#define CT_DATASTORE_ERROR_KEY_NOT_FOUND       1  /* Didn't find key on read or update */
#define CT_DATASTORE_ERROR_DUP_KEY             2  /* Duplicate key on write */
#define CT_DATASTORE_ERROR_WRONG_COMMAND       3  /* Command not supported */
#define CT_DATASTORE_ERROR_NO_ACTIVE_RECORD    4  /* No record read in update */
#define CT_DATASTORE_ERROR_RECORD_DELETED      5  /* A record is not there */
#define CT_DATASTORE_ERROR_END_OF_FILE         6  /* end in next/prev/first/last */
#define CT_DATASTORE_ERROR_WRONG_CREATE_OPTION 7  /* Wrong create option */
#define CT_DATASTORE_ERROR_NO_SUCH_TABLE       8  /* The table does not exist */
#define CT_DATASTORE_ERROR_TABLE_EXIST         9  /* The table existed */
#define CT_DATASTORE_ERROR_LOCK_TIMEOUT        10 /* Timed out waiting on lock */
#define CT_DATASTORE_ERROR_DEADLOCK            11 /* Call would cause a deadlock */

#define CT_DATASTORE_MAX_KEY_LENGTH 3072

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {
class IMessageService;
class PeerInfo;
} } } } } } // namespace

using namespace com::deepis::db::store::relative::distributed;

namespace com { namespace deepis { namespace datastore { namespace api {

class DeepThreadContext;
class Condition;
class DeepCharset;
class DeepCursor;

typedef void (*DEEP_TIMER_CALLBACK) (void*);

class DeepStore {
	public:
		static DeepStore* create(const char* filePath,
				long long options,
				const char* keyName,
				int keyType,
				int keyLength,
				int recordLength,
				#ifdef DEEP_DISTRIBUTED
				int mapBehaviorValue = 0,
				#endif
				bool isReservedKey = false);

		static bool validatePaths(const char* filePath, const char* dataDir, const char* indexDir);

		static int clobber(const char* filePath);
		static int rename(const char* from, const char* to);

		static void setBuildNumber(int bld);
		static void setMajorVersion(int maj);
		static void setMinorVersion(int min);
		static void setRevisionNumber(int rev);

		static void setDebugEnabled(bool enabled);
		static void setInfoEnabled(bool enabled);
		static void setWarnEnabled(bool enabled);
		static void setErrorEnabled(bool enabled);

		static void setFileSize(long long fileSize);
		static void setTrtFileSize(long long fileSize);
		static void setThreadLimit(int threadLimit);
		static void setCacheSize(long long cacheSize);
		static void setLockTimeout(int timeout);
		static void setFileDescriptorLimit(int limit);
		static void setWorkThreads(int threads);
		static void setReorgThreads(int threads);
		static void setIsolationLevel(int isolation);
		static void setInfinitelimit(bool enabled);
		static void setTransactionChunk(int chunk);
		static void setTransactionStream(bool enabled);
		static void setSegmentSize(int size);
		static void setDynamicResources(bool dynamic);
		static bool getDynamicResources(void);
		static void setDurable(bool enabled);
		static void setDurableSyncInterval(long long interval);
		#ifdef DEEP_SYNCHRONIZATION_GROUPING
		static void setDurableHoldDownTime(long long time);
		static void setDurableHoldDownThreshold(long long threshold);
		#endif
		static void setValueCompressPercent(double percent);
		static void setDebugEnabled(int option, bool enabled);
		static void setProfilingEnabled(int option, bool enabled);
		static void setProfilingTriggerInterval(int interval);
		static void setStatisticsFlushInterval(int interval);
		static const char** getLoggingTopicNames(void);
		static void setLoggingTopicOptions(unsigned long long options);
		static void setLogOption(int option, bool enabled);
		static void setMemoryFragment(bool enabled);
		static void setMemoryAnalytics(bool enabled);
		static void setCheckpointMode(unsigned int mode);
		static void setAutomaticCheckpointInterval(unsigned int interval);
		static bool performManualCheckpoint(void);
		static void setRecoveryReplay(bool enabled);
		static void setRecoveryRealign(bool enabled);
		static void setRecoverySafe(bool enabled);
		static void setVirtualized(bool enabled);

		static void setSeekStatistics(bool enabled);
		static void setSeekStatisticsResetInterval(int interval);
		static void setSeekStatisticsDisplayInterval(int interval);
		static void setReorgWorkPercent(int percent);
		static void setFragmentationPercent(int percent);
		static void setSemiPurge(bool enabled);
		static void setDynamicSummarization(bool enabled);
		static void setRangeSync(bool enabled);
		static void setAllowLrtVrtMismatch(bool enabled);
		static void setCardinalityRecalculateRecovery(bool enabled);
		static void setFileRefCheckMod(int mod);

		static int generateSystemUid(void);
		static void setActivationKey(const char*);
		static const char* getActivationKey(void);
		static int validateActivationKey(const char*);

		static void shutdownCacheManagement(void);
		static void runtimeGarbageCollection(bool flag);
		static void shutdownGarbageCollection(bool flag);

		static bool checkMemoryRequirements();

		static void setCharset(DeepCharset* charset) {
			s_charset = charset;
		}

		static inline DeepCharset* getCharset() {
			return s_charset;
		}

		static void setTimer(DEEP_TIMER_CALLBACK callback, void* context);

	private:
		static DeepCharset* s_charset;

		long long m_identifier;
		DeepStore* m_primary;

	public:
		DeepStore(void) :
			m_identifier(-1) {
			m_primary = this;
		}
		virtual ~DeepStore() {}

		virtual int get(DeepThreadContext* context, const unsigned char* key, unsigned char** record, unsigned int* length, unsigned char* retkey, int type, int lockOption, bool keyOnly, unsigned int keyPartMask, DeepCursor* cursor = 0, Condition* condition = 0, bool packKey = true) = 0;
		virtual int get(DeepThreadContext* context, const unsigned char* key, unsigned char** record, unsigned int* length, unsigned char* retkey, int type, int lockOption = CT_DATASTORE_LOCK_NONE) = 0;

		virtual int put(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length) = 0;
		virtual int put(DeepThreadContext* context, const unsigned char* record, unsigned int length) = 0;

		virtual int update(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock = true) = 0;
		virtual int update(DeepThreadContext* context, const unsigned char* oldRecord, unsigned int oldLength, const unsigned char* newRecord, unsigned int newLength, bool lock = true) = 0;

		virtual int remove(DeepThreadContext* context, const unsigned char* key, const unsigned char* record, unsigned int length, bool lock = true) = 0;
		virtual int remove(DeepThreadContext* context, const unsigned char* record, unsigned int length, bool lock = true) = 0;

		virtual long long size(DeepThreadContext* context) = 0;
		virtual long long length(DeepThreadContext*, int option) = 0;
		virtual long long meanRecLength(DeepThreadContext*) = 0;
		virtual long long range(DeepThreadContext* context, const unsigned char* startKey, int startType, unsigned int startMask, const unsigned char* endKey, int endType, unsigned int endMask) = 0;
		virtual void cardinality(DeepThreadContext* context, long long* stats, bool recalculate) = 0;
		virtual void acquireCursor(DeepThreadContext* context, DeepCursor* cursor) = 0;
		virtual void releaseCursor(DeepThreadContext* context, DeepCursor* cursor) = 0;
		virtual DeepCursor* createCursor(DeepThreadContext* context) = 0;
		virtual void unlockRow(DeepThreadContext* context) = 0;

		#ifdef DEEP_DISTRIBUTED
		virtual int initialize(IMessageService* messageService = 0x0, PeerInfo* peerInfo = 0x0) = 0;
		#else
		virtual int initialize() = 0;
		#endif

		virtual int open(DeepThreadContext* context) = 0;
		virtual int close(DeepThreadContext* context = NULL) = 0;

		virtual int clear(DeepThreadContext* context = NULL, bool final = false) = 0;
		virtual int recover(DeepThreadContext* context, bool rebuild) = 0;
		virtual int optimize(DeepThreadContext* context, int type) = 0;

		virtual const char* getName() = 0;
		virtual int getErrorCode() = 0;
		virtual void getErrorKey(unsigned char* retkey) = 0;

		virtual unsigned char* getHiddenKey(unsigned char* key) = 0;

		virtual int associate(DeepStore* datastore, bool hasPrimary, bool dynamic = false) = 0;
		virtual int disassociate(DeepStore* datastore, bool dynamic = false) = 0;

		virtual int associateTransaction(void* transaction) = 0;

		virtual unsigned long long generateUniqueKey() = 0;

		virtual int getReservedKeyBlock(unsigned long long offset, unsigned long long block, unsigned long long* first, unsigned long long* reserved, DeepThreadContext* context) = 0;
		virtual unsigned long long getNextReservedKey(DeepThreadContext* context) = 0;

		virtual int addKeyPart(unsigned int fieldIndex, unsigned char type, unsigned short length, unsigned int valueOffset, unsigned int nullOffset, unsigned char nullBit, bool isIgnored = false, bool isReservedKey = false, short variablePosition = -1, short primaryPosition = -1) = 0;

		virtual void populateKey(const unsigned char* record, unsigned char* key) = 0;

		virtual int compareKey(DeepThreadContext* context, const unsigned char* key1, const unsigned char* key2) = 0;
		
		virtual int addField(unsigned char type, unsigned char realType, unsigned int packLength, unsigned int rowPackLength, unsigned int keyLength, 
				     unsigned int lengthBytes, unsigned short index, unsigned char nullBit, unsigned int nullOffset, unsigned int valueOffset,
				     int characterSet, bool gcolVirtual, const char* fieldName = "") = 0;

		virtual void* getMap() = 0;

		inline void setPrimary(DeepStore* primary) {
			m_primary = primary;
		}

		inline DeepStore* getPrimary() {
			return m_primary;
		}

		inline void setIdentifier(long long identifier) {
			m_identifier = identifier;
		}

		inline long long getIdentifier() {
			return m_identifier;
		}
		
		virtual void markSchema() = 0;

		virtual void commitSchema() = 0;

		virtual void rollbackSchema() = 0;

		virtual void setHasVariableLengthFields(bool hasVariableLengthFields) = 0;

		virtual void setHasVirtualFields(bool hasVirtualFields) = 0;
		
		virtual void setNullBytes(unsigned int nullBytes) = 0;
		
		virtual void setBlobStartOffset(unsigned int blobStartOffset) = 0;
		
		virtual void setBlobPtrSize(unsigned int blobPtrSize) = 0;
		
		virtual void setUnpackedRowLength(unsigned long long unpackedRowLength) = 0;
		
		virtual unsigned long long getAutoIncrementValue() = 0;
		
		virtual void setAutoIncrementValue(unsigned long long autoIncrementValue) = 0;
		
		virtual void setDirectoryPaths(const char* dataDir, const char* indexDir) = 0;
		
		virtual void setCharacterSet(int characterSet) = 0;
		
		virtual void setIsTemporary(bool isTemporary) = 0;
		
		virtual void setIndexOrientation(unsigned int orientation) = 0;

		virtual void setComment(const char* comment, unsigned int length) = 0;
		
		virtual char* packRow(const char* unpackedRow, unsigned int length, char** buffer, unsigned int* bufferLength) = 0;
		
		virtual char* unpackRow(char* unpackedRow, const char* packedRow, char** blobBuffer, unsigned int* blobBufferLength) = 0;
		
		virtual char* unpackRowFromKey(char* unpackedRow, const char* unpackedKey, char** blobBuffer, unsigned int* blobBufferLength, bool preserve) = 0;
};

class DeepCursor {
	public:
		virtual ~DeepCursor() {}
};

class DeepThreadContext {
	private:
		void* m_inners;
		int m_lockCount;

		inline void* getDatastoreWorkspace(DeepStore* datastore);

	public:
		void* key;
		void* value;
		void* returnKey;
		void* compositeKey;
		void* compositeReturnKey;

	public:
		enum TransactionType {
			NONE = 0,
			SAVEPOINT = 1,
			STATEMENT = 2
		};

	public:
		DeepThreadContext();
		~DeepThreadContext();

		// XXX: inline for optimal performance (used internally)
		inline void* getTransactionInline();

		inline bool inReservedTransactionInline(DeepStore* datastore);
		inline unsigned long long getReservedKey(DeepStore* datastore);
		inline unsigned long long getReservedBlock(DeepStore* datastore);

		inline bool getOverrideInline(DeepStore* datastore);
		inline bool getUniqueChecksInline(DeepStore* datastore);
		inline bool inLoadDataTransactionInline(DeepStore* datastore);

		void setReserved(DeepStore* datastore, bool reserved);
		void setLoadData(DeepStore* datastore, bool loadData);
		void setOverride(DeepStore* datastore, bool override);
		void setUniqueChecks(DeepStore* datastore, bool uniqueChecks);

		short beginTransaction(TransactionType type = SAVEPOINT, int isolation = CT_DATASTORE_ISO_INHERIT);
		short prepareTransaction(short level = 0);
		short commitTransaction(short level = 0);
		short rollbackTransaction(short level = 0);
		short associateTransaction(DeepStore* datastore);
		short getTransactionLevel();
		TransactionType getTransactionType(short level);

		bool inTransaction(TransactionType type = SAVEPOINT);

		bool isCacheAllowed(DeepStore* datastore);

		inline int getLockCount() {
			return m_lockCount;
		}

		inline int getAndIncrementLockCount() {
			return m_lockCount++;
		}

		inline int decrementAndGetLockCount() {
			return --m_lockCount;
		}
};

class Condition {
	public:
		Condition() {}
		virtual ~Condition() {};

		virtual int check_condition(unsigned char* key) = 0;

};

class DeepCharset {
	public:
		DeepCharset() {}
		virtual ~DeepCharset() {}

		virtual int compare(int charset, char* key1, unsigned short length1, char* key2, unsigned short length2);

		virtual unsigned short populate(int charset, char* keyBuffer,
			const char* value, unsigned int valueLengthInBytes,
			unsigned short maxKeyLengthInBytes);

		virtual unsigned int getMultiByteMaxLen(int charset);

		virtual bool getMultiBytePack();
};

#define TABLE_NAME_MAX        256
#define AUTO_INCREMENT_OFFSET 256
#define TABLE_ROW_SIZE        TABLE_NAME_MAX + sizeof(unsigned long long)

} } } } // namespace

#endif //COM_DEEPIS_DATASTORE_API_DEEPISDATASTORE_H_
