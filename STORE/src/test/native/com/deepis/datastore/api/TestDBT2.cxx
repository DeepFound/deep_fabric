#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cxx/lang/System.h"
#include "cxx/lang/Thread.h"
#include "cxx/lang/Runnable.h"

#include "cxx/util/concurrent/atomic/AtomicInteger.h"

#include "com/deepis/datastore/api/DeepStore.h"

using namespace cxx::lang;
using namespace cxx::util::concurrent::atomic;
using namespace com::deepis::datastore::api;

static int CLIENTS = 10;
static int PUT_COUNT_PER_CLIENT = 1000000;
static int UPDATE_COUNT_PER_CLIENT = 1000000;

static int TOTAL = CLIENTS * PUT_COUNT_PER_CLIENT;

static int INT_SIZE   = sizeof(int);

static int DATA_SIZE = INT_SIZE + INT_SIZE + INT_SIZE + INT_SIZE;

static int CUSTOMER_KEY_SIZE   = INT_SIZE + INT_SIZE + INT_SIZE;
static int DISTRICT_KEY_SIZE   = INT_SIZE + INT_SIZE;

static int UPDATE_OFFSET = INT_SIZE + INT_SIZE + INT_SIZE;

static DeepStore* CUSTOMER;
static DeepStore* DISTRICT;

static AtomicInteger CLIENTS_RUNNING;

DeepThreadContext* CTX;

void testPut(DeepThreadContext* context, int clientNum);
int testRemove(DeepThreadContext* context, DeepStore* datastore, int clientNum);
int testUpdate(DeepThreadContext* context, DeepStore* datastore, int clientNum, int keyParts);

class UpdateClient : public Runnable {
	private:
		int m_clientNum;
		DeepThreadContext* m_ctx;

	public:
		UpdateClient(int num) :
			m_clientNum(num) {
			m_ctx = new DeepThreadContext();
		}

		virtual ~UpdateClient() {
			delete m_ctx;
		}

		virtual void run() {
			test();

			CLIENTS_RUNNING.getAndDecrement();
		}

		void test() {
			for (int i=0; i<UPDATE_COUNT_PER_CLIENT; i++) {
				if ((i % 100000) == 0) {
					printf("Client(%d) Update : %d\n", m_clientNum, i);
				}

				m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
				m_ctx->beginTransaction(DeepThreadContext::STATEMENT);
				m_ctx->associateTransaction(CUSTOMER);
				m_ctx->associateTransaction(DISTRICT);

				if (testUpdate(m_ctx, CUSTOMER, m_clientNum, 3) == 0) {
					if (testUpdate(m_ctx, DISTRICT, m_clientNum, 2) == 0) {
						m_ctx->commitTransaction();

					} else {
						m_ctx->rollbackTransaction();
					}

				} else {
					m_ctx->rollbackTransaction();
				}

				//Thread::sleep((rand() % 10) + 1);
			}
		}
};

class PutClient : public Runnable {
	private:
		int m_clientNum;

		DeepThreadContext* m_ctx;

	public:
		PutClient(int clientNum) :
			m_clientNum(clientNum) {
			m_ctx = new DeepThreadContext();
		}

		virtual ~PutClient() {
			delete m_ctx;
		}

		virtual void run() {
			testPut(m_ctx, m_clientNum);

			CLIENTS_RUNNING.getAndDecrement();
		}
};

class RemoveClient : public Runnable {
	private:
		int m_clientNum;

		DeepThreadContext* m_ctx;

	public:
		RemoveClient(int clientNum) :
			m_clientNum(clientNum) {
			m_ctx = new DeepThreadContext();
		}

		virtual ~RemoveClient() {
			delete m_ctx;
		}

		virtual void run() {
			for (int i=0; i<UPDATE_COUNT_PER_CLIENT; i++) {
				if ((i % 100000) == 0) {
					printf("Client(%d) Remove : %d\n", m_clientNum, i);
				}

				m_ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
				m_ctx->beginTransaction(DeepThreadContext::STATEMENT);
				m_ctx->associateTransaction(CUSTOMER);
				m_ctx->associateTransaction(DISTRICT);

				if (testRemove(m_ctx, CUSTOMER, m_clientNum) == 0) {
					if (testRemove(m_ctx, DISTRICT, m_clientNum) == 0) {
						m_ctx->commitTransaction();

					} else {
						m_ctx->rollbackTransaction();
					}

				} else {
					m_ctx->rollbackTransaction();
				}
			}

			CLIENTS_RUNNING.getAndDecrement();
		}
};

