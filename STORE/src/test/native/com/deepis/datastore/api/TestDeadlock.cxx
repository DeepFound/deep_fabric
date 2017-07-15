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

static const int DATA_SIZE = (2 * sizeof(int));

static DeepStore* PRIMARY1;
static DeepStore* PRIMARY2;

DeepThreadContext* CTX;

class Client : public Runnable {
	private:
		DeepStore* m_datastore1;
		DeepStore* m_datastore2;

		DeepThreadContext* m_ctx;

		int m_num;
		unsigned char key1[sizeof(int)];
		unsigned char key2[sizeof(int)];
		unsigned char retkey[sizeof(int)];
		unsigned char* retvalue;

	public:
		Client(int num, DeepStore* datastore1, DeepStore* datastore2) :
			m_datastore1(datastore1),
			m_datastore2(datastore2),
			m_num(num) {

			m_ctx = new DeepThreadContext();

			int k1 = 1;
			int k2 = 2;
			memcpy(key1, &k1, sizeof(int));
			memcpy(key2, &k2, sizeof(int));

			retvalue = (unsigned char*) malloc(DATA_SIZE);
			memset(retvalue, 1, DATA_SIZE);
		}

		virtual ~Client() {
			free(retvalue);

			delete m_ctx;
		}

		virtual void run() {
			CLIENTS_RUNNING.getAndIncrement();

			if (m_num % 2) {
				testSuccess();

			} else {
				testFailure();
			}

			CLIENTS_RUNNING.getAndDecrement();
		}

		void testSuccess() {

			unsigned int length = DATA_SIZE;

			m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);

			m_ctx->associateTransaction(m_datastore1);

			printf("\nClient(%d) - Acquiring read lock on KEY1\n", m_num);

			if (m_datastore1->get(m_ctx, key1, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE /*lock*/ ) != 0) {
				printf("\nFAILED - Client(%d) get KEY1\n", m_num);
				exit(1);
			}

			Thread::sleep(2000);

			m_ctx->associateTransaction(m_datastore2);

			printf("\nClient(%d) - Acquiring read lock on KEY2\n", m_num);

			if (m_datastore2->get(m_ctx, key2, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE /*lock*/ ) != 0) {
				printf("\nWARN - Client(%d) get KEY2 failed, other client should have caused deadlock\n", m_num);
				m_ctx->rollbackTransaction();

			} else {
				m_ctx->commitTransaction();
			}
		}

		void testFailure() {

			unsigned int length = DATA_SIZE;

			m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);

			m_ctx->associateTransaction(m_datastore2);

			printf("\nClient(%d) - Acquiring read lock on KEY2\n", m_num);

			if (m_datastore2->get(m_ctx, key2, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE /*lock*/) != 0) {
				printf("\nFAILED - Client(%d) get KEY2\n", m_num);
				exit(1);
			}

			Thread::sleep(2000);

			printf("\nClient(%d) - Acquiring read lock on KEY1\n", m_num);

			m_ctx->associateTransaction(m_datastore1);

			if (m_datastore1->get(m_ctx, key1, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE /*lock*/) != 0) {
				int errorCode = m_datastore1->getErrorCode();
				if (errorCode != CT_DATASTORE_ERROR_DEADLOCK) {
					printf("\nFAILED - Client(%d) Expected deadlock error, got %d instead\n", m_num, errorCode);
					exit(1);

				} else {
					printf("\nSUCCESS - Client(%d) received deadlock error\n", m_num);
				}

			} else {
				printf("\nFAILED - Client(%d) get succeeded on KEY2, should have caused deadlock\n", m_num);
				exit(1);
			}

			m_ctx->rollbackTransaction();
		}
};

void testDeadlock(DeepStore* datastore1, DeepStore* datastore2) {
	Client lockHolder1(1, datastore1, datastore2);
	Client lockHolder2(2, datastore1, datastore2);

	Thread lockHolderThread1(&lockHolder1);
	Thread lockHolderThread2(&lockHolder2);

	lockHolderThread1.start();

	Thread::sleep(1000);

	lockHolderThread2.start();

	do {
		Thread::sleep(1000);

	} while (CLIENTS_RUNNING.get() > 0);
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

	CTX = new DeepThreadContext();

	PRIMARY1 = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);
	PRIMARY1->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY1->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
	PRIMARY1->initialize();

	PRIMARY2 = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);
	PRIMARY2->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY2->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
	PRIMARY2->initialize();

	PRIMARY1->open(CTX);
	PRIMARY1->recover(CTX, false);
	PRIMARY2->open(CTX);
	PRIMARY2->recover(CTX, false);
}

void shutdown() {

	PRIMARY1->close();
	PRIMARY2->close();

	delete PRIMARY2;
	delete PRIMARY1;
	delete CTX;
}

void setup() {

	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY1);
	CTX->associateTransaction(PRIMARY2);

	int k1 = 1;

	unsigned char value1[DATA_SIZE];
	memset(value1, 0, DATA_SIZE);
	memcpy(value1, &k1, sizeof(int));
	memcpy(value1 + sizeof(int), &k1, sizeof(int));

	PRIMARY1->put(CTX, value1, DATA_SIZE);
	PRIMARY2->put(CTX, value1, DATA_SIZE);

	int k2 = 2;

	unsigned char value2[DATA_SIZE];
	memset(value2, 0, DATA_SIZE);
	memcpy(value2, &k2, sizeof(int));
	memcpy(value2 + sizeof(int), &k2, sizeof(int));

	PRIMARY1->put(CTX, value2, DATA_SIZE);
	PRIMARY2->put(CTX, value2, DATA_SIZE);

	CTX->commitTransaction();
}

int main(int argc, char** argv) {

	startup(true);

	setup();

	printf("\nTEST DEADLOCK WITHIN SAME MAP\n");

	testDeadlock(PRIMARY1, PRIMARY1);

	printf("\nTEST DEADLOCK ACROSS DIFFERENT MAPS\n");

	testDeadlock(PRIMARY1, PRIMARY2);

	shutdown();

	return 0;
}
