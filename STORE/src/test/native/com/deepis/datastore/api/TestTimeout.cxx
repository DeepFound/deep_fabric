#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cxx/lang/Thread.h"
#include "cxx/lang/System.h"
#include "cxx/lang/Runnable.h"

//#include "cxx/util/Logger.h"
#include "cxx/util/concurrent/atomic/AtomicInteger.h"

#include "com/deepis/datastore/api/DeepStore.h"

using namespace cxx::lang;
using namespace cxx::util;
using namespace cxx::util::concurrent::atomic;
using namespace com::deepis::datastore::api;

static AtomicInteger CLIENTS_RUNNING;

static int LOCK_TIMEOUT = 5; // secs
static int LOCK_HOLD = ((LOCK_TIMEOUT * 4) + 2);

static int DATA_SIZE = (3 * sizeof(int));

static DeepStore* PRIMARY;
static DeepStore* SECONDARY;

DeepThreadContext* CTX;

unsigned char* KEY;

void startup(bool del);
void shutdown();
void setup();
int getErrorCode();

class ReadLockHolder : public Runnable {
	private:
		DeepStore* m_datastore;
		DeepThreadContext* m_ctx;

	public:
		ReadLockHolder() {
			m_ctx = new DeepThreadContext();
		}

		virtual ~ReadLockHolder() {
			delete m_ctx;
		}

		void setDatastore(DeepStore* datastore) {
			m_datastore = datastore;
		}

		virtual void run() {
			test();

			CLIENTS_RUNNING.getAndDecrement();
		}

		const char* getDatastoreName() {
			return (m_datastore == PRIMARY) ? "Reader(PRIMARY)" : "Reader(SECONDARY)";
		}

		void test() {
			int k = 1;
			unsigned char key[sizeof(int) + sizeof(int)];
			memcpy(key, &k, sizeof(int));
			memcpy(key + sizeof(int), &k, sizeof(int));  // ignored if datastore is PRIMARY

			unsigned char retkey[sizeof(int) + sizeof(int)];
			memset(retkey, 1, sizeof(int) + sizeof(int));

			unsigned char* retvalue = (unsigned char*) malloc(DATA_SIZE);
			memset(retvalue, 1, DATA_SIZE);

			unsigned int length = DATA_SIZE;

			// XXX: always begin transaction and set read lock on primary
			m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
			m_ctx->associateTransaction(PRIMARY);

			if (m_datastore->get(m_ctx, key, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE /*lock*/) != CT_DATASTORE_SUCCESS) {
				printf("\nFAILED - %s get\n", getDatastoreName());
				exit(1);
			}

			if (m_datastore == PRIMARY) {
				printf("\n%s - Acquired read lock - key is %d, holding for %d seconds\n",
						getDatastoreName(), *((int*)retkey), LOCK_HOLD);

			} else {
				printf("\n%s - Acquired read lock - key is %d.%d, holding for %d seconds\n",
						getDatastoreName(), *((int*)retkey), *((int*)(retkey + sizeof(int))), LOCK_HOLD);
			}
			//printf("%s - value is %d.%d.%d\n", getDatastoreName(), *((int*)retvalue),
			//		*((int*)(retvalue + sizeof(int))), *((int*)(retvalue + sizeof(int) + sizeof(int))));

			Thread::sleep(LOCK_HOLD * 1000);

			m_ctx->commitTransaction();

			printf("\n%s - Lock released...\n", getDatastoreName());

			free(retvalue);
		}
};

class Writer : public Runnable {
	private:
		bool m_isTx;
		DeepThreadContext* m_ctx;

	public:
		Writer(bool isTx) {
			m_isTx = isTx;
			m_ctx = new DeepThreadContext();
		}

		virtual ~Writer() {
			delete m_ctx;
		}

		virtual void run() {
			test();

			CLIENTS_RUNNING.getAndDecrement();
		}

