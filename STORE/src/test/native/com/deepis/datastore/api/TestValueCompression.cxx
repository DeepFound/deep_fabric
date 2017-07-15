#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cxx/lang/Thread.h"

#include "com/deepis/datastore/api/DeepStore.h"

using namespace cxx::lang;
using namespace com::deepis::datastore::api;

#if 1
// XXX: normal configuration
static bool TEST_DELETES = true;
static bool TEST_UPDATES = true;
static bool TEST_RECOVER = false;

static bool TEST_ONE_DELETE = false;
static bool TEST_LESS_ROWS = false;
static int CACHE_SIZE = 1000000; //1 MB
#else
// XXX: DATABASE-1171 configuration
static bool TEST_DELETES = true;
static bool TEST_UPDATES = false;
static bool TEST_RECOVER = false;

static bool TEST_ONE_DELETE = true;
static bool TEST_LESS_ROWS = true;
static int CACHE_SIZE = 100000; //1 MB
#endif

static int FILE_SIZE =  10000000; //10 MB
static int ROWS = 0;
static int* UPDATED_ROWS;
static int* DELETED_ROWS;

static int DATA_SIZE = (sizeof(int) + sizeof(int));

bool contains(int* arry, int size, int key) {
	for (int x = 0; x < size; x++) {
		if (arry[x] == key) return true;
	}
	return false;
}

static DeepStore* PRIMARY;
static DeepStore* SECONDARY1;

DeepThreadContext* CTX;

void startup(bool del) {

	DeepStore::setDebugEnabled(true);

	DeepStore::setFileSize(FILE_SIZE);
	DeepStore::setCacheSize(CACHE_SIZE);
	DeepStore::setInfinitelimit(false);
	DeepStore::setTransactionChunk(10000);

	int options = CT_DATASTORE_OPTION_CREATE | CT_DATASTORE_OPTION_CLEANUP | CT_DATASTORE_OPTION_MEMORY_COMPRESS | CT_DATASTORE_OPTION_DURABLE | CT_DATASTORE_OPTION_KEY_COMPRESS | CT_DATASTORE_OPTION_VALUE_COMPRESS;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	PRIMARY = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);
	PRIMARY->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	//to match secondary field
	PRIMARY->addField(CT_DATASTORE_LONG_INT /*type*/, 
			     CT_DATASTORE_LONG_INT /*real type*/, 
			     sizeof(int) /*pack length*/, 
			     0 /*row pack length*/, 
			     0 /*key length*/, 
			     0 /*length bytes*/, 
			     1 /*index*/, 
			     0 /*null bit*/, 
			     0 /*null offset*/, 
			     0 /*value offset*/, 
			     0 /*char set*/,
			     false /* virtual */);
	PRIMARY->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
	
	PRIMARY->initialize();

	SECONDARY1 = DeepStore::create("./s1.datastore", options, "SECONDARY1", CT_DATASTORE_COMPOSITE, sizeof(int) + sizeof(int), DATA_SIZE);
	//secondary field
	SECONDARY1->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	//primary suffix field
	SECONDARY1->addField(CT_DATASTORE_LONG_INT /*type*/, 
			     CT_DATASTORE_LONG_INT /*real type*/, 
			     sizeof(int) /*pack length*/, 
			     0 /*row pack length*/, 
			     0 /*key length*/, 
			     0 /*length bytes*/, 
			     1 /*index*/, 
			     0 /*null bit*/, 
			     0 /*null offset*/, 
			     0 /*value offset*/, 
			     0 /*char set*/,
			     false /* virtual */);
	//secondary key
	SECONDARY1->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), sizeof(int), 0, 0);
	//primary suffix
	SECONDARY1->addKeyPart(1u /*file index*/, 
                              CT_DATASTORE_LONG_INT /*type*/,
                              sizeof(int) /*length*/,
                              0 /*value offset*/,
                              0 /*null offset*/, 
                              0 /*null bit*/,
                             true /*is ignored*/,
			      false /*is reserved key*/,
			      -1 /*variable position*/,
			      0 /*primary position*/);

	SECONDARY1->initialize();

	PRIMARY->associate(SECONDARY1, false);
	CTX = new DeepThreadContext();

	printf("Opening Primary\n");
	bool success = (PRIMARY->open(CTX) == CT_DATASTORE_SUCCESS);

	if (success == true) {
		printf("Opening Secondary 1\n");
		success = (SECONDARY1->open(CTX) == CT_DATASTORE_SUCCESS);
	}

	if (TEST_RECOVER == true && success == false) {
		PRIMARY->recover(CTX, true);
	} else {
		PRIMARY->recover(CTX, false);
	}
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

