#ifdef DEEP_DISTRIBUTED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cxx/lang/Thread.h"

#include "com/deepis/db/store/relative/distributed/MapBehavior.h"
#include "com/deepis/db/store/relative/distributed/Serializer.h"
#include "com/deepis/db/store/relative/distributed/PeerInfo.h"
#include "cxx/util/concurrent/atomic/AtomicInteger.h"

#include "com/deepis/datastore/api/DeepStore.h"
#include "cxx/fabric/RealtimeFabricNode.h"
#include "com/deepis/communication/fabricconnector/FabricMessageService.cxx"
#include "com/deepis/communication/fabricconnector/FabricMessageServiceFactory.h"
#include "cxx/util/HashMap.cxx"

using namespace com::deepis::db::store::relative::distributed;
using namespace com::deepis::datastore::api;
using namespace com::deepis::communication::fabricconnector;
using namespace cxx::fabric;
using namespace cxx::lang;

static int COUNT = 1000000;
static int COMMIT = 10000;
static int DATA_SIZE = (3 * sizeof(int)) + 1;

static DeepStore* MASTER_TABLE;
static DeepStore* SLAVE_TABLE;
static IMessageService* MS_MASTER;
static IMessageService* MS_SLAVE;

RealtimeFabricNode* MASTER_NODE;
RealtimeFabricNode* SLAVE_NODE;

Thread* MASTER_THREAD;
Thread* SLAVE_THREAD;

DeepThreadContext* CTX;

cxx::util::concurrent::atomic::AtomicInteger gRunningThreads(2);

void startup(bool del) {
	PeerInfo* masterInfo = new PeerInfo(1 /* serverId */, "tcp://127.0.0.1:10001", DISTRIBUTED_MASTER);
	PeerInfo* slaveInfo = new PeerInfo(2 /* serverId */, "tcp://127.0.0.1:10002", DISTRIBUTED_HA_ASYNC_SLAVE);
		
	MS_MASTER = FabricMessageServiceFactory::createMessageService(CT_DATASTORE_LONG_INT);
	MS_SLAVE = FabricMessageServiceFactory::createMessageService(CT_DATASTORE_LONG_INT);

	MASTER_NODE = new RealtimeFabricNode(masterInfo->getServerId(), masterInfo->getAddress(), "", singletonCassiServiceBridge, MS_MASTER, &gRunningThreads);
	MASTER_NODE->addPeer("tcp://127.0.0.1:10002");

	SLAVE_NODE = new RealtimeFabricNode(slaveInfo->getServerId(), slaveInfo->getAddress(), "", singletonCassiServiceBridge, MS_SLAVE, &gRunningThreads);
	SLAVE_NODE->addPeer("tcp://127.0.0.1:10001");

	MASTER_THREAD = new Thread(MASTER_NODE);
	SLAVE_THREAD = new Thread(SLAVE_NODE);

        MASTER_THREAD->start();
        SLAVE_THREAD->start();

	DeepStore::setFileSize(4294967295);
	DeepStore::setCacheSize(10737418240);
	DeepStore::setTransactionChunk(COMMIT);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	MASTER_TABLE = DeepStore::create("./datastore", options, "MASTER_TABLE", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, DISTRIBUTED_MASTER);
	MASTER_TABLE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	MASTER_TABLE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	MASTER_TABLE->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 1, 0, 0);
	MASTER_TABLE->initialize(MS_MASTER, masterInfo);

	SLAVE_TABLE = DeepStore::create("./datastore1", options, "SLAVE_TABLE", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, DISTRIBUTED_HA_ASYNC_SLAVE);
	SLAVE_TABLE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	SLAVE_TABLE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	SLAVE_TABLE->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 1, 0, 0);
	SLAVE_TABLE->initialize(MS_SLAVE, slaveInfo);

	CTX = new DeepThreadContext();

	printf("Opening Master\n");
	MASTER_TABLE->open(CTX);
	MASTER_TABLE->recover(CTX, false);

	printf("Opening Slave\n");
	SLAVE_TABLE->open(CTX);
	SLAVE_TABLE->recover(CTX, false);

	while ((MS_MASTER->getPeers() == 0) || (MS_SLAVE->getPeers() == 0)) {
		// XXX: wait for both peers to come up
	}

	MS_MASTER->addSubscriber(MS_SLAVE->getReceiverPeerInfo());
	MS_SLAVE->subscribeTo(MS_MASTER->getReceiverPeerInfo());
}

