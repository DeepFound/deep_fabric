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
static int GET_COUNT_PER_CLIENT = 1000000;

static int INT_SIZE   = sizeof(int);
static int ULONG_SIZE = sizeof(unsigned long long);
static int FLOAT_SIZE = sizeof(float);

static int DATA_SIZE =
		INT_SIZE +    // 'transactionid int not null auto_increment, '
		ULONG_SIZE +  // 'dateandtime datetime, '
		INT_SIZE +    // 'cashregisterid int not null, '
		INT_SIZE +    // 'customerid int not null, '
		INT_SIZE +    // 'productid int not null, '
		FLOAT_SIZE;   // 'price float not null, '

static int PRIMARY_KEY_SIZE   = INT_SIZE;
static int MARKET_KEY_SIZE    = FLOAT_SIZE + INT_SIZE + INT_SIZE;
static int REGISTER_KEY_SIZE  = INT_SIZE + FLOAT_SIZE + INT_SIZE + INT_SIZE;
static int PDC_KEY_SIZE       = FLOAT_SIZE + ULONG_SIZE + INT_SIZE + INT_SIZE;

static int PRICE_OFFSET            = INT_SIZE + ULONG_SIZE + INT_SIZE + INT_SIZE + INT_SIZE;
static int CUSTOMER_ID_OFFSET      = INT_SIZE + ULONG_SIZE + INT_SIZE;
static int CASH_REGISTER_ID_OFFSET = INT_SIZE + ULONG_SIZE;
static int TIME_OFFSET             = INT_SIZE;

static DeepStore* PRIMARY;
static DeepStore* MARKET;
static DeepStore* REGISTER;
static DeepStore* PDC;

DeepThreadContext* CTX;

static AtomicInteger CLIENTS_RUNNING;

void testGet(DeepThreadContext* context, int clientNum, int seed);
void testPut(DeepThreadContext* context, int clientNum);

class GetClient : public Runnable {
	private:
		int m_clientNum;
		DeepThreadContext* m_ctx;

	public:
		GetClient(int num) :
			m_clientNum(num) {
			m_ctx = new DeepThreadContext();
		}

		virtual ~GetClient() {
			delete m_ctx;
		}

		virtual void run() {
			test();

			CLIENTS_RUNNING.getAndDecrement();
		}

		void test() {
			longtype lstart = System::currentTimeMillis();

			for (int i=0; i<GET_COUNT_PER_CLIENT; i++) {
				if ((i % 100000) == 0) {
					longtype lstop = System::currentTimeMillis();
					printf("Client(%d) Get : %d -- %lld\n", m_clientNum, i, (lstop-lstart));
					lstart = System::currentTimeMillis();
				}

				testGet(m_ctx, m_clientNum, i);

				//Thread::sleep((rand() % 10) + 1);
			}
		}
};

class PutClient : public Runnable {
	private:
		int m_clientNum;
		DeepThreadContext* m_ctx;

	public:
		PutClient(int num) :
			m_clientNum(num) {
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

	PRIMARY = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, PRIMARY_KEY_SIZE, DATA_SIZE);
	 // transaction id
	PRIMARY->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY->addKeyPart(
			0u,
			CT_DATASTORE_LONG_INT, // type
			INT_SIZE,              // size in key
			0,                     // offset in value
			0,                     // null offset in value
			0);                    // null bit
	PRIMARY->initialize();