void testPrint() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 0; i < ROWS; i++) {

		unsigned char key[sizeof(int)];
		memcpy(key, &i, sizeof(int));

		unsigned char retkey[sizeof(int) + sizeof(int)];
		unsigned int length = DATA_SIZE;

		if (PRIMARY->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Primary get %d (%d, %d)\n", i, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
			exit(1);
		}

		int pkey = 0;
		int skey = 0;

		memcpy(&pkey, value, sizeof(int));
		memcpy(&skey, value + sizeof(int), sizeof(int));

		unsigned char skeydata[sizeof(int)];
		memcpy(skeydata, &skey, sizeof(int));
		memcpy(skeydata + sizeof(int), &pkey, sizeof(int));

		if (SECONDARY1->get(CTX, skeydata, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != CT_DATASTORE_SUCCESS) {
			printf("PRIMARY Key %d Secondary %d contains %d\n", pkey, skey, ((TEST_UPDATES == true) && contains(UPDATED_ROWS, ROWS, pkey))?1:0);
			printf("Failed to get by secondary %d (%d, %d)\n", skey, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
		}
	}

	free(value);

	CTX->commitTransaction();
}

void printSecondary() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memset(value, 0, DATA_SIZE);

	unsigned char skeydata[sizeof(int)];
	memset(skeydata, 0, sizeof(int));

	unsigned char retkey[sizeof(int) + sizeof(int)];
	memset(retkey, 0, sizeof(int) + sizeof(int));
	unsigned int length = DATA_SIZE;

	int pkey = 0;
	int skey = 0;

	bool first = true;	
	while (true) {

		if (first == true) {

			if (SECONDARY1->get(CTX, skeydata, &value, &length, retkey, CT_DATASTORE_GET_FIRST) != CT_DATASTORE_SUCCESS) {
				break;
			}
			first = false;
		} else {
			memcpy(skeydata, &skey, sizeof(int));
			memcpy(skeydata + sizeof(int), &pkey, sizeof(int));

			if (SECONDARY1->get(CTX, skeydata, &value, &length, retkey, CT_DATASTORE_GET_NEXT) != CT_DATASTORE_SUCCESS) {
				break;
			}
		}
	
		memcpy(&pkey, value, sizeof(int));
		memcpy(&skey, value + sizeof(int), sizeof(int));

		printf("SECONDARY - PRIMARY Key %d Secondary %d contains %d\n", pkey, skey, ((TEST_UPDATES == true) && contains(UPDATED_ROWS, ROWS, pkey))?1:0);
	}	

	free(value);

	CTX->commitTransaction();
}

void testPut() {

	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	int rowBytes = /*key size*/ sizeof(int) + /*value size*/ DATA_SIZE;
	ROWS = 1 * (CACHE_SIZE / rowBytes);

	if (TEST_LESS_ROWS == true) {
		ROWS = ROWS / 4;
	}

	printf("Inserting %d rows...\n", ROWS);
	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 0; i < ROWS; i++) {

		memset(value, 0, DATA_SIZE);
		memcpy(value, &i, sizeof(int));
		memcpy(value + sizeof(int), &i, sizeof(int));

		if (i % 1000 == 0) {
			printf("Put at primary key: %d, value: %d %d \n", i, i, i);
			CTX->commitTransaction();
			CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
			CTX->associateTransaction(PRIMARY);
		}

		if (PRIMARY->put(CTX, value, DATA_SIZE) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Primary put on key %d\n", i);
			exit(1);
		}
	}
	free(value);

	CTX->commitTransaction();
}

