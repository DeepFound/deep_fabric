#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com/deepis/datastore/api/DeepStore.h"

using namespace com::deepis::datastore::api;

static int COUNT = 2;
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
	PRIMARY->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
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

	for (int i = 1; i < COUNT; i++) {
		int k = i * 10;
		int j = i * 100;

		unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
		memset(value, 0, DATA_SIZE);
		memcpy(value, &i, sizeof(int));
		memcpy(value + sizeof(int), &k, sizeof(int));
		memcpy(value + sizeof(int) + sizeof(int), &j, sizeof(int));

		//printf("Put primary key: %d, value: %d %d %d\n", i, i, k, j);

		PRIMARY->put(CTX, value, DATA_SIZE);

		free(value);
	}

	CTX->commitTransaction();
}

void testUnique(bool isTx) {

	// make primary unique
	int i = COUNT + 1;

	// secondary is a duplicate
	int k = 10;
	int j = 100;

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memset(value, 0, DATA_SIZE);
	memcpy(value, &i, sizeof(int));
	memcpy(value + sizeof(int), &k, sizeof(int));
	memcpy(value + sizeof(int) + sizeof(int), &j, sizeof(int));

	printf("Testing unique secondary constraint ( isTx = %d ) : %d, value: %d %d %d\n", isTx, i, i, k, j);

	if (isTx == true) {
		CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
		CTX->associateTransaction(PRIMARY);
	}

	int status = PRIMARY->put(CTX, value, DATA_SIZE);

	if (status == CT_DATASTORE_SUCCESS) {
		printf("FAILED - put succeeded should have failed due to secondary unique constraint\n");
		exit(1);

	} else {
		int errorCode = SECONDARY1->getErrorCode();

		if (errorCode == CT_DATASTORE_ERROR_DUP_KEY) {
			printf("OK - got DUPLICATE error code on secondary datastore\n");
			if (isTx == true) {
				CTX->rollbackTransaction();
			}

		} else {
			printf("FAILED - expected DUPLICATE error code on secondary datastores, error: %d\n", errorCode);
			exit(1);
		}
	}

	free(value);
}

int main(int argc, char** argv) {

	startup(true);

	setup();

	testUnique(true /*isTx*/);
	// try again to make sure the rollback worked properly
	testUnique(true /*isTx*/);

	shutdown();

	return 0;
}
