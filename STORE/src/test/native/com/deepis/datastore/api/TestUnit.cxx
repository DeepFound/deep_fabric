#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com/deepis/datastore/api/DeepStore.h"

using namespace com::deepis::datastore::api;

static int COUNT = 1000;
static int DATA_SIZE = (3 * sizeof(int)) + 1;

static DeepStore* PRIMARY;
static DeepStore* SECONDARY1;
static DeepStore* SECONDARY2;

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
	PRIMARY->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	PRIMARY->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 1, 0, 0);
	PRIMARY->initialize();

	SECONDARY1 = DeepStore::create("./s1.datastore", options, "SECONDARY1", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);
	SECONDARY1->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	SECONDARY1->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	SECONDARY1->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 1 + sizeof(int), 0, 0);
	SECONDARY1->initialize();

	SECONDARY2 = DeepStore::create("./s2.datastore", options, "SECONDARY2", CT_DATASTORE_COMPOSITE, sizeof(int) + sizeof(int), DATA_SIZE);
	SECONDARY2->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	SECONDARY2->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 1, 0, 0, 0, 0, false);
	SECONDARY2->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 1 + sizeof(int), 0, 0);
	SECONDARY2->addKeyPart(1u, CT_DATASTORE_LONG_INT, sizeof(int), 1 + sizeof(int) + sizeof(int), 0, 0);
	SECONDARY2->initialize();

	PRIMARY->associate(SECONDARY1, false);
	PRIMARY->associate(SECONDARY2, false);

	CTX = new DeepThreadContext();

	printf("Opening Primary\n");
	PRIMARY->open(CTX);

	printf("Opening Secondary 1\n");
	SECONDARY1->open(CTX);

	printf("Opening Secondary 2\n");
	SECONDARY2->open(CTX);
	PRIMARY->recover(CTX, false);
}

void shutdown() {

	printf("Closing Primary\n");
	PRIMARY->close();

	printf("Closing Secondary 1\n");
	SECONDARY1->close();

	printf("Closing Secondary 2\n");
	SECONDARY2->close();

	delete SECONDARY2;
	delete SECONDARY1;
	delete PRIMARY;
	delete CTX;
}

void testPut() {

	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	for (int i = 1; i < COUNT; i++) {
		int k = i * 10;
		int j = i * 100;

		unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
		memset(value, 0, DATA_SIZE);
		memcpy(value + 1, &i, sizeof(int));
		memcpy(value + 1 + sizeof(int), &k, sizeof(int));
		memcpy(value + 1 + sizeof(int) + sizeof(int), &j, sizeof(int));

		//printf("Put primary key: %d, value: %d %d %d\n", i, i, k, j);

		PRIMARY->put(CTX, value, DATA_SIZE);

		free(value);
	}

	CTX->commitTransaction();
}

void testGet() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 1; i < COUNT; i++) {

		unsigned char key[sizeof(int)];
		memcpy(key, &i, sizeof(int));

		unsigned char retkey[sizeof(int)];

		unsigned int length = DATA_SIZE;

		if (PRIMARY->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != 0) {
			printf("FAILED - Primary get %d\n", i);
			exit(1);
		}

		//printf("primary key: %d value: %d %d %d\n", i, *((int*) (value + 1)), *((int*) (value + 1 + sizeof(int))), *((int*) (value + 1 + sizeof(int) + sizeof(int))));


		memset(value, 0, DATA_SIZE);

		int k = i * 10;

		unsigned char s1key[sizeof(int) + sizeof(int)];
		memcpy(s1key, &k, sizeof(int));

		unsigned char rets1key[sizeof(int) + sizeof(int)];

		if (SECONDARY1->get(CTX, s1key, &value, &length, rets1key, CT_DATASTORE_GET_EXACT) != 0) {
			printf("FAILED - s1 get %d\n", k);
			exit(1);
		}

		//printf("s1 key: %d value: %d %d %d\n", k, *((int*) (value + 1)), *((int*) (value + 1 + sizeof(int))), *((int*) (value + 1 + sizeof(int) + sizeof(int))));

		memset(value, 0, DATA_SIZE);

		int j = i * 100;

		unsigned char s2key[sizeof(int) + sizeof(int)];
		memcpy(s2key, &k, sizeof(int));
		memcpy(s2key + sizeof(int), &j, sizeof(int));

		unsigned char rets2key[sizeof(int) + sizeof(int)];

		if (SECONDARY2->get(CTX, s2key, &value, &length, rets2key, CT_DATASTORE_GET_EXACT) != 0) {
			printf("FAILED - s2 get %d.%d\n", k, j);
			exit(1);
		}

		//printf("s2 key: %d.%d value: %d %d %d\n", k, j, *((int*) (value + 1)), *((int*) (value + 1 + sizeof(int))), *((int*) (value + 1 + sizeof(int) + sizeof(int))));
	}

	free(value);

	CTX->commitTransaction();
}

int main(int argc, char** argv) {

	startup(true);

	testPut();
	testGet();

	shutdown();

	return 0;
}