void testUpdates() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	unsigned char* oldvalue = (unsigned char*) malloc(DATA_SIZE);
	unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);

	unsigned char retkey[sizeof(int) + sizeof(int)];
	unsigned int length = DATA_SIZE;
			

	UPDATED_ROWS = (int*) malloc(ROWS * sizeof(int));
	for (int x = 0; x < ROWS; x++) UPDATED_ROWS[x] = -1;

	printf("Randomly updating rows...\n");

	for (int y = 0; y < ROWS; y++) {
		int key = rand() % ROWS;
		UPDATED_ROWS[y] = key;
	}

	for (int i = 0; i < ROWS; i++) {

		if (i % 1000 == 0) {
			printf("Update number: %d\n", i);
			CTX->commitTransaction();
			CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
			CTX->associateTransaction(PRIMARY);
		}

		int oldk = UPDATED_ROWS[i];
		if (contains(UPDATED_ROWS, i, oldk) == true) {
			continue;
		}

		unsigned char oldkey[sizeof(int)];
		memcpy(oldkey, &oldk, sizeof(int));

		memset(oldvalue, 0, DATA_SIZE);

		if (PRIMARY->get(CTX, oldkey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Primary get for update %d (%d, %d)\n", oldk, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
			exit(1);
		}

		int newk = ROWS + oldk;
		memset(newvalue, 0, DATA_SIZE);
		memcpy(newvalue, &oldk, sizeof(int));
		memcpy(newvalue + sizeof(int), &newk, sizeof(int));

		if (PRIMARY->update(CTX, oldvalue, DATA_SIZE, newvalue, DATA_SIZE, false) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Primary update number %d on key %d (%d, %d)\n", i, oldk, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
			exit(1);
		}
	}

	free(oldvalue);
	free(newvalue);
	CTX->commitTransaction();
}

void testDelete() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	DELETED_ROWS = (int*) malloc(ROWS * sizeof(int));
	for (int x = 0; x < ROWS; x++) DELETED_ROWS[x] = -1;

	int start = 0;
	int end = ROWS;
	if (TEST_ONE_DELETE == true) {
		start = 1000;
		end = 1001;
	}

	int deletes = 0;
	for (int i = start; i < end; i += 10) {
		
		unsigned char key[sizeof(int)];
		memcpy(key, &i, sizeof(int));

		unsigned char retkey[sizeof(int) + sizeof(int)];
		unsigned int length = DATA_SIZE;

		if (deletes % 1000 == 0) {
			printf("Deleting keys which are multiples of 10: %d\n", deletes);
			CTX->commitTransaction();
			CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
			CTX->associateTransaction(PRIMARY);
		}
		deletes++;

		DELETED_ROWS[i] = i;		

		if (PRIMARY->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Primary get for delete %d (%d, %d)\n", i, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
			exit(1);
		}

		if (PRIMARY->remove(CTX, value, length, false) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Primary delete on key %d (%d, %d)", i, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
			exit(1);
		}
	}

	free(value);

	CTX->commitTransaction();
}

