#ifdef DEEP_DISTRIBUTED
#include "cxx/lang/System.h"
#include "cxx/util/Logger.h"

#include "cxx/io/EncodeProtocol.h"
#include "com/deepis/db/store/relative/distributed/StatusRequest.h"
#include "com/deepis/db/store/relative/distributed/DataRequest.h"
#include "com/deepis/db/store/relative/distributed/VirtualInfo.h"
#include "com/deepis/db/store/relative/distributed/LogEntry.h"
#include "com/deepis/db/store/relative/distributed/QuickList.h"
#include "com/deepis/db/store/relative/distributed/PeerInfo.h"

#include "com/deepis/db/store/relative/distributed/Serializer.h"
#include "com/deepis/db/store/relative/distributed/ObjectMessageService.cxx"

using namespace cxx::io;
using namespace cxx::lang;
using namespace com::deepis::db::store::relative::distributed;

const int KEY = 123456;
const unsigned int MAX_VIEWPOINT = 654321;
const int REQUEST_TYPE = 1;
const int ACTION = 1;
const int VIEWPOINT = 99;
const int VIRTUAL_SIZE = 1500;
const unsigned int MIN_VIEWPOINT = 123;
const unsigned int MAX_VIRTUAL_VIEWPOINT = 33;
const unsigned int MAX_REAL_VIEWPOINT = 22;
const long VERSION = 987654321;

const int DATA_SIZE = 26;
char DATA[DATA_SIZE] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

void testStatusRequest();
void testStatusResponse();
void testDataRequest();
void testVirtualInfo();
void testDataLogEntry();
void testDataLogEntryBytes();
void testVirtualLogEntry();
void testPeerInfo();
void testVirtualKeySpaceVersion();
void testQuickListOfDataLogEntry();
void testQuickListOfVirtualLogEntry();

// TODO
#if 0
void testVirtualKeySpace();
void testSchema();
#endif

int main(int argc, char** argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);

	testStatusRequest();

	testStatusResponse();

	testDataRequest();

	testVirtualInfo();

	testDataLogEntry();

	testDataLogEntryBytes();

	testVirtualLogEntry();

	testPeerInfo();

	testVirtualKeySpaceVersion();

	testQuickListOfDataLogEntry();

	testQuickListOfVirtualLogEntry();

	// TODO
	#if 0
	testVirtualKeySpace();

	testSchema();
	#endif

	return 0;
}

void testStatusRequest() {
	DEEP_LOG(INFO, OTHER, " TEST STATUS REQUEST\n");
	
	StatusRequest<int> sr(KEY, MAX_VIEWPOINT);
	StatusRequest<int> sr2;

	ObjectMessageService<int> ms;

	encodeProtocol::writer* encodeWriter = Serializer<int>::serialize(&ms, &sr, null);

	ubytetype* data = null;
	uinttype dataSize = 0;
	encodeWriter->getMutableBuffer(data, dataSize);

	Serializer<int>::deserialize(&sr2, data, dataSize);

	if (sr.equals(&sr2) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - test status request\n");
		exit(-1);
	}

	delete encodeWriter;
}

void testStatusResponse() {
	DEEP_LOG(INFO, OTHER, " TEST STATUS RESPONSE\n");
	
	StatusResponse sr(MAX_VIEWPOINT);
	StatusResponse sr2;

	ObjectMessageService<int> ms;

	encodeProtocol::writer* encodeWriter = Serializer<int>::serialize(&ms, &sr, null);

	ubytetype* data = null;
	uinttype dataSize = 0;
	encodeWriter->getMutableBuffer(data, dataSize);

	Serializer<int>::deserialize(&sr2, data, dataSize);

	if (sr.equals(&sr2) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - test status response\n");
		exit(-1);
	}

	delete encodeWriter;
}