void startup(bool del) {

	//DeepStore::setDebugEnabled(true);

	//DeepStore::setFileSize(4294967295);
	DeepStore::setFileSize(536870912);
	DeepStore::setCacheSize(536870912);
	DeepStore::setTransactionChunk(10000);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	CUSTOMER = DeepStore::create("./customer.datastore", options, "PRIMARY", CT_DATASTORE_COMPOSITE, CUSTOMER_KEY_SIZE, DATA_SIZE);
	CUSTOMER->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, false);
	CUSTOMER->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 1, 0, 0, 0, 0, false);
	CUSTOMER->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 2, 0, 0, 0, 0, false);
	CUSTOMER->addKeyPart(
			0u,
			CT_DATASTORE_LONG_INT, // type
			INT_SIZE,              // size in key
			0,                     // offset in value
			0,                     // null offset in value
			0);                    // null bit
	CUSTOMER->addKeyPart(1u, CT_DATASTORE_LONG_INT, INT_SIZE, INT_SIZE, 0, 0);
	CUSTOMER->addKeyPart(2u, CT_DATASTORE_LONG_INT, INT_SIZE, INT_SIZE + INT_SIZE, 0, 0);
	CUSTOMER->initialize();

	DISTRICT = DeepStore::create("./district.datastore", options, "PRIMARY", CT_DATASTORE_COMPOSITE, DISTRICT_KEY_SIZE, DATA_SIZE);
	DISTRICT->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, false);
	DISTRICT->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 1, 0, 0, 0, 0, false);
	DISTRICT->addKeyPart(0u, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0);
	DISTRICT->addKeyPart(1u, CT_DATASTORE_LONG_INT, INT_SIZE, INT_SIZE, 0, 0);
	DISTRICT->initialize();

	CTX = new DeepThreadContext();

	CUSTOMER->open(CTX);
	CUSTOMER->recover(CTX, false);
	DISTRICT->open(CTX);
	DISTRICT->recover(CTX, false);
}

void shutdown() {

	CUSTOMER->close();
	DISTRICT->close();

	delete CUSTOMER;
	delete DISTRICT;
	delete CTX;
}

void testPut(DeepThreadContext* context, int clientNum) {
	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memset(value, 0, DATA_SIZE);

	longtype gstart = System::currentTimeMillis();
	longtype lstart = System::currentTimeMillis();

	int beginId = PUT_COUNT_PER_CLIENT * clientNum;
	int endId = beginId + PUT_COUNT_PER_CLIENT;

	for (int j = beginId; j < endId; j += 1000) {

		context->beginTransaction(DeepThreadContext::SAVEPOINT);
		context->associateTransaction(CUSTOMER);
		context->associateTransaction(DISTRICT);

		for (int i=0; i < 1000; i++) {
			int key = j + i;

			for (int k=0; (k*INT_SIZE)<DATA_SIZE; k++) {
				memcpy(value + (k*INT_SIZE), &key, INT_SIZE);
			}

			if (CUSTOMER->put(context, value, DATA_SIZE) != 0) {
				printf("Client(%d) FAILED - Put CUSTOMER\n", clientNum);
				exit(1);
			}

			if (DISTRICT->put(context, value, DATA_SIZE) != 0) {
				printf("Client(%d) FAILED - Put DISTRICT\n", clientNum);
				exit(1);
			}

			if ((key % 100000) == 0) {
				longtype lstop = System::currentTimeMillis();
				printf("Client(%d) Put %d --  %lld\n", clientNum, key, (lstop-lstart));
				lstart = System::currentTimeMillis();
			}
		}

		context->commitTransaction();
	}

	free(value);

	longtype gstop = System::currentTimeMillis();

	printf("Client(%d) PUT TIME: %d, %lld\n", clientNum, PUT_COUNT_PER_CLIENT, (gstop-gstart));
}

