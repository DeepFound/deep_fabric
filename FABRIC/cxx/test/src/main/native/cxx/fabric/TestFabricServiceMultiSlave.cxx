#ifdef DEEP_DISTRIBUTED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "cxx/util/Logger.h"
#include "cxx/lang/Runnable.h"
#include "cxx/lang/String.h"
#include "cxx/lang/Thread.h"
#include "cxx/util/CommandLineOptions.h"

#include "cxx/fabric/RealtimeFabric.h"
#include "cxx/fabric/RealtimeFabricNode.h"
#include "cxx/replication/FileReplication.h"
#include "com/deepis/communication/fabricconnector/DummyBridge.h"

#include "cxx/util/concurrent/atomic/AtomicInteger.h"

cxx::util::concurrent::atomic::AtomicInteger gRunningThreads(4);

void startup(cxx::lang::String* pcPaths) {
	cxx::fabric::RealtimeFabricNode cMaster(1 /* serverId */,
						"tcp://127.0.0.1:10001",
						"",
						cxx::replication::getFileReplicationBridgeInstance,
						pcPaths,
						&gRunningThreads);

	cMaster.addPeer("tcp://127.0.0.1:10002");
	cMaster.addPeer("tcp://127.0.0.1:10003");
//	cMaster.addPeer("tcp://127.0.0.1:10004");

	cxx::fabric::RealtimeFabricNode cSlave1(2 /* serverId */,
					       "tcp://127.0.0.1:10002",
					       "tcp://127.0.0.1:10001",
					       cxx::replication::getFileReplicationBridgeInstance,
					       &pcPaths[2],
					       &gRunningThreads);

	cxx::fabric::RealtimeFabricNode cSlave2(3 /* serverId */,
					       "tcp://127.0.0.1:10003",
					       "tcp://127.0.0.1:10001",
					       cxx::replication::getFileReplicationBridgeInstance,
					       &pcPaths[2],
					       &gRunningThreads);

//	cxx::fabric::RealtimeFabricNode cSlave3(4 /* serverId */,
//					       "tcp://127.0.0.1:10004",
//					       "tcp://127.0.0.1:10001",
//					       cxx::replication::getFileReplicationBridgeInstance,
//					       &pcPaths[2],
//					       &gRunningThreads);

	cxx::lang::Thread cMasterThread(&cMaster);
	cxx::lang::Thread cSlave1Thread(&cSlave1);
	cxx::lang::Thread cSlave2Thread(&cSlave2);
//	cxx::lang::Thread cSlave3Thread(&cSlave3);

	cMasterThread.start();
	cSlave1Thread.start();
	cSlave2Thread.start();
//	cSlave3Thread.start();

	for(;;) {
		sleep(1);
	}
}

int main(int argc, char** argv) {
        cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
        cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
        cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
        cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	com::deepis::core::util::CommandLineOptions options(argc, argv);

	cxx::lang::String cPaths[4];

	cPaths[0] = options.getString("-fromm");
	cPaths[1] = "<default>";
	cPaths[2] = "";
	cPaths[3] = "";

	startup(cPaths);

	return 0;
}
#else
int main() { }
#endif