		void test() {
			int k = 1;
			int m = 2;

			unsigned char key[sizeof(int)];
			memcpy(key, &k, sizeof(int));

			unsigned char retkey[sizeof(int)];

			unsigned char* oldvalue = (unsigned char*) malloc(DATA_SIZE);
			memset(oldvalue, 0, DATA_SIZE);
			memcpy(oldvalue, &k, sizeof(int));
			memcpy(oldvalue + sizeof(int), &k, sizeof(int));
			memcpy(oldvalue + sizeof(int) + sizeof(int), &k, sizeof(int));

			unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
			memset(newvalue, 0, DATA_SIZE);
			memcpy(newvalue, &k, sizeof(int));
			memcpy(newvalue + sizeof(int), &k, sizeof(int));
			memcpy(newvalue + sizeof(int) + sizeof(int), &m, sizeof(int));

			printf("\nWriter - Attempting UPDATE...\n");

			if (m_isTx == true) {
				m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
				m_ctx->associateTransaction(PRIMARY);
			}

			long long stime = System::currentTimeMillis();

			int status = PRIMARY->update(m_ctx, oldvalue, DATA_SIZE, newvalue, DATA_SIZE);

			if (status != CT_DATASTORE_SUCCESS) {
				status = getErrorCode();
				if (status == CT_DATASTORE_ERROR_LOCK_TIMEOUT) {
					long long durationSeconds = (System::currentTimeMillis() - stime) / 1000;
					if (durationSeconds == LOCK_TIMEOUT) {
						printf("\nOK - UPDATE timed out in %lld seconds\n", durationSeconds);

					} else {
						printf("\nWARN - UPDATE timed out in %lld seconds, expected %d seconds\n", durationSeconds, LOCK_TIMEOUT);
					}

					if (m_isTx == true) {
						m_ctx->rollbackTransaction();
					}

				} else {
					printf("\nFAILED - UPDATE failed with error %d, should have timed out\n", status);
					exit(1);
				}

			} else {
				printf("\nFAILED - UPDATE succeeded but should have timed out\n");
				exit(1);
			}

			printf("\nWriter - Attempting REMOVE...\n");

			if (m_isTx == true) {
				m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
				m_ctx->associateTransaction(PRIMARY);
			}

			stime = System::currentTimeMillis();

			status = PRIMARY->remove(m_ctx, oldvalue, DATA_SIZE);

			if (status != CT_DATASTORE_SUCCESS) {
				status = getErrorCode();
				if (status == CT_DATASTORE_ERROR_LOCK_TIMEOUT) {
					long long durationSeconds = (System::currentTimeMillis() - stime) / 1000;
					if (durationSeconds == LOCK_TIMEOUT) {
						printf("\nOK - REMOVE timed out in %lld seconds\n", durationSeconds);

					} else {
						printf("\nWARN - REMOVE timed out in %lld seconds, expected %d seconds\n", durationSeconds, LOCK_TIMEOUT);
					}

					if (m_isTx == true) {
						m_ctx->rollbackTransaction();
					}

				} else {
					printf("\nFAILED - REMOVE failed with error %d, should have timed out\n", status);
					exit(1);
				}

			} else {
				printf("\nFAILED - REMOVE succeeded but should have timed out\n");
				exit(1);
			}

			if (m_isTx == true) {
				printf("\nWriter - Attempting PRIMARY GET with read lock...\n");

				m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
				m_ctx->associateTransaction(PRIMARY);

				stime = System::currentTimeMillis();

				unsigned int length = DATA_SIZE;

				status = PRIMARY->get(m_ctx, key, (unsigned char**) &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE /*lock*/ );

				if (status != CT_DATASTORE_SUCCESS) {
					status = getErrorCode();
					if (status == CT_DATASTORE_ERROR_LOCK_TIMEOUT) {
						long long durationSeconds = (System::currentTimeMillis() - stime) / 1000;
						if (durationSeconds == LOCK_TIMEOUT) {
							printf("\nOK - PRIMARY GET timed out in %lld seconds\n", durationSeconds);

						} else {
							printf("\nWARN - PRIMARY GET timed out in %lld seconds, expected %d seconds\n", durationSeconds, LOCK_TIMEOUT);
						}

						m_ctx->rollbackTransaction();

					} else {
						printf("\nFAILED - PRIMARY GET failed with error %d, should have timed out\n", status);
						exit(1);
					}

				} else {
					printf("\nFAILED - PRIMARY GET succeeded but should have timed out\n");
					exit(1);
				}

				unsigned char skey[sizeof(int) + sizeof(int)];
				memcpy(skey, &k, sizeof(int));
				memcpy(skey + sizeof(int), &k, sizeof(int));

				unsigned char sretkey[sizeof(int) + sizeof(int)];
				memset(sretkey, 1, sizeof(int) + sizeof(int));

				unsigned char* retvalue = (unsigned char*) malloc(DATA_SIZE);
				memset(retvalue, 1, DATA_SIZE);

				printf("\nWriter - Attempting SECONDARY GET with read lock...\n");

				m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
				m_ctx->associateTransaction(PRIMARY);

				stime = System::currentTimeMillis();

				status = SECONDARY->get(m_ctx, skey, &retvalue, &length, sretkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE /*lock*/ );
				if (status != CT_DATASTORE_SUCCESS) {
					status = getErrorCode();
					if (status == CT_DATASTORE_ERROR_LOCK_TIMEOUT) {
						long long durationSeconds = (System::currentTimeMillis() - stime) / 1000;
						if (durationSeconds == LOCK_TIMEOUT) {
							printf("\nOK - SECONDARY GET with LOCK timed out in %lld seconds\n", durationSeconds);

						} else {
							printf("\nWARN - SECONDARY GET with LOCK timed out in %lld seconds, expected %d seconds\n", durationSeconds, LOCK_TIMEOUT);
						}

						m_ctx->rollbackTransaction();

					} else {
						printf("\nFAILED - SECONDARY GET with LOCK failed with error %d, should have timed out\n", status);
						exit(1);
					}

				} else {
					printf("\nFAILED - SECONDARY GET with LOCK succeeded but should have timed out\n");
					exit(1);
				}

				free(retvalue);
			}

			free(oldvalue);
			free(newvalue);
		}
};

