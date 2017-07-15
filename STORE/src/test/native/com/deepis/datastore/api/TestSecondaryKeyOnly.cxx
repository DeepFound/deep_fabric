#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com/deepis/datastore/api/DeepStore.h"

using namespace com::deepis::datastore::api;

static int DATA_SIZE = (3 * sizeof(int)) + 1;

static DeepStore* PRIMARY;
static DeepStore* SECONDARY1;

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

	PRIMARY = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);
	PRIMARY->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, false);
	PRIMARY->initialize();

	SECONDARY1 = DeepStore::create("./s1.datastore", options, "SECONDARY1", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);
	SECONDARY1->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	SECONDARY1->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), sizeof(int), 0, 0);
	SECONDARY1->initialize();

	PRIMARY->associate(SECONDARY1, false);

	CTX = new DeepThreadContext();

	printf("Opening Primary\n");
	PRIMARY->open(CTX);

	printf("Opening Secondary 1\n");
	SECONDARY1->open(CTX);
	PRIMARY->recover(CTX, false);
}

void shutdown() {

	printf("Closing Primary\n");
	PRIMARY->close();

	printf("Closing Secondary 1\n");
	SECONDARY1->close();

	delete SECONDARY1;
	delete PRIMARY;

	delete CTX;
}

void setup() {

	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	int i = 1;
	int k = 10;
	int j = 100;

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memset(value, 0, DATA_SIZE);
	memcpy(value, &i, sizeof(int));
	memcpy(value + sizeof(int), &k, sizeof(int));
	memcpy(value + sizeof(int) + sizeof(int), &j, sizeof(int));

	//printf("Put primary key: %d, value: %d %d %d\n", i, i, k, j);

	PRIMARY->put(CTX, value, DATA_SIZE);

	free(value);

	CTX->commitTransaction();
}

void testSecondaryKeyOnly() {

	int i = 1;
	int k = 10;
	int j = 100;

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memset(value, 0, DATA_SIZE);
	memcpy(value, &i, sizeof(int));
	memcpy(value + sizeof(int), &k, sizeof(int));
	memcpy(value + sizeof(int) + sizeof(int), &j, sizeof(int));

	k = 11;
	j = 111;
	unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(newvalue, 0, DATA_SIZE);
	memcpy(newvalue, &i, sizeof(int));
	memcpy(newvalue + sizeof(int), &k, sizeof(int));
	memcpy(newvalue + sizeof(int) + sizeof(int), &j, sizeof(int));

	unsigned int length = DATA_SIZE;
	unsigned char* retkey = (unsigned char*) malloc(sizeof(int));
	unsigned char* retvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(retvalue, 0, DATA_SIZE);

	int oldSecondaryKey = 11;

	unsigned char bOldSecondaryKey[sizeof(int)];
	memcpy(bOldSecondaryKey, &oldSecondaryKey, sizeof(int));

	//
	// delete, savepoint, insert, savepoint, update, rollback, rollback, rollback
	//
	int masterTrx = CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	int status = PRIMARY->remove(CTX, value, DATA_SIZE);
	if (status != CT_DATASTORE_SUCCESS) {
		printf("FAILED - remove failed\n");
		exit(1);
	}

	int oneTrx = CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	status = PRIMARY->put(CTX, value, DATA_SIZE);
	if (status != CT_DATASTORE_SUCCESS) {
		printf("FAILED - put failed\n");
		exit(1);
	}

	int twoTrx = CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	status = PRIMARY->update(CTX, value, DATA_SIZE, newvalue, DATA_SIZE);
	if (status != CT_DATASTORE_SUCCESS) {
		printf("FAILED - update failed\n");
		exit(1);
	}

	// the secondary key '11' should be present here
	status = SECONDARY1->get(CTX, bOldSecondaryKey, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT);
	if (status != CT_DATASTORE_SUCCESS) {
		printf("FAILED - get failed, but should have succeeded\n");
		exit(1);
	}

	CTX->rollbackTransaction(twoTrx);

	// the secondary key '11' should have been rolled back at this point, it was add during the update
	status = SECONDARY1->get(CTX, bOldSecondaryKey, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT);
	if (status == CT_DATASTORE_SUCCESS) {
		printf("FAILED - get succeeded after rollback 2, but should have failed, %d\n", *((int*) retkey));
		exit(1);
	}

	CTX->rollbackTransaction(oneTrx);

	CTX->rollbackTransaction(masterTrx);

	free(value);

	free(newvalue);
}

int main(int argc, char** argv) {

	startup(true);

	setup();

	testSecondaryKeyOnly();

	shutdown();

	return 0;
}