int testUpdate(DeepThreadContext* context, DeepStore* datastore, int clientNum, int keyParts) {

	// update keys from 1-100
	int randkey = (rand() % 100) + 1;

	unsigned char* key = (unsigned char*) malloc(keyParts*INT_SIZE);
	memset(key, 0, keyParts*INT_SIZE);

	for (int i=0; i<keyParts; i++) {
		memcpy(key + (i*INT_SIZE), &randkey, INT_SIZE);
	}

	unsigned char* retkey = (unsigned char*) malloc(CUSTOMER_KEY_SIZE);

	unsigned char* oldvalue = (unsigned char*) malloc(DATA_SIZE);

	unsigned int length = DATA_SIZE;

	if (datastore->get(context, key, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE) != CT_DATASTORE_SUCCESS) {
		int errorCode = datastore->getErrorCode();
		if (errorCode != CT_DATASTORE_ERROR_DEADLOCK) {
			printf("\nFAILED - Client(%d) get failed (not a deadlock): %d\n", clientNum, errorCode);
			exit(1);

		} else {
			printf("\nDEADLOCK - abort\n");
			free(oldvalue);
			free(key);
			free(retkey);
			return 1;
		}
	} else {
		//printf("\nSUCCESS - get: %d\n", randkey);
	}

	unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);

	int rupdate = (rand() % 1000);

	memcpy(newvalue, oldvalue, DATA_SIZE);
	memcpy(newvalue + UPDATE_OFFSET, &rupdate, INT_SIZE);

	if (datastore->update(context, oldvalue, DATA_SIZE, newvalue, DATA_SIZE, false) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Client(%d) Update (update)\n", clientNum);
		exit(1);
	}

	//printf("\nSUCCESS - update: %d\n", randkey);

	free(oldvalue);
	free(newvalue);
	free(key);
	free(retkey);

	return 0;
}

int testRemove(DeepThreadContext* context, DeepStore* datastore, int clientNum) {

	int randkey = (rand() % TOTAL) + 1;

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	unsigned int length = DATA_SIZE;

	for (int k=0; (k*INT_SIZE)<DATA_SIZE; k++) {
		memcpy(value + (k*INT_SIZE), &randkey, INT_SIZE);
	}

	int status = datastore->remove(context, value, length);

	if (status != CT_DATASTORE_SUCCESS) {
		printf("ABORT - Client(%d) remove failed  %d\n", clientNum, randkey);
	}

	free(value);

	return status;
}

int main(int argc, char** argv) {

	srand(System::currentTimeMillis());

	startup(true);

	//
	// Put
	//

	PutClient** putClients = new PutClient*[CLIENTS];
	Thread** threads = new Thread*[CLIENTS];

	printf("%d PUT CLIENTs\n", CLIENTS);

	for (int i = 0; i < CLIENTS; i++) {
		CLIENTS_RUNNING.getAndIncrement();

		Thread::sleep(100);

		putClients[i] = new PutClient(i);
		threads[i] = new Thread(putClients[i]);
		threads[i]->start();
	}

	do {
		printf("    Waiting for %d put client(s)...\n", CLIENTS_RUNNING.get());

		Thread::sleep(1000);

	} while (CLIENTS_RUNNING.get() > 0);

	for (int i = 0; i < CLIENTS; i++) {
		delete putClients[i];
		delete threads[i];
	}

	delete [] putClients;

/*
	//
	// Update
	//
	UpdateClient* clients[CLIENTS];

	printf("%d UPDATE CLIENTs\n", CLIENTS);

	for (int i = 0; i < CLIENTS; i++) {
		CLIENTS_RUNNING.getAndIncrement();

		clients[i] = new UpdateClient(i);
		threads[i] = new Thread(clients[i]);
		threads[i]->start();
	}

	do {
		printf("    Waiting for %d update client(s)...\n", CLIENTS_RUNNING.get());

		Thread::sleep(1000);

	} while (CLIENTS_RUNNING.get() > 0);

	for (int i = 0; i < CLIENTS; i++) {
		delete clients[i];
		delete threads[i];
	}
*/

	//
	// Remove
	//
	RemoveClient** clients = new RemoveClient*[CLIENTS];

	printf("%d REMOVE CLIENTs\n", CLIENTS);

	srand(time(0));

	for (int i = 0; i < CLIENTS; i++) {
		CLIENTS_RUNNING.getAndIncrement();

		clients[i] = new RemoveClient(i);
		threads[i] = new Thread(clients[i]);
		threads[i]->start();
	}

	do {
		printf("    Waiting for %d remove client(s)...\n", CLIENTS_RUNNING.get());

		Thread::sleep(1000);

	} while (CLIENTS_RUNNING.get() > 0);

	for (int i = 0; i < CLIENTS; i++) {
		delete clients[i];
		delete threads[i];
	}

	delete [] clients;
	delete [] threads;

	shutdown();

	return 0;
}
