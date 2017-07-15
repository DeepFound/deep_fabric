#ifdef DEEP_DISTRIBUTED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com/deepis/db/store/relative/distributed/MapBehavior.h"
#include "com/deepis/db/store/relative/distributed/ObjectMessageService.cxx"
#include "com/deepis/datastore/api/DeepStore.h"
#include "com/deepis/datastore/api/deep/ObjectMessageServiceFactory.h"

using namespace com::deepis::db::store::relative::distributed;
using namespace com::deepis::datastore::api;

static int COUNT = 1000;
static int DATA_SIZE = (3 * sizeof(int)) + 1;

static IMessageService* MS_MASTER = null;
static IMessageService* MS_SLAVE = null;

static DeepStore* MASTER;
static DeepStore* SLAVE;

DeepThreadContext* CTX;

void startup(bool del) {
	//DeepStore::setDebugEnabled(true);

	DeepStore::setFileSize(4294967295);
	DeepStore::setCacheSize(1073741824);
	DeepStore::setTransactionChunk(10000);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	MS_MASTER = ObjectMessageServiceFactory::createMessageService(CT_DATASTORE_LONG_INT);
	MS_SLAVE = ObjectMessageServiceFactory::createMessageService(CT_DATASTORE_LONG_INT);

	MASTER = DeepStore::create("./datastore", options, "MASTER", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, DISTRIBUTED_MASTER);
	MASTER->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	MASTER->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	MASTER->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 1, 0, 0);
	MASTER->initialize(MS_MASTER);

	SLAVE = DeepStore::create("./datastore1", options, "SLAVE", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, DISTRIBUTED_HA_SYNC_SLAVE);
	SLAVE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	SLAVE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	SLAVE->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 1, 0, 0);
	SLAVE->initialize(MS_SLAVE);

	CTX = new DeepThreadContext();

	printf("Opening Master\n");
	MASTER->open(CTX);
	MASTER->recover(CTX, false);

	printf("Opening Slave\n");
	SLAVE->open(CTX);
	SLAVE->recover(CTX, false);

	MS_MASTER->addSubscriber(MS_SLAVE->getReceiverPeerInfo());
	MS_SLAVE->subscribeTo(MS_MASTER->getReceiverPeerInfo());
}

void shutdown() {
	printf("Closing Master\n");
	MASTER->close();
	delete MASTER;

	printf("Closing Slave\n");
	SLAVE->close();
	delete SLAVE;

	delete CTX;

	delete MS_MASTER;
	delete MS_SLAVE;
}

void testPut() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(MASTER);

	for (int i = 1; i < COUNT; i++) {
		int k = i * 10;
		int j = i * 100;

		unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
		memset(value, 0, DATA_SIZE);
		memcpy(value + 1, &i, sizeof(int));
		memcpy(value + 1 + sizeof(int), &k, sizeof(int));
		memcpy(value + 1 + sizeof(int) + sizeof(int), &j, sizeof(int));

		MASTER->put(CTX, value, DATA_SIZE);

		free(value);
	}

	CTX->commitTransaction();
}

void testMasterGet() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(MASTER);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 1; i < COUNT; i++) {

		unsigned char key[sizeof(int)];
		memcpy(key, &i, sizeof(int));

		unsigned char retkey[sizeof(int)];

		unsigned int length = DATA_SIZE;

		if (MASTER->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != 0) {
			printf("FAILED - Master get %d\n", i);
			exit(1);
		}
	}

	free(value);

	CTX->commitTransaction();
}

void testSlaveGet() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(SLAVE);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 1; i < COUNT; i++) {

		unsigned char key[sizeof(int)];
		memcpy(key, &i, sizeof(int));

		unsigned char retkey[sizeof(int)];

		unsigned int length = DATA_SIZE;

		if (SLAVE->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != 0) {
			printf("FAILED - Slave get %d\n", i);
			exit(1);
		}
	}

	free(value);

	CTX->commitTransaction();
}

int main(int argc, char** argv) {
	startup(true);

	testPut();
	testMasterGet();
	testSlaveGet();

	shutdown();

	return 0;
}
#else
int main() { }
#endif
