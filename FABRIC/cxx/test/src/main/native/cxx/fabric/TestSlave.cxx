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
static int DATA_SIZE = (3 * sizeof(int)) + 1;

static DeepStore* SLAVE_TABLE;
static IMessageService* MS_SLAVE;
RealtimeFabricNode* SLAVE_NODE;
Thread* SLAVE_THREAD;

DeepThreadContext* CTX;

cxx::util::concurrent::atomic::AtomicInteger gRunningThreads(2);

void startup(bool del) {
	PeerInfo* masterInfo = new PeerInfo(1 /* serverId */, "tcp://127.0.0.1:10001", DISTRIBUTED_MASTER);
	PeerInfo* slaveInfo = new PeerInfo(2 /* serverId */, "tcp://127.0.0.1:10002", DISTRIBUTED_HA_SYNC_SLAVE);
		
	MS_SLAVE = FabricMessageServiceFactory::createMessageService(CT_DATASTORE_LONG_INT);

	SLAVE_NODE = new RealtimeFabricNode(slaveInfo->getServerId(), slaveInfo->getAddress(), "", singletonCassiServiceBridge, MS_SLAVE, &gRunningThreads);
	SLAVE_NODE->appPeer("tcp://127.0.0.1:10001");

	SLAVE_THREAD = new Thread(SLAVE_NODE);
        SLAVE_THREAD->start();

	DeepStore::setFileSize(4294967295);
	DeepStore::setCacheSize(1073741824);
	DeepStore::setTransactionChunk(10000);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	SLAVE_TABLE = DeepStore::create("./datastore1", options, "SLAVE_TABLE", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, DISTRIBUTED_HA_SYNC_SLAVE);
	SLAVE_TABLE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	SLAVE_TABLE->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	SLAVE_TABLE->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 1, 0, 0);
	SLAVE_TABLE->initialize(MS_SLAVE, slaveInfo);

	CTX = new DeepThreadContext();

	printf("Opening Slave\n");
	SLAVE_TABLE->open(CTX);
	SLAVE_TABLE->recover(CTX, false);

	while (MS_SLAVE->getPeers() == 0) {
		// XXX: wait for a peer
		sleep(1);
	}

	MS_SLAVE->subscribeTo(masterInfo);
}

void shutdown() {
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

	SLAVE_THREAD->join();
	delete SLAVE_THREAD;

	printf("Closing Slave\n");
	SLAVE_TABLE->close();
	delete SLAVE_TABLE;

	delete CTX;
	delete MS_SLAVE;
}

void testSlaveGet() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(SLAVE_TABLE);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 1; i < COUNT; i++) {

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

int main(int argc, char** argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	printf("SLAVE - STARTING UP\n");

	startup(true);

	printf("SLAVE - STARTUP COMPLETE - WAITING FOR MASTER PUT\n");

	sleep(10);

	testSlaveGet();

	shutdown();

	return 0;
}
#else
int main() { }
#endif