int getErrorCode() {
	int status = PRIMARY->getErrorCode();

	if (status == CT_DATASTORE_SUCCESS) {
		status = SECONDARY->getErrorCode();
	}

	return status;
}

void testTimeout(DeepStore* datastore, bool isTx) {
	CLIENTS_RUNNING.getAndIncrement();

	ReadLockHolder lockHolder;
	Thread lockHolderThread(&lockHolder);

	lockHolder.setDatastore(datastore);
	lockHolderThread.start();

	Thread::sleep(1000);

	CLIENTS_RUNNING.getAndIncrement();

	Writer writer(isTx);
	Thread writerThread(&writer);
	writerThread.start();

	Thread::sleep(1000);

	do {
		Thread::sleep(1000);

	} while (CLIENTS_RUNNING.get() > 0);

	printf("\nSUCCESS\n");
}

int main(int argc, char** argv) {

	startup(true);

	setup();

	printf("\n\nTESTING PRIMARY READ LOCK with TX WRITER\n");

	testTimeout(PRIMARY, true);

	printf("\n\nTESTING SECONDARY READ LOCK with TX WRITER\n");

	testTimeout(SECONDARY, true);

	//printf("\n\nTESTING PRIMARY READ LOCK with NON-TX WRITER\n");

	//testTimeout(PRIMARY, false);

	//printf("\n\nTESTING SECONDARY READ LOCK with NON-TX WRITER\n");

	//testTimeout(SECONDARY, false);

	shutdown();

	return 0;
}

void startup(bool del) {

	//DeepStore::setDebugEnabled(true);

	DeepStore::setFileSize(4294967295);
	DeepStore::setCacheSize(1073741824);
	DeepStore::setTransactionChunk(10000);
	DeepStore::setLockTimeout(LOCK_TIMEOUT);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	PRIMARY = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);
	PRIMARY->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	PRIMARY->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
	PRIMARY->initialize();

	SECONDARY = DeepStore::create("./s.datastore", options, "SECONDARY", CT_DATASTORE_COMPOSITE, sizeof(int) + sizeof(int), DATA_SIZE);
	SECONDARY->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	SECONDARY->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	SECONDARY->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), sizeof(int), 0, 0);
	SECONDARY->addKeyPart(1u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);  // primary key suffix
	SECONDARY->initialize();

	PRIMARY->associate(SECONDARY, true);

	CTX = new DeepThreadContext();

	PRIMARY->open(CTX);
	SECONDARY->open(CTX);
	PRIMARY->recover(CTX, false);
}

void shutdown() {

	PRIMARY->close();
	SECONDARY->close();

	delete SECONDARY;
	delete PRIMARY;

	delete CTX;
}

void setup() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	for (int k = 1; k < 5; k++) {
		unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
		memset(value, 0, DATA_SIZE);
		memcpy(value, &k, sizeof(int));
		memcpy(value + sizeof(int), &k, sizeof(int));
		memcpy(value + sizeof(int) + sizeof(int), &k, sizeof(int));

		PRIMARY->put(CTX, value, DATA_SIZE);

		free(value);
	}

	CTX->commitTransaction();
}
