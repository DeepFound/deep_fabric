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

cxx::util::concurrent::atomic::AtomicInteger gRunningThreads(1);

void startup() {
	cxx::fabric::RealtimeFabricNode cFabric(1 /* serverId */,
						"tcp://127.0.0.1:10001",
						"tcp://127.0.0.1:10001",
						::getDummyBridgeInstance,
						0,
						&gRunningThreads);

	cxx::lang::Thread cFabricThread(&cFabric);

	cFabricThread.start();

	sleep(1);

	cFabric.stopService();

	sleep(3);
}

int main(int argc, char** argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	startup();

	return 0;
}
#else
int main() { }
#endif