void testDataRequest() {
	DEEP_LOG(INFO, OTHER, " TEST DATA REQUEST\n");

	DataRequest<int> dr((DataRequestType)REQUEST_TYPE, KEY, VIRTUAL_SIZE, MAX_VIRTUAL_VIEWPOINT, MAX_REAL_VIEWPOINT);
	DataRequest<int> dr2;


	ObjectMessageService<int> ms;

	encodeProtocol::writer* encodeWriter = Serializer<int>::serialize(&ms, &dr, null);

	ubytetype* data = null;
	uinttype dataSize = 0;
	encodeWriter->getMutableBuffer(data, dataSize);

	Serializer<int>::deserialize(&dr2, data, dataSize);
	
	if (dr.equals(&dr2) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - test data request\n");
		exit(-1);
	}

	delete encodeWriter;
}

void testVirtualInfo() {
	DEEP_LOG(INFO, OTHER, " TEST VIRTUAL INFO\n");

	VirtualInfo vi(VIRTUAL_SIZE, MIN_VIEWPOINT, MAX_VIEWPOINT, VERSION);
	VirtualInfo vi2;

	ObjectMessageService<int> ms;

	encodeProtocol::writer* encodeWriter = Serializer<int>::serialize(&ms, &vi, null);

	ubytetype* data = null;
	uinttype dataSize = 0;
	encodeWriter->getMutableBuffer(data, dataSize);

	Serializer<voidptr>::deserialize(&vi2, data, dataSize);

	if (vi.equals(&vi2) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - test virtual info\n");
		exit(-1);
	}

	delete encodeWriter;
}

void testDataLogEntry() {
	DEEP_LOG(INFO, OTHER, " TEST DATA LOG ENTRY\n");

	DataLogEntry<int> dle((LogEntryAction)ACTION, KEY, VIEWPOINT, DATA_SIZE, (bytearray) DATA);
	DataLogEntry<int> dle2;

	ObjectMessageService<int> ms;

	encodeProtocol::writer* encodeWriter = Serializer<int>::serialize(&ms, &dle, null);

	ubytetype* data = null;
	uinttype dataSize = 0;
	encodeWriter->getMutableBuffer(data, dataSize);

	Serializer<int>::deserialize(&dle2, data, dataSize);

	if (dle.equals(&dle2) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - data log entry\n");
		exit(-1);
	}

	delete encodeWriter;
}

void testDataLogEntryBytes() {
	DEEP_LOG(INFO, OTHER, " TEST DATA LOG ENTRY BYTES\n");

	DataLogEntry<int> dle((LogEntryAction)ACTION, KEY, VIEWPOINT, DATA_SIZE, (bytearray) DATA);
	DataLogEntry<int> dle2;

	uinttype dataSize = 0;
	ubytetype* data = dle.getBytes(&dataSize);

	dle2.setBytes(data);

	delete data;

	if (dle.equals(&dle2) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - data log entry bytes\n");
		exit(-1);
	}
}

void testVirtualLogEntry() {
	DEEP_LOG(INFO, OTHER, " TEST VIRTUAL LOG ENTRY\n");

	VirtualLogEntry<int> vle((LogEntryAction)ACTION, KEY, VIRTUAL_SIZE, MIN_VIEWPOINT, MAX_VIEWPOINT);
	VirtualLogEntry<int> vle2;

	ObjectMessageService<int> ms;

	encodeProtocol::writer* encodeWriter = Serializer<int>::serialize(&ms, &vle, null);

	ubytetype* data = null;
	uinttype dataSize = 0;
	encodeWriter->getMutableBuffer(data, dataSize);

	Serializer<int>::deserialize(&vle2, data, dataSize);

	if (vle.equals(&vle2) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - virtual log entry\n");
		exit(-1);
	}

	delete encodeWriter;
}