void testGetPrimary() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 0; i < ROWS; i++) {

		unsigned char key[sizeof(int)];
		memcpy(key, &i, sizeof(int));

		unsigned char retkey[sizeof(int) + sizeof(int)];
		unsigned int length = DATA_SIZE;

		if (i % 1000 == 0) {
			printf("Verify at primary key: %d\n", i);
		}

		int result = PRIMARY->get(CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != CT_DATASTORE_SUCCESS;

		if ((TEST_DELETES == false && result != CT_DATASTORE_SUCCESS)  || (TEST_DELETES == true && contains(DELETED_ROWS, ROWS, i) == false && result != CT_DATASTORE_SUCCESS)) {
			printf("FAILED - Primary get %d (%d, %d)\n", i, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
			exit(1);

		} else if (TEST_DELETES == true && contains(DELETED_ROWS, ROWS, i) == true && result == CT_DATASTORE_SUCCESS) {
			int retVal;
			memcpy(&retVal, value + sizeof(int), sizeof(int));
			printf("FAILED - Primary get succeeded on deleted key %d (%d) (%d, %d)\n", i, retVal, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
			exit(1);
		}

		if ((TEST_DELETES == true) && (i % 10 == 0)) {
			continue;
		}

		int retVal;
		memcpy(&retVal, value + sizeof(int), sizeof(int));

		bool valid = false;
		bool updated = ((TEST_UPDATES == true) && contains(UPDATED_ROWS, ROWS, i));

		int expected = 0;
		if (updated) {
			expected = i + ROWS;
		} else {
			expected = i;
		}

		if (retVal == expected) {
			valid = true;
		}

		if (valid == false) {
			printf("FAILED - Primary get %d value was %d expected %d\n", i, retVal, expected);
			exit(1);
		}	
	}

	free(value);

	CTX->commitTransaction();
}

void testGetSecondary() {
	CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	CTX->associateTransaction(PRIMARY);

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	for (int i = 0; i < ROWS; i++) {

		unsigned int length = DATA_SIZE;
		memset(value, 0, length);

		int k = i;
		if ((TEST_UPDATES == true) && contains(UPDATED_ROWS, ROWS, i)) {
			k += ROWS;
		}

		unsigned char s1key[sizeof(int)];
		memset(s1key, 0, sizeof(int));
		memcpy(s1key, &k, sizeof(int));

		unsigned char rets1key[sizeof(int) + sizeof(int)];

		if (i % 1000 == 0) {
			printf("Verify at secondary key: %d\n", i);
		}

		int result = SECONDARY1->get(CTX, s1key, &value, &length, rets1key, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_NONE);
		if ((TEST_DELETES == false && result != CT_DATASTORE_SUCCESS)  || (TEST_DELETES == true && contains(DELETED_ROWS, ROWS, i) == false && result != CT_DATASTORE_SUCCESS)) {
			printf("FAILED - Secondary get %d (%d, %d)\n", k, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
			//exit(1);

		} else if (TEST_DELETES == true && contains(DELETED_ROWS, ROWS, i) == true && result == CT_DATASTORE_SUCCESS) {
			printf("FAILED - Secondary get succeeded on deleted key %d (%d, %d)\n", i, PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
			//exit(1);
		}
	}

	free(value);

	CTX->commitTransaction();
}

int main(int argc, char** argv) {

	startup(true);

	testPut();
	
	if (TEST_UPDATES == true) {
		testUpdates();
	}

	if (TEST_DELETES == true) {
		testDelete();
	}

	testGetPrimary();
	testGetSecondary();
//	testPrint();
//	printSecondary();

	Thread::sleep(5000);
	shutdown();
	printf("Shutting down...\n");
	Thread::sleep(5000);

	if (TEST_RECOVER == true) {
		printf("Testing recovery...\n");
		printf("Deleting IRT files...\n");
		system("rm *.irt");
	}

	startup(false);

	printf("Starting up...\n");
	Thread::sleep(5000);

	testGetPrimary();
	testGetSecondary();

	printf("Waiting 10s to shutdown");
	Thread::sleep(10000);
	
	PRIMARY->shutdownCacheManagement();
	PRIMARY->shutdownGarbageCollection(true);

//	shutdown();

	if (TEST_UPDATES == true) {
		free(UPDATED_ROWS);
	}

	if (TEST_DELETES == true) {
		free(DELETED_ROWS);
	}
	
	return 0;
}
