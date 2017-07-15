#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com/deepis/datastore/api/DeepStore.h"

using namespace com::deepis::datastore::api;

static int MESSAGE_SIZE = 16;
static int ID_SIZE = sizeof(int);
static int FILL_SIZE = sizeof(int);

static int DATA_SIZE = MESSAGE_SIZE + ID_SIZE + FILL_SIZE;
static int KEY_SIZE = MESSAGE_SIZE + ID_SIZE;

static DeepStore* PRIMARY;
static DeepThreadContext* CTX;

void startup(bool del) {

	DeepStore::setDebugEnabled(true);

	DeepStore::setFileSize(4294967295);
	DeepStore::setCacheSize(1073741824);
	DeepStore::setTransactionChunk(10000);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	CTX = new DeepThreadContext();

	PRIMARY = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_COMPOSITE, KEY_SIZE, DATA_SIZE);
	PRIMARY->addField(CT_DATASTORE_TEXT, CT_DATASTORE_TEXT, MESSAGE_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY->addField(CT_DATASTORE_TEXT, CT_DATASTORE_TEXT, ID_SIZE, 0, 0, 0, 1, 0, 0, 0, 0, false);
	PRIMARY->addKeyPart(0u, CT_DATASTORE_TEXT, MESSAGE_SIZE, 0, 0, 0);
	PRIMARY->addKeyPart(1u, CT_DATASTORE_LONG_INT, ID_SIZE, 0 + MESSAGE_SIZE, 0, 0);
	PRIMARY->initialize();

	printf("Opening Primary\n");
	PRIMARY->open(CTX);
	PRIMARY->recover(CTX, false);
}

void shutdown() {

	printf("Closing Primary\n");

	PRIMARY->close();

	delete PRIMARY;
	delete CTX;
}

/*
insert into tweets values (1, 10, 100, 'one');
insert into tweets values (2, 20, 200, 'two');
insert into tweets values (3, 30, 300, 'three');
insert into tweets values (4, 10, 100, 'one');
insert into tweets values (22, 220, 200, 'two');
insert into tweets values (44, 10, 400, 'four');
 */

int IDS[] = {1, 2, 3, 4, 22, 44};
const char* MESSAGES[] = {"one", "two", "three", "one", "two", "four"};
int FILL[] = {10, 20, 30, 40, 50, 60};
int UPDATE_FILL[] = {100, 200, 300, 400, 500, 600};
int COUNT = 6;

void testPut() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	for (int i = 0; i < COUNT; i++) {
		int id = IDS[i];
		int fill = FILL[i];
		const char* message = MESSAGES[i];

		unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
		memset(value, 0, DATA_SIZE);

		memcpy(value, message, strlen(message));
		memcpy(value + MESSAGE_SIZE, &id, ID_SIZE);
		memcpy(value + MESSAGE_SIZE + ID_SIZE, &fill, FILL_SIZE);

		printf("Put: %s.%d\n", message, id);

		if (PRIMARY->put(CTX, value, DATA_SIZE) != 0) {
			free(value);
			printf("FAILED - Putd %s.%d\n", message, id);
			exit(0);
		}

		free(value);
	}

	CTX->commitTransaction();
}

void testGet() {
	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	unsigned int length = DATA_SIZE;

	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	for (int i = 0; i < COUNT; i++) {
		int id = IDS[i];
		const char* message = MESSAGES[i];

		unsigned char* key = (unsigned char*) malloc(KEY_SIZE);
		memset(key, 0, KEY_SIZE);

		memcpy(key, message, strlen(message));
		memcpy(key + MESSAGE_SIZE, &id, ID_SIZE);

		unsigned char* retkey = (unsigned char*) malloc(KEY_SIZE);

		printf("Get: %s.%d\n", message, id);

		if (PRIMARY->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != 0) {
			free(key);
			free(retkey);
			printf("FAILED - Get %s.%d\n", message, id);
			exit(0);
		}

		printf("  Value: %s.%d\n", value, *((int*) (value + MESSAGE_SIZE)));

		free(key);
		free(retkey);
	}

	CTX->commitTransaction();

	free(value);
}

void testUpdate(bool statement) {
	unsigned char* ovalue = (unsigned char*) malloc(DATA_SIZE);
	unsigned char* nvalue = (unsigned char*) malloc(DATA_SIZE);
	unsigned int length = DATA_SIZE;

	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);

	if (statement == true) {
		CTX->beginTransaction(DeepThreadContext::STATEMENT);
	}

	CTX->associateTransaction(PRIMARY);

	for (int i = 0; i < COUNT; i++) {
		int id = IDS[i];
		int update_fill = UPDATE_FILL[i];
		const char* message = MESSAGES[i];

		unsigned char* key = (unsigned char*) malloc(KEY_SIZE);
		memset(key, 0, KEY_SIZE);

		memcpy(key, message, strlen(message));
		memcpy(key + MESSAGE_SIZE, &id, ID_SIZE);

		unsigned char* retkey = (unsigned char*) malloc(KEY_SIZE);

		printf("Update (get with lock): %s.%d\n", message, id);

		if (PRIMARY->get(CTX, key, &ovalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE) != 0) {
			free(key);
			free(retkey);
			printf("FAILED - Get %s.%d\n", message, id);
			exit(0);
		}

		printf("  Old Value: %s.%d.%d\n", ovalue, *((int*) (ovalue + MESSAGE_SIZE)), *((int*) (ovalue + MESSAGE_SIZE + ID_SIZE)));

		memcpy(nvalue, ovalue, DATA_SIZE);

		memcpy(nvalue + MESSAGE_SIZE + ID_SIZE, &update_fill, FILL_SIZE);

		if (PRIMARY->update(CTX, ovalue, DATA_SIZE, nvalue, DATA_SIZE, false /*already locked*/) != CT_DATASTORE_SUCCESS) {
			free(key);
			free(retkey);
			printf("FAILED - Update failed using oldvalue/newvalue\n\n");
			exit(1);
		}

		printf("  New Value: %s.%d.%d\n", nvalue, *((int*) (nvalue + MESSAGE_SIZE)), *((int*) (nvalue + MESSAGE_SIZE + ID_SIZE)));

		free(key);
		free(retkey);
	}

	CTX->commitTransaction();

	free(ovalue);
	free(nvalue);
}

void testNext() {
	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	unsigned int length = DATA_SIZE;

	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	for (int i = 0; i < COUNT; i++) {
		const char* message = MESSAGES[i];

		unsigned char* key = (unsigned char*) malloc(KEY_SIZE);
		memset(key, 0, KEY_SIZE);

		memcpy(key, message, strlen(message));

		unsigned char* retkey = (unsigned char*) malloc(KEY_SIZE);

		printf("Next: %s\n", message);

		// use exact here to simulate mysql partial query
		if (PRIMARY->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_NONE, false, 1) != 0) {
			free(key);
			free(retkey);
			printf("FAILED - Next %s\n", message);
			exit(0);
		}

		printf("  Value: %s.%d\n", value, *((int*) (value + MESSAGE_SIZE)));

		free(key);
		free(retkey);
	}

	CTX->commitTransaction();

	free(value);
}

int main(int argc, char** argv) {

	startup(true);

	testPut();

	testGet();

	testUpdate(false /*no statement*/);
	testUpdate(true /*statement*/);

	testNext();

	shutdown();

	startup(false);

	testGet();

	shutdown();

	return 0;
}