void testPeerInfo() {
	DEEP_LOG(INFO, OTHER, " TEST PEER INFO\n");

	PeerInfo pi(123, "127.0.0.1:1001", DISTRIBUTED_MASTER, null);
	PeerInfo pi2;

	ObjectMessageService<int> ms;

	encodeProtocol::writer* encodeWriter = Serializer<int>::serialize(&ms, &pi, null);

	ubytetype* data = null;
	uinttype dataSize = 0;
	encodeWriter->getMutableBuffer(data, dataSize);

	Serializer<int>::deserialize(&pi2, data, dataSize);

	if (pi.equals(&pi2) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - PEER INFO\n");
		exit(-1);
	}

	delete encodeWriter;
}

void testVirtualKeySpaceVersion() {
	DEEP_LOG(INFO, OTHER, " TEST VIRTUAL KEY SPACE VERSION\n");

	longtype v1 = 1234;
	longtype v2 = 0;

	ObjectMessageService<int> ms;

	encodeProtocol::writer* encodeWriter = Serializer<int>::serialize(&ms, v1, null);

	ubytetype* data = null;
	uinttype dataSize = 0;
	encodeWriter->getMutableBuffer(data, dataSize);

	Serializer<int>::deserialize(&v2, data, dataSize);

	if (v1 != v2) {
		DEEP_LOG(ERROR, OTHER, "FAILED - VIRTUAL KEY SPACE VERSION\n");
		exit(-1);
	}

	delete encodeWriter;
}

void testQuickListOfDataLogEntry() {
	DEEP_LOG(INFO, OTHER, " TEST QUICK LIST OF DATA LOG ENTRY\n");

	DataLogEntry<int>* dle1 = new DataLogEntry<int>((LogEntryAction)ACTION, KEY, VIEWPOINT, DATA_SIZE, (bytearray) DATA);
	DataLogEntry<int>* dle2 = new DataLogEntry<int>((LogEntryAction)ACTION, KEY + 1, VIEWPOINT, DATA_SIZE, (bytearray) DATA);
	DataLogEntry<int>* dle3 = new DataLogEntry<int>((LogEntryAction)ACTION, KEY + 2, VIEWPOINT, DATA_SIZE, (bytearray) DATA);
	
	QuickList<DataLogEntry<int>*>* entries = new QuickList<DataLogEntry<int>*>();
	QuickList<DataLogEntry<int>*>* entries2 = new QuickList<DataLogEntry<int>*>();

	entries->addEntry(dle1);
	entries->addEntry(dle2);
	entries->addEntry(dle3);

	ObjectMessageService<int> ms;

	encodeProtocol::writer* encodeWriter = Serializer<int>::serialize(&ms, entries, null);

	ubytetype* data = null;
	uinttype dataSize = 0;
	encodeWriter->getMutableBuffer(data, dataSize);

	Serializer<int>::deserialize(entries2, data, dataSize);

	DataLogEntry<int>* dle = entries2->getHead()->getEntry();
	if (dle1->equals(dle) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - QUICK LIST DATA LOG ENTRY 1\n");
		exit(-1);
	}
	entries2->advanceHead();

	dle = entries2->getHead()->getEntry();
	if (dle2->equals(dle) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - QUICK LIST DATA LOG ENTRY 2\n");
		exit(-1);
	}
	entries2->advanceHead();

	dle = entries2->getHead()->getEntry();
	if (dle3->equals(dle) == false) {
		DEEP_LOG(ERROR, OTHER, "FAILED - QUICK LIST DATA LOG ENTRY 3\n");
		exit(-1);
	}

	delete entries;
	delete entries2;
}

void testQuickListOfVirtualLogEntry() {
	DEEP_LOG(INFO, OTHER, " TEST QUICK LIST OF VIRTUAL LOG ENTRY\n");
}

// TODO
#if 0
void testVirtualKeySpace() {
	DEEP_LOG(INFO, OTHER, " TEST VIRTUAL KEY SPACE\n");

}

void testSchema() {
	DEEP_LOG(INFO, OTHER, " TEST SCHEMA\n");

}
#endif
#else
int main() { }
#endif