	MARKET = DeepStore::create("./market.datastore", options, "MARKET", CT_DATASTORE_COMPOSITE, MARKET_KEY_SIZE, DATA_SIZE);
	MARKET->addField(CT_DATASTORE_FLOAT, CT_DATASTORE_FLOAT, FLOAT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, false);
	MARKET->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 1, 0, 0, 0, 0, false);
	MARKET->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 2, 0, 0, 0, 0, false);
	MARKET->addKeyPart(0u, CT_DATASTORE_FLOAT, FLOAT_SIZE, PRICE_OFFSET, 0, 0);               // price
	MARKET->addKeyPart(1u, CT_DATASTORE_LONG_INT, INT_SIZE, CUSTOMER_ID_OFFSET, 0, 0); // customer id
	MARKET->addKeyPart(2u, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0);       // PRIMARY SUFFIX transaction id
	MARKET->initialize();

	REGISTER = DeepStore::create("./register.datastore", options, "REGISTER", CT_DATASTORE_COMPOSITE, REGISTER_KEY_SIZE, DATA_SIZE);
	REGISTER->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, false);
	REGISTER->addField(CT_DATASTORE_FLOAT, CT_DATASTORE_FLOAT, FLOAT_SIZE, 0, 0, 0, 1, 0, 0, 0, 0, false);
	REGISTER->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 2, 0, 0, 0, 0, false);
	REGISTER->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 3, 0, 0, 0, 0, false);
	REGISTER->addKeyPart(0u, CT_DATASTORE_LONG_INT, INT_SIZE, CASH_REGISTER_ID_OFFSET, 0, 0);                // cash register id
	REGISTER->addKeyPart(1u, CT_DATASTORE_FLOAT, FLOAT_SIZE, PRICE_OFFSET, 0, 0);                   // price
	REGISTER->addKeyPart(2u, CT_DATASTORE_LONG_INT, INT_SIZE, CUSTOMER_ID_OFFSET, 0, 0); // customer id
	REGISTER->addKeyPart(3u, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0);       // PRIMARY SUFFIX transaction id
	REGISTER->initialize();

	PDC = DeepStore::create("./pdc.datastore", options, "PDC", CT_DATASTORE_COMPOSITE, PDC_KEY_SIZE, DATA_SIZE);
	PDC->addField(CT_DATASTORE_FLOAT, CT_DATASTORE_FLOAT, FLOAT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, false);
	PDC->addField(CT_DATASTORE_ULONGLONG, CT_DATASTORE_ULONGLONG, ULONG_SIZE, 0, 0, 0, 1, 0, 0, 0, 0, false);
	PDC->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 2, 0, 0, 0, 0, false);
	PDC->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0, 3, 0, 0, 0, 0, false);
	PDC->addKeyPart(0u, CT_DATASTORE_FLOAT, FLOAT_SIZE, PRICE_OFFSET, 0, 0);                            // price
	PDC->addKeyPart(1u, CT_DATASTORE_ULONGLONG, ULONG_SIZE, TIME_OFFSET, 0, 0);                // datetime
	PDC->addKeyPart(2u, CT_DATASTORE_LONG_INT, INT_SIZE, CUSTOMER_ID_OFFSET, 0, 0); // customer id
	PDC->addKeyPart(3u, CT_DATASTORE_LONG_INT, INT_SIZE, 0, 0, 0);       // PRIMARY SUFFIX transaction id
	PDC->initialize();

	PRIMARY->associate(MARKET, true /* has primary */);
	PRIMARY->associate(REGISTER, true /* has primary */);
	PRIMARY->associate(PDC, true /* has primary */);

	CTX = new DeepThreadContext();

	PRIMARY->open(CTX);
	MARKET->open(CTX);
	REGISTER->open(CTX);
	PDC->open(CTX);
	PRIMARY->recover(CTX, false);
}

void shutdown() {

	PRIMARY->close();

	REGISTER->close();
	MARKET->close();
	PDC->close();

	delete REGISTER;
	delete MARKET;
	delete PDC;

	delete PRIMARY;
	delete CTX;
}

