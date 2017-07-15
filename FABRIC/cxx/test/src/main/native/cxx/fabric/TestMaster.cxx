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

static int COUNT = 1000;
static int COMMIT = 100;
static int DATA_SIZE = (3 * sizeof(int)) + 1;

static DeepStore* MASTER_TABLE;
static IMessageService* MS_MASTER;
RealtimeFabricNode* MASTER_NODE;
Thread* MASTER_THREAD;
DeepThreadContext* CTX;

cxx::util::concurrent::atomic::AtomicInteger gRunningThreads(2);

void startup(bool del) {
	PeerInfo* masterInfo = new PeerInfo(1 /* serverId */, "tcp://127.0.0.1:10001", DISTRIBUTED_MASTER);
	PeerInfo* slaveInfo = new PeerInfo(2 /* serverId */, "tcp://127.0.0.1:10002", DISTRIBUTED_HA_SYNC_SLAVE);
		
	MS_MASTER = FabricMessageServiceFactory::createMessageService(CT_DATASTORE_LONG_INT);

	MASTER_NODE = new RealtimeFabricNode(masterInfo->getServerId(), masterInfo->getAddress(), "", singletonCassiServiceBridge, MS_MASTER, &gRunningThreads);
	MASTER_NODE->addPeer("tcp://127.0.0.1:10002");

	MASTER_THREAD = new Thread(MASTER_NODE);

        MASTER_THREAD->start();

	DeepStore::setFileSize(4294967295);
	DeepStore::setCacheSize(1073741824);
	DeepStore::setTransactionChunk(10000);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	MASTER_TABLE = DeepStore::create("./datastore", options, "MASTER_TABLE", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, DISTRIBUTED_MASTER);
	MASTER_TABLE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	MASTER_TABLE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	MASTER_TABLE->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 1, 0, 0);
	MASTER_TABLE->initialize(MS_MASTER, masterInfo);

	CTX = new DeepThreadContext();

	printf("Opening Master\n");
	MASTER_TABLE->open(CTX);
	MASTER_TABLE->recover(CTX, false);

	while (MS_MASTER->getPeers() == 0) {
		// XXX: wait for a peer
		sleep(1);
	}

	MS_MASTER->addSubscriber(slaveInfo);
}

void shutdown() {
	MASTER_NODE->stopService();

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
	delete MASTER_THREAD;

	printf("Closing Master\n");
	MASTER_TABLE->close();
	delete MASTER_TABLE;

	delete CTX;
	delete MS_MASTER;
}

void testPut() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(MASTER_TABLE);

	boolean more = false;

	for (int i = 1; i < COUNT; i++) {
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
		}


		free(value);
	}

	if (more == true) {
		CTX->commitTransaction();
	}
}

void testMasterGet() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(MASTER_TABLE);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 1; i < COUNT; i++) {

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

int main(int argc, char** argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	printf("MASTER - STARTING UP\n");

	startup(true);

	printf("MASTER - STARTUP COMPLETE\n");

	testPut();

	testMasterGet();

	printf("MASTER PUT / GET COMPLETE\n");

	while (MS_MASTER->getPeers() != 0) {
		// XXX: wait for slave to finish
		sleep(1);
	}


	return 0;
}
#else
int main() { }
#endif
