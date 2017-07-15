#ifdef DEEP_DISTRIBUTED
#include "cxx/lang/Thread.h"
#include "cxx/lang/System.h"

#include "cxx/util/Logger.h"

#include "cxx/util/CommandLineOptions.h"

#include "com/deepis/db/store/relative/core/RealTimeMap.h"
#include "com/deepis/db/store/relative/core/RealTimeMap.cxx"
#include "com/deepis/db/store/relative/distributed/Library.h"

using namespace cxx::lang;
using namespace cxx::util;
using namespace com::deepis::core::util;
using namespace com::deepis::db::store::relative::core;

static int COMMIT = 100;

static int DATA_SIZE = sizeof(ulongtype);;
static nbyte DATA(DATA_SIZE);

template class RealTimeMap<int>;

static IMessageService* MS_MASTER = null;
static IMessageService* MS_SLAVE = null;

static RealTimeMap<int>* MAP_MASTER = null;
static RealTimeMap<int>* MAP_SLAVE = null;
static MapFacilitator<int>* FACILITATOR_MASTER = null;
static MapFacilitator<int>* FACILITATOR_SLAVE = null;

void startup(boolean del);
void shutdown();
void printMaps();

void testPut(int start, int end);
void testRemove(int start, int remove);
void testSubscribe();
void testStatus();
void testOperationCount(MapFacilitator<int>* MF, ulongtype count);
void testMasterGet(int start, int end, boolean contains = true);
void testSlaveGet(int start, int end, boolean contains = true);

// inserts
void testSlaveRetrieveFull();
void testSlaveRetrievePartial();
void testSlaveRetrieveOutOfOrder();
void testSlaveRetrieveNoOp();

// deletes 
void testSlaveDeleteRetrieve();

void testContains(int start, int end, bool exists);

int main(int argc, char** argv) {
	CommandLineOptions options(argc, argv);

	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);

	startup(true);

	// first just put 10k rows on master, and get them on the slave
	testSlaveRetrieveFull();

	// remove last 100 rows, slave should no longer have rows, then put them back
	testSlaveDeleteRetrieve();

	// add an additional 500 rows on master, and get everything on slave, make sure that we only do the additional 500 operations (not 10.5k)
	testSlaveRetrievePartial();

	// add additional 10k rows, and get them out of order on the slave (should be able to get rows out of order in insert only)
	testSlaveRetrieveOutOfOrder();

	// getting everything again should require no operations
	testSlaveRetrieveNoOp();

	printMaps();

	shutdown();

	return 0;
}

void testSlaveRetrieveFull() {
	testPut(1, 10000);

	testSubscribe();
	printMaps();

	testStatus();

	testMasterGet(1, 10000);
	testSlaveGet(1, 10000);

	testOperationCount(FACILITATOR_MASTER, 0);
	testOperationCount(FACILITATOR_SLAVE, 10000);

	printMaps();

	testStatus();
}

void testSlaveRetrievePartial() {
	testPut(10001, 10500);
	printMaps();

	testMasterGet(1, 10500);
	testSlaveGet(1, 10500);

	testOperationCount(FACILITATOR_MASTER, 0);
	testOperationCount(FACILITATOR_SLAVE, 500);
}

void testSlaveRetrieveOutOfOrder() {
	testPut(11000, 21000);
	printMaps();

	testStatus();

	testMasterGet(11000, 21000);
	testSlaveGet(20000, 21000);

	testOperationCount(FACILITATOR_MASTER, 0);
	testOperationCount(FACILITATOR_SLAVE, 1001);

	testStatus();

	testSlaveGet(11000, 21000);
	testOperationCount(FACILITATOR_SLAVE, 9000);

	testStatus();
}

void testSlaveRetrieveNoOp() {
	testMasterGet(1, 10500);
	testSlaveGet(1, 10500);

	testOperationCount(FACILITATOR_MASTER, 0);
	testOperationCount(FACILITATOR_SLAVE, 0);

	testStatus();
}

void testSlaveDeleteRetrieve() {
	testRemove(9900, 10000);
	printMaps();

	testStatus();

	testSlaveGet(9900, 10000, false /* contains */);
	testOperationCount(FACILITATOR_SLAVE, 101);

	testPut(9900, 10000);
	printMaps();

	testMasterGet(9900, 10000, true);
	testSlaveGet(9900, 10000, true /* contains */);
	testOperationCount(FACILITATOR_SLAVE, 101);

	printMaps();
	testStatus();
}

