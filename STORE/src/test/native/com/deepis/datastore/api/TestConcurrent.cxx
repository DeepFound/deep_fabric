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

static int NUM_THREADS = 32;

static AtomicInteger CLIENTS_RUNNING;

static int COUNT = 1000;
static int DATA_SIZE = (2 * sizeof(int));

static DeepStore* PRIMARY1;
static DeepStore* PRIMARY2;

DeepThreadContext* CTX;

unsigned char KEY[sizeof(int)];

void startup(bool del);
void shutdown();
void setup();

class TestThread : public Runnable {
	private:
		int m_clientNum;
		DeepThreadContext* m_ctx;

	public:
		TestThread(int num) :
			m_clientNum(num) {
			m_ctx = new DeepThreadContext();
		}

		virtual ~TestThread() {
			delete m_ctx;
		}

		virtual void run() {
			test();

			CLIENTS_RUNNING.getAndDecrement();
		}

		void test() {
			unsigned int length = DATA_SIZE;
			unsigned char* retvalue = (unsigned char*) malloc(DATA_SIZE);
			memset(retvalue, 1, DATA_SIZE);

			for (int i = 1; i <= COUNT; i++) {
				unsigned char retkey[sizeof(int)];

				// working across two separate datastores (tables)
				m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);

				m_ctx->associateTransaction(PRIMARY1);
				m_ctx->associateTransaction(PRIMARY2);

				if (PRIMARY1->get(m_ctx, KEY, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE /*lock*/) != CT_DATASTORE_SUCCESS) {
					printf("FAILED - CLIENT(%d) Primary get %d - error code: %d\n", m_clientNum, i, PRIMARY1->getErrorCode());
					exit(1);
				}

				int nextKey = *((int*) (retvalue + sizeof(int)));
				int newNextKey = nextKey + 1;

				//printf("CLIENT(%d) Attempt insert of next key : %d\n", m_clientNum, nextKey);

				unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
				memcpy(newvalue, retvalue, DATA_SIZE);
				memcpy(newvalue + sizeof(int), &newNextKey, sizeof(int));

				if (PRIMARY1->update(m_ctx, retvalue, newvalue, DATA_SIZE) != CT_DATASTORE_SUCCESS) {
					printf("FAILED - CLIENT(%d) Primary update %d - error code: %d\n", m_clientNum, nextKey, PRIMARY1->getErrorCode());
					exit(1);
				}

				unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
				memset(value, 0, DATA_SIZE);
				memcpy(value, &nextKey, sizeof(int));
				memcpy(value + sizeof(int), &nextKey, sizeof(int));

				//printf("CLIENT(%d) Putting : %d\n", m_clientNum, nextKey);

				if (PRIMARY2->put(m_ctx, value, DATA_SIZE) != CT_DATASTORE_SUCCESS) {
					printf("FAILED - CLIENT(%d) put %d - error code: %d\n", m_clientNum, nextKey, PRIMARY2->getErrorCode());
					exit(1);
				}

				// commit work on both datastores
				m_ctx->commitTransaction();

				free(newvalue);
				free(value);

				Thread::sleep((rand() % 10) + 1);
			}

			free(retvalue);
		}
};

int main(int argc, char** argv) {

	TestThread** clients = new TestThread*[NUM_THREADS];
	Thread** threads = new Thread*[NUM_THREADS];

	startup(true);

	setup();

	printf("%d CLIENTs\n", NUM_THREADS);

	srand(System::currentTimeMillis());

	for (int i = 0; i < NUM_THREADS; i++) {
		CLIENTS_RUNNING.getAndIncrement();

		clients[i] = new TestThread(i);
		threads[i] = new Thread(clients[i]);
		threads[i]->start();
	}

	do {
		printf("    Waiting for %d client(s)...\n", CLIENTS_RUNNING.get());

		Thread::sleep(1000);

	} while (CLIENTS_RUNNING.get() > 0);

	for (int i = 0; i < NUM_THREADS; i++) {
		delete clients[i];
		delete threads[i];
	}

	delete [] clients;
	delete [] threads;

	shutdown();

	return 0;
}

void startup(bool del) {

	//DeepStore::setDebugEnabled(true);

	DeepStore::setFileSize(4294967295);
	DeepStore::setCacheSize(1073741824);
	DeepStore::setTransactionChunk(10000);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	PRIMARY1 = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);
	PRIMARY1->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY1->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
	PRIMARY1->initialize();

	PRIMARY2 = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);
	PRIMARY2->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY2->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
	PRIMARY2->initialize();

	CTX = new DeepThreadContext();

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

	int k = 1;
	int j = 1;

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memset(value, 0, DATA_SIZE);
	memcpy(value, &k, sizeof(int));
	memcpy(value + sizeof(int), &j, sizeof(int));

	printf("Put primary key: %d, value: %d %d\n", k, k, j);

	PRIMARY1->put(CTX, value, DATA_SIZE);

	memcpy(KEY, &k, sizeof(int));

	CTX->commitTransaction();

	free(value);
}