void shutdown() {
	MASTER_NODE->stopService();
        SLAVE_NODE->stopService();

        sleep(1);

        float fMax = 3.0;

        while (0 < gRunningThreads.get()) {
                /** There is a bug in libZMQ that can cause the
                terminate to block forever. So next best is to force
                the closure here. */
                sleep(0.1);
                fMax -= 0.1;
                if (0 >= fMax) {
                        exit(0);
                }
        }

	MASTER_THREAD->join();
	SLAVE_THREAD->join();
	
	delete MASTER_THREAD;
	delete SLAVE_THREAD;

	printf("Closing Master\n");
	MASTER_TABLE->close();
	delete MASTER_TABLE;

	printf("Closing Slave\n");
	SLAVE_TABLE->close();
	delete SLAVE_TABLE;

	delete CTX;
	delete MS_MASTER;
	delete MS_SLAVE;
}

void testPut() {
	longtype start = System::currentTimeMillis();

	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(MASTER_TABLE);

	boolean more = false;

	for (int i = 1; i <= COUNT; i++) {
		int k = i * 10;
		int j = i * 100;

		unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
		memset(value, 0, DATA_SIZE);
		memcpy(value + 1, &i, sizeof(int));
		memcpy(value + 1 + sizeof(int), &k, sizeof(int));
		memcpy(value + 1 + sizeof(int) + sizeof(int), &j, sizeof(int));

		MASTER_TABLE->put(CTX, value, DATA_SIZE);
		more = true;
	
		if (i % COMMIT == 0) {
			CTX->commitTransaction();
			CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
			CTX->associateTransaction(MASTER_TABLE);
			more = false;

			fprintf(stderr, "\n MASTER COMMIT %d \n", i);
		}


		free(value);
	}

	if (more == true) {
		CTX->commitTransaction();
	}

	longtype stop = System::currentTimeMillis();

	fprintf(stderr, "MASTER PUT TIME - %llds \n", (stop - start) / 1000);
}

void testMasterGet() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(MASTER_TABLE);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 1; i <= COUNT; i++) {

		unsigned char key[sizeof(int)];
		memcpy(key, &i, sizeof(int));

		unsigned char retkey[sizeof(int)];

		unsigned int length = DATA_SIZE;

		if (MASTER_TABLE->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != 0) {
			printf("FAILED - Master get %d\n", i);
			exit(1);
		}
	}

	free(value);

	CTX->commitTransaction();
}

void testSlaveGet() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(SLAVE_TABLE);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 1; i <= COUNT; i++) {

		unsigned char key[sizeof(int)];
		memcpy(key, &i, sizeof(int));

		unsigned char retkey[sizeof(int)];

		unsigned int length = DATA_SIZE;

		if (SLAVE_TABLE->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != 0) {
			printf("FAILED - Slave get %d\n", i);
			exit(1);
		}
	}

	free(value);

	CTX->commitTransaction();
}

void waitForSlave() {
	longtype start = System::currentTimeMillis();

	DeepThreadContext* ctx_master = new DeepThreadContext();
	DeepThreadContext* ctx_slave = new DeepThreadContext();

	ctx_master->beginTransaction(DeepThreadContext::SAVEPOINT);
	ctx_master->associateTransaction(MASTER_TABLE);

	ctx_slave->beginTransaction(DeepThreadContext::SAVEPOINT);
	ctx_slave->associateTransaction(SLAVE_TABLE);

	long long master_size = 0;
	long long slave_size = 0;

	do {
		master_size = MASTER_TABLE->size(ctx_master);
		slave_size = SLAVE_TABLE->size(ctx_slave);

		fprintf(stderr, "WAITING FOR SLAVE - MASTER COUNT %lld SLAVE COUNT %lld\n", master_size, slave_size);

		Thread::sleep(1000);

	} while (slave_size < COUNT);

	delete ctx_master;
	delete ctx_slave;

	longtype stop = System::currentTimeMillis();

	fprintf(stderr, "SLAVE CATCH UP TIME - %llds \n", (stop - start) / 1000);
}

int main(int argc, char** argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	startup(true);

	testPut();

	waitForSlave();

	testMasterGet();
	testSlaveGet();

	shutdown();

	return 0;
}
#else
int main() { }
#endif