void startup(boolean del) {
	DEEP_LOG(INFO, OTHER, " START OPEN TIME\n");

	// XXX: O_SINGULAR (no threading) is specified since berkley-db's default mode is non-thread (i.e. apples to apples, expect for one internal thread)
	longtype options = RealTimeMap<int>::O_CREATE | RealTimeMap<int>::O_SINGULAR | RealTimeMap<int>::O_FIXEDKEY | RealTimeMap<int>::O_KEYCOMPRESS;
	if (del == true) {
		options |= RealTimeMap<int>::O_DELETE;
	}

	longtype start = System::currentTimeMillis();

	Properties::setTransactionChunk(COMMIT);

	MS_MASTER = new ObjectMessageService<int>();
	MS_SLAVE = new ObjectMessageService<int>();

	MAP_MASTER = new RealTimeMap<int>("./datastoreM", options, sizeof(int), DATA_SIZE);
	FACILITATOR_MASTER = new MapFacilitator<int>(MAP_MASTER, DISTRIBUTED_MASTER, MS_MASTER);
	MAP_MASTER->setMapFacilitator(FACILITATOR_MASTER);
	MS_MASTER->initialize(FACILITATOR_MASTER);
	MAP_MASTER->mount();
	MAP_MASTER->recover(false);

	MAP_SLAVE = new RealTimeMap<int>("./datastoreS", options, sizeof(int), DATA_SIZE);
	FACILITATOR_SLAVE = new MapFacilitator<int>(MAP_SLAVE, DISTRIBUTED_LAZY_SLAVE, MS_SLAVE);
	MAP_SLAVE->setMapFacilitator(FACILITATOR_SLAVE);
	MS_SLAVE->initialize(FACILITATOR_SLAVE);
	MAP_SLAVE->mount();
	MAP_SLAVE->recover(false);

	longtype stop = System::currentTimeMillis();

	DEEP_LOG(INFO, OTHER, " COMPLETE OPEN TIME: %lld\n", (stop-start));
}

void shutdown() {
	MAP_SLAVE->unmount(false);

	delete MAP_SLAVE;
	MAP_SLAVE = null;

	delete FACILITATOR_SLAVE;
	FACILITATOR_SLAVE = null;

	MAP_MASTER->unmount(false);

	delete MAP_MASTER;
	MAP_MASTER = null;

	delete FACILITATOR_MASTER;
	FACILITATOR_MASTER = null;

	delete MS_MASTER;
	MS_MASTER = null;

	delete MS_SLAVE;
	MS_SLAVE = null;
}

void waitForIndex() {
	for (int x = 15; x > 0; x--) {
		DEEP_LOG(INFO, OTHER, " SLEEP FOR INDEX %ds...\n", x);
		Thread::sleep(1000);
	}
}

void printMaps() {
	DEEP_LOG(INFO, OTHER, " \n\nMASTER VIRTUAL REP\n");
	FACILITATOR_MASTER->printVirtualRepresentation();

	DEEP_LOG(INFO, OTHER, " \n\nSLAVE VIRTUAL REP - KEY SPACE VERSION %lld \n", FACILITATOR_SLAVE->getVirtualKeySpaceVersion());
	FACILITATOR_SLAVE->printMap();
}

void testSubscribe() {
	FACILITATOR_MASTER->addSubscriber(FACILITATOR_SLAVE->getPeerInfo());
	FACILITATOR_SLAVE->subscribeTo(FACILITATOR_MASTER->getPeerInfo());
}

void testOperationCount(MapFacilitator<int>* MF, ulongtype count) {
	ulongtype opCount = MF->getSegmentOperationCount();
	boolean pass = (count == opCount);

	if (pass == true) {
		DEEP_LOG(INFO, OTHER, "%s OPERATION COUNT CORRECT %lld \n", MF == FACILITATOR_MASTER ? "MASTER" : "SLAVE", count);
	} else {
		DEEP_LOG(ERROR, OTHER, "FAILED - %s OPERATION COUNT ACTUAL %lld != EXPECTED %lld \n", MF == FACILITATOR_MASTER ? "MASTER" : "SLAVE", opCount, count);
		exit(-1);
	}

	MF->resetSegmentOperationCount();
}

void testStatus() {
	DEEP_LOG(INFO, OTHER, "MASTER MAX VIEWPOINT %d SLAVE MAX VIEWPOINT %d\n", FACILITATOR_MASTER->getMaxViewpoint(), FACILITATOR_SLAVE->getMaxViewpoint());
	DEEP_LOG(INFO, OTHER, "SLAVE KEYSPACE OWNER HAS UPDATES %d\n ", FACILITATOR_SLAVE->checkOwnerForUpdates());
}