int CASH_REGISTER_IDS[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int getCashRegisterId(int seed) {
	return CASH_REGISTER_IDS[seed % 10];
}

int CUSTOMER_IDS[] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
int getCustomerId(int seed) {
	return CUSTOMER_IDS[seed % 10];
}

int PRODUCT_IDS[] = {10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000};
int getProductId(int seed) {
	return PRODUCT_IDS[seed % 10];
}

float PRICES[] = {100.00, 200.00, 300.00, 400.00, 500.00, 600.00, 700.00, 800.00, 900.00, 1000.00};
float getPrice(int seed) {
	return PRICES[seed % 10];
}

void testPut(DeepThreadContext* context, int clientNum) {
	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	longtype gstart = System::currentTimeMillis();
	longtype lstart = System::currentTimeMillis();

	int beginId = PUT_COUNT_PER_CLIENT * clientNum;
	int endId = beginId + PUT_COUNT_PER_CLIENT;

	for (int j = beginId; j < endId; j += 1000) {

		context->beginTransaction(DeepThreadContext::SAVEPOINT);
		context->associateTransaction(PRIMARY);

		for (int i=0; i < 1000; i++) {
			int transactionId       = j + i;
			unsigned long long time = System::currentTimeMillis();
			int cashRegisterId      = getCashRegisterId(transactionId);
			int customerId          = getCustomerId(transactionId);
			int productId           = getProductId(transactionId);
			float price             = getPrice(transactionId);

			int cursor = 0;

			memset(value, 0, DATA_SIZE);

			memcpy(value, &transactionId, INT_SIZE);
			cursor += INT_SIZE;

			memcpy(value + cursor, &time, ULONG_SIZE);
			cursor += ULONG_SIZE;

			memcpy(value + cursor, &cashRegisterId, INT_SIZE);
			cursor += INT_SIZE;

			memcpy(value + cursor, &customerId, INT_SIZE);
			cursor += INT_SIZE;

			memcpy(value + cursor, &productId, INT_SIZE);
			cursor += INT_SIZE;

			memcpy(value + cursor, &price, FLOAT_SIZE);

			if (PRIMARY->put(context, value, DATA_SIZE) != 0) {
				printf("Client(%d) FAILED - Put : '%d'.'%llu'.'%d'.'%d'.'%d'.'%f'\n", clientNum, transactionId, time, cashRegisterId, customerId, productId, price);
				exit(1);
			}

			if ((transactionId % 100000) == 0) {
				longtype lstop = System::currentTimeMillis();
				printf("Client(%d) Put : '%d'.'%llu'.'%d'.'%d'.'%d'.'%f' --  %lld\n", clientNum, transactionId, time, cashRegisterId, customerId, productId, price, (lstop-lstart));
				lstart = System::currentTimeMillis();
			}
		}

		context->commitTransaction();
	}

	longtype gstop = System::currentTimeMillis();

	printf("Client(%d) PUT TIME: %d, %lld\n", clientNum, PUT_COUNT_PER_CLIENT, (gstop-gstart));

	free(value);
}

void testGet(DeepThreadContext* context, int clientNum, int seed) {

	int transactionId       = seed;
	int cashRegisterId      = getCashRegisterId(seed);
	int customerId          = getCustomerId(seed);
	//int productId           = getProductId(seed);
	float price             = getPrice(seed);

	int cursor = 0;

	//
	// transaction
	//
	unsigned char* key = (unsigned char*) malloc(INT_SIZE);
	memset(key, 0, INT_SIZE);

	memcpy(key, &transactionId, INT_SIZE);

	unsigned char* retkey = (unsigned char*) malloc(INT_SIZE);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	//printf("Client(%d) Transaction key: '%d'\n", clientNum, transactionId);

	unsigned int length = DATA_SIZE;

	if (PRIMARY->get(context, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Client(%d) Transaction get %d\n", clientNum, transactionId);
		exit(1);
	}

	memset(value, 0, DATA_SIZE);

	//
	// market
	//
	unsigned char* marketKey = (unsigned char*) malloc(MARKET_KEY_SIZE);
	memset(marketKey, 0, MARKET_KEY_SIZE);

	memcpy(marketKey, &price, FLOAT_SIZE);
	cursor += FLOAT_SIZE;

	memcpy(marketKey + cursor, &customerId, INT_SIZE);

	//printf("Client(%d) Market key: '%f'.'%d'\n", clientNum, price, customerId);

	unsigned char* retMarketKey = (unsigned char*) malloc(MARKET_KEY_SIZE);

	// 2 parts
	unsigned int mask = 3;

	if (MARKET->get(context, marketKey, &value, &length, retMarketKey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_NONE, false, mask) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Client(%d) Market get '%f'.'%d'\n", clientNum, price, customerId);
		exit(1);
	}

	memset(value, 0, DATA_SIZE);
	cursor = 0;

	//
	// register
	//
	unsigned char* registerKey = (unsigned char*) malloc(REGISTER_KEY_SIZE);
	memset(registerKey, 0, REGISTER_KEY_SIZE);

	memcpy(registerKey, &cashRegisterId, INT_SIZE);
	cursor += INT_SIZE;

	memcpy(registerKey + cursor, &price, FLOAT_SIZE);
	cursor += FLOAT_SIZE;

	memcpy(registerKey + cursor, &customerId, INT_SIZE);

	//printf("Client(%d) Register key: '%d'.'%f'.'%d'\n", clientNum, cashRegisterId, price, customerId);

	unsigned char* retRegisterKey = (unsigned char*) malloc(REGISTER_KEY_SIZE);

	// 3 parts
	mask = 7;

	if (REGISTER->get(context, registerKey, &value, &length, retRegisterKey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_NONE, false, mask) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Client(%d) Register get '%d'.'%f'.'%d'\n", clientNum, cashRegisterId, price, customerId);
		exit(1);
	}

	free(value);
	free(key);
	free(retkey);
	free(marketKey);
	free(retMarketKey);
	free(registerKey);
	free(retRegisterKey);
}

int main(int argc, char** argv) {

	startup(true);

	//
	// Put
	//

	PutClient** putClients = new PutClient*[CLIENTS];
	Thread** threads = new Thread*[CLIENTS];

	printf("%d CLIENTs\n", CLIENTS);

	srand(System::currentTimeMillis());

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

	//
	// Get
	//
	/*
	GetClient* clients[CLIENTS];

	printf("%d CLIENTs\n", CLIENTS);

	srand(time(0));

	for (int i = 0; i < CLIENTS; i++) {
		CLIENTS_RUNNING.getAndIncrement();

		clients[i] = new GetClient(i);
		threads[i] = new Thread(clients[i]);
		threads[i]->start();
	}

	do {
		printf("    Waiting for %d client(s)...\n", CLIENTS_RUNNING.get());

		Thread::sleep(1000);

	} while (CLIENTS_RUNNING.get() > 0);

	for (int i = 0; i < CLIENTS; i++) {
		delete clients[i];
		delete threads[i];
	}
	*/

	shutdown();

	delete [] putClients;
	delete [] threads;

	return 0;
}
