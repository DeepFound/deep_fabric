#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cxx/lang/Thread.h"
#include "cxx/lang/System.h"
#include "cxx/lang/Runnable.h"

//#include "cxx/util/Logger.h"
#include "cxx/util/concurrent/atomic/AtomicInteger.h"

#ifdef DEEP_DISTRIBUTED
#include "com/deepis/db/store/relative/distributed/MapBehavior.h"
using namespace com::deepis::db::store::relative::distributed;
#endif
#include "com/deepis/datastore/api/DeepStore.h"

using namespace cxx::lang;
using namespace cxx::util;
using namespace cxx::util::concurrent::atomic;
using namespace com::deepis::datastore::api;

static int NUM_THREADS = 1;

static AtomicInteger CLIENTS_RUNNING;

static int COUNT = 1000;
static ulongtype BLOCK = 1000;
static int DATA_SIZE = sizeof(int);

static DeepStore* PRIMARY1;

DeepThreadContext* CTX;

class TestThread : public Runnable {
	private:
		int m_num;
		DeepThreadContext* m_ctx;

	public:
		TestThread(int num) :
			m_num(num) {
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
			ulongtype first = 0;
			ulongtype reserved = 0;
			for (int i = 1; i <= COUNT; i++) {

				m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
				m_ctx->associateTransaction(PRIMARY1);

				PRIMARY1->getReservedKeyBlock(0, BLOCK, &first, &reserved, m_ctx);
				if (reserved != BLOCK) {
					printf("CLIENT(%d) GET KEY BLOCK FAILED\n", m_num);
					exit(1);
				}

				if ((i % 100) == 0) {
					printf("CLIENT(%d) RESERVED BLOCK %llu - %llu\n", m_num, first, first + reserved -1);
				}

				m_ctx->setReserved(PRIMARY1, true);

				for(int key = first; key < ((int) (first + reserved)); key++) {
					unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
					memcpy(newvalue, &key, sizeof(int));

					if (PRIMARY1->put(m_ctx, newvalue, DATA_SIZE) != CT_DATASTORE_SUCCESS) {
						printf("CLIENT(%d) PUT FAILED\n", m_num);
						exit(1);
					}

					free(newvalue);
				}

				//printf("CLIENT(%d) COMMITTING\n", m_num);

				m_ctx->commitTransaction();

				Thread::sleep((rand() % 10) + 1);
			}
		}
};

void startup(bool del) {

	//DeepStore::setDebugEnabled(true);

	DeepStore::setFileSize(4294967295);
	DeepStore::setCacheSize(1073741824);
	DeepStore::setTransactionChunk(10000);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	#ifdef DEEP_DISTRIBUTED
	PRIMARY1 = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, DISTRIBUTED_STANDALONE, true);
	#else
	PRIMARY1 = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, true);
	#endif
	PRIMARY1->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY1->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
	PRIMARY1->initialize();

	CTX = new DeepThreadContext();

	PRIMARY1->open(CTX);
	PRIMARY1->recover(CTX, false);
}

void shutdown() {

	PRIMARY1->close();

	delete PRIMARY1;
	delete CTX;
}

int main(int argc, char** argv) {

	TestThread** clients = new TestThread*[NUM_THREADS];
	Thread** threads = new Thread*[NUM_THREADS];

	startup(true);

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