void testPut(int start, int end) {
	int commit = 0;
	Transaction* tx = Transaction::create(true);
	tx->begin();
	MAP_MASTER->associate(tx);

	longtype gstart = System::currentTimeMillis();
	longtype lstart = System::currentTimeMillis();
	for (int i = start; i <= end; i++) {

		int key = i;
		if (MAP_MASTER->put(key, &DATA, RealTimeMap<int>::UNIQUE, tx) == false) {
			DEEP_LOG(ERROR, OTHER, "FAILED - Create %d, %d\n", i, MAP_MASTER->getErrorCode());
			exit(-1);
		}

		if ((i % 100000) == 0) {
			longtype lstop = System::currentTimeMillis();
			DEEP_LOG(INFO, OTHER, "   Put %d, %lld\n", i, (lstop-lstart));
			lstart = System::currentTimeMillis();
		}

		if (++commit == COMMIT) {
			//DEEP_LOG(INFO, OTHER, " COMMIT AT: %d\n", i);
			commit = 0;
			tx->commit(tx->getLevel());
			tx->begin();
		}
	}

	if (commit != 0) {
		DEEP_LOG(INFO, OTHER, "  MORE TO COMMIT: %d\n", commit);
		tx->commit(tx->getLevel());
	}

	Transaction::destroy(tx);

	longtype gstop = System::currentTimeMillis();

	DEEP_LOG(INFO, OTHER, " PUT TIME: %d, %lld\n", end - start, (gstop-gstart));

	Thread::sleep(1000);
}

void testRemove(int start, int end) {
	int commit = 0;
	Transaction* tx = Transaction::create(true);
	tx->begin();
	MAP_MASTER->associate(tx);

	longtype gstart = System::currentTimeMillis();
	longtype lstart = System::currentTimeMillis();

	for (int i = start; i <= end; i++) {

		int key = i;
		if (MAP_MASTER->remove(key, &DATA, RealTimeMap<int>::DELETE_POPULATED, tx, RealTimeMap<int>::LOCK_WRITE) == false) {
			DEEP_LOG(ERROR, OTHER, "FAILED - Remove %d, %d\n", i, MAP_MASTER->getErrorCode());
			exit(-1);
		}

		if ((i % 100000) == 0) {
			longtype lstop = System::currentTimeMillis();
			DEEP_LOG(INFO, OTHER, "   Remove %d, %lld\n", i, (lstop-lstart));
			lstart = System::currentTimeMillis();
		}

		if (++commit == COMMIT) {
			commit = 0;
			tx->commit(tx->getLevel());
			tx->begin();
		}
	}

	if (commit != 0) {
		DEEP_LOG(INFO, OTHER, "  MORE TO COMMIT: %d\n", commit);
		tx->commit(tx->getLevel());
	}

	Transaction::destroy(tx);

	longtype gstop = System::currentTimeMillis();

	DEEP_LOG(INFO, OTHER, " REMOVE TIME: %d, %lld\n", end - start, (gstop-gstart));

	Thread::sleep(1000);
}

void testMasterGet(int start, int end, boolean contains) {
	Transaction* tx = Transaction::create(true);
	tx->begin();
	MAP_MASTER->associate(tx);

	int retkey = 0;

	longtype gstart = System::currentTimeMillis();
	longtype lstart = System::currentTimeMillis();
	for (int i = start; i < end; i++) {

		int key = i;
		if (MAP_MASTER->get(key, &DATA, RealTimeMap<int>::EXACT, &retkey, tx) != contains) {
			DEEP_LOG(ERROR, OTHER, "FAILED - Master Get %d\n", i);
			exit(-1);
		}

		if ((i % 100000) == 0) {
			longtype lstop = System::currentTimeMillis();
			DEEP_LOG(INFO, OTHER, " Master Get %d, %lld\n", i, (lstop-lstart));
			lstart = System::currentTimeMillis();
		}
	}

	Transaction::destroy(tx);

	longtype gstop = System::currentTimeMillis();

	DEEP_LOG(INFO, OTHER, "MASTER GET TIME: %d, %d, %lld\n", start, end, (gstop-gstart));
}

void testSlaveGet(int start, int end, boolean contains) {
	Transaction* tx = Transaction::create(true);
	tx->begin();
	tx->setVirtualKeySpaceVersion(FACILITATOR_SLAVE->getVirtualKeySpaceVersion());
	MAP_SLAVE->associate(tx);

	int retkey = 0;

	longtype gstart = System::currentTimeMillis();
	longtype lstart = System::currentTimeMillis();
	for (int i = start; i < end; i++) {

		int key = i;
		if (MAP_SLAVE->get(key, &DATA, RealTimeMap<int>::EXACT, &retkey, tx) != contains) {
			DEEP_LOG(ERROR, OTHER, "FAILED - Slave Get %d\n", i);
			exit(-1);
		}

		if ((i % 100000) == 0) {
			longtype lstop = System::currentTimeMillis();
			DEEP_LOG(INFO, OTHER, " Slave Get %d, %lld\n", i, (lstop-lstart));
			lstart = System::currentTimeMillis();
		}
	}

	tx->commit(tx->getLevel());
	Transaction::destroy(tx);

	longtype gstop = System::currentTimeMillis();

	DEEP_LOG(INFO, OTHER, "SLAVE GET TIME: %d, %d, %lld\n", start, end, (gstop-gstart));
}
#else
int main() { }
#endif
