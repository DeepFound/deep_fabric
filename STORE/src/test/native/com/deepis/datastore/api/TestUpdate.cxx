#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "com/deepis/datastore/api/DeepStore.h"

using namespace com::deepis::datastore::api;

static int DATA_SIZE = (2 * sizeof(int));

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

	PRIMARY->open(CTX);

	SECONDARY1->open(CTX);
	PRIMARY->recover(CTX, false);
}

void shutdown() {

	PRIMARY->close();
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

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memset(value, 0, DATA_SIZE);
	memcpy(value, &i, sizeof(int));
	memcpy(value + sizeof(int), &k, sizeof(int));

	PRIMARY->put(CTX, value, DATA_SIZE);


	int i2 = 2;
	int k2 = 1000;

	unsigned char* value2 = (unsigned char*) malloc(DATA_SIZE);
	memset(value2, 0, DATA_SIZE);
	memcpy(value2, &i2, sizeof(int));
	memcpy(value2 + sizeof(int), &k2, sizeof(int));

	PRIMARY->put(CTX, value2, DATA_SIZE);

	CTX->commitTransaction();

	free(value);
	free(value2);
}

int testUpdateSecondaryKey(bool isTx, int oldSecondaryKey) {
	printf("\n\nTesting update of unique secondary ( isTx = %d, secondary = %d)\n\n", isTx, oldSecondaryKey);

	if (isTx == true) {
		CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
		CTX->associateTransaction(PRIMARY);
	}

	unsigned int length = DATA_SIZE;
	unsigned char* retkey = (unsigned char*) malloc(sizeof(int));
	unsigned char* retvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(retvalue, 0, DATA_SIZE);

	unsigned char* oldvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(oldvalue, 1, DATA_SIZE);

	unsigned char bOldSecondaryKey[sizeof(int)];
	memcpy(bOldSecondaryKey, &oldSecondaryKey, sizeof(int));

	if (SECONDARY1->get(CTX, bOldSecondaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) != CT_DATASTORE_SUCCESS)  {
		printf("FAILED - Secondary get failed before update\n\n");
		exit(1);
	}

	printf("OK - Got old value for update with primary: %d secondary: %d\n", *((int*)oldvalue), *((int*)(oldvalue + sizeof(int))));

	// make sure secondary is different for update
	int newSecondaryKey = *((int*)retkey) + 1;

	unsigned char bNewSecondaryKey[sizeof(int)];
	memcpy(bNewSecondaryKey, &newSecondaryKey, sizeof(int));

	unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
	memcpy(newvalue, oldvalue, DATA_SIZE);
	memcpy(newvalue + sizeof(int), &newSecondaryKey, sizeof(int));

	printf("ATTEMPT - Update value (should change secondary key from: %d to: %d)\n", oldSecondaryKey, newSecondaryKey);

	if (PRIMARY->update(CTX, oldvalue, DATA_SIZE, newvalue, DATA_SIZE, !isTx) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Update failed using oldvalue/newvalue\n\n");
		exit(1);
	}

	if (isTx == true) {
		CTX->commitTransaction();
	}

	printf("OK - Update succeeded, verifying...\n");

	if (SECONDARY1->get(CTX, bOldSecondaryKey, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Secondary get succeeded after update, old key still present: %d\n\n", oldSecondaryKey);
		exit(1);
	}

	printf("OK - Old secondary removed : %d\n", oldSecondaryKey);

	if (SECONDARY1->get(CTX, bNewSecondaryKey, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Secondary get failed after update, new key not present: %d\n\n", newSecondaryKey);
		exit(1);
	}

	printf("OK - New secondary added: %d\n", newSecondaryKey);

	int returnKey = *((int*)retkey);

	free(oldvalue);
	free(newvalue);
	free(retvalue);
	free(retkey);

	return returnKey;
}

void testRollbackUpdateSecondaryKey() {
	printf("\n\nTesting rollback of secondary key update\n\n");

	// Master Begin (Level 0)
	int masterLevel = CTX->beginTransaction(DeepThreadContext::SAVEPOINT);

	// Statement Begin (Level 0 to 1)
	int statementLevel = CTX->beginTransaction(DeepThreadContext::STATEMENT);
	CTX->associateTransaction(PRIMARY);

	int i = 5;
	int k = 50;

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memset(value, 0, DATA_SIZE);
	memcpy(value, &i, sizeof(int));
	memcpy(value + sizeof(int), &k, sizeof(int));

	if (PRIMARY->put(CTX, value, DATA_SIZE) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Primary put\n\n");
		exit(1);
	}

	printf("OK - secondary key is %d\n", k);

	// Statement Commit (Level 1 to 0)
	CTX->commitTransaction(statementLevel);

	// Savepoint Begin (Level 0 to 1)
	int savepointLevel = CTX->beginTransaction(DeepThreadContext::SAVEPOINT);

	printf("OK - savepoint begin\n");

	// Statement Begin (Level 1 to 2)
	statementLevel = CTX->beginTransaction(DeepThreadContext::STATEMENT);
	CTX->associateTransaction(PRIMARY);

	int k2 = 500;

	unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(newvalue, 0, DATA_SIZE);
	memcpy(newvalue, &i, sizeof(int));
	memcpy(newvalue + sizeof(int), &k2, sizeof(int));

	if (PRIMARY->update(CTX, value, DATA_SIZE, newvalue, DATA_SIZE) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Update failed \n\n");
		exit(1);
	}

	printf("OK - secondary key updated to %d\n", k2);

	// Statement Commit (Level 2 to 1)
	CTX->commitTransaction(statementLevel);

	// Savepoint Rollback (Level 1 to 0)
	CTX->rollbackTransaction(savepointLevel);

	printf("OK - savepoint rolled back\n");

	unsigned char primaryKey[sizeof(int)];
	memcpy(primaryKey, &i, sizeof(int));

	unsigned int length = DATA_SIZE;
	unsigned char* retkey = (unsigned char*) malloc(sizeof(int));
	unsigned char* retvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(retvalue, 0, DATA_SIZE);

	if (PRIMARY->get(CTX, primaryKey, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Primary get failed: %d\n\n", i);
		exit(1);
	}

	printf("OK - verifying that secondary key was rolled back to %d\n", k);

	int secondaryKey = *((int*) (retvalue + sizeof(int)));

	if (secondaryKey != k) {
		printf("FAILED - Secondary key value is incorrect: %d\n\n", secondaryKey);
		exit(1);
	}

	printf("OK - secondary key was rolled back to %d\n", k);

	// MasterLevel Rollback (Level 0 to -1)
	CTX->rollbackTransaction(masterLevel);

	free(value);
	free(newvalue);
	free(retvalue);
	free(retkey);
}

int testOnDuplicateSecondaryUpdate(bool isTx, int oldSecondaryKey) {
	printf("\n\nTesting 'on duplicate' update of unique secondary ( isTx = %d, secondary : %d )\n\n", isTx, oldSecondaryKey);

	if (isTx == true) {
		CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
		CTX->associateTransaction(PRIMARY);
	}

	// create a unique primary key, since we want the dup to occur on the secondary
	int newPrimaryKey = oldSecondaryKey * 10;

	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memset(value, 0, DATA_SIZE);
	memcpy(value, &newPrimaryKey, sizeof(int));
	memcpy(value + sizeof(int), &oldSecondaryKey, sizeof(int));

	unsigned char bNewPrimaryKey[sizeof(int)];
	memcpy(bNewPrimaryKey, &newPrimaryKey, sizeof(int));

	unsigned char bOldSecondaryKey[sizeof(int)];
	memcpy(bOldSecondaryKey, &oldSecondaryKey, sizeof(int));

	unsigned int length = DATA_SIZE;
	unsigned char* retkey = (unsigned char*) malloc(sizeof(int));
	unsigned char* retvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(retvalue, 0, DATA_SIZE);

	printf("ATTEMPT - Put (should cause dup on secondary with primary: %d secondary: %d)\n", newPrimaryKey, oldSecondaryKey);

	int status = PRIMARY->put(CTX, value, DATA_SIZE);

	if (status == CT_DATASTORE_SUCCESS) {
		printf("FAILED - put succeeded should have failed due to secondary unique constraint\n\n");
		exit(1);

	} else {
		int errorCode = SECONDARY1->getErrorCode();

		if (errorCode == CT_DATASTORE_ERROR_DUP_KEY) {
			printf("OK - got DUPLICATE error code on secondary datastore\n");
			if (isTx == true) {
				//
				// Deep does not rollback here !!!
				//
				// CTX->rollbackTransaction();
			}

			// make sure the put did not succeed on new primary
			if (PRIMARY->get(CTX, bNewPrimaryKey, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT) == CT_DATASTORE_SUCCESS) {
				printf("FAILED - Primary get succeeded after failed put, key was added: %d\n\n", newPrimaryKey);
				exit(1);
			}

		} else {
			printf("FAILED - expected DUPLICATE error code on secondary datastores, error: %d\n\n", errorCode);
			exit(1);
		}
	}

	unsigned char* oldvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(oldvalue, 1, DATA_SIZE);

	// get with lock (in case of transaction) using the old secondary
	if (SECONDARY1->get(CTX, bOldSecondaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Secondary get failed before update\n\n");
		exit(1);
	}

	printf("OK - Got old value for update with primary: %d secondary: %d\n", *((int*)oldvalue), *((int*)(oldvalue + sizeof(int))));

	// make sure secondary is different for update
	int newSecondaryKey = *((int*)retkey) + 1;

	unsigned char bNewSecondaryKey[sizeof(int)];
	memcpy(bNewSecondaryKey, &newSecondaryKey, sizeof(int));

	unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
	memcpy(newvalue, oldvalue, DATA_SIZE);
	memcpy(newvalue + sizeof(int), &newSecondaryKey, sizeof(int));

	printf("ATTEMPT - Update value (should change secondary key from: %d to: %d)\n", oldSecondaryKey, newSecondaryKey);

	if (PRIMARY->update(CTX, oldvalue, DATA_SIZE, newvalue, DATA_SIZE, !isTx) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Update failed, PRIMARY error: %d, SECONDARY error: %d\n\n", PRIMARY->getErrorCode(), SECONDARY1->getErrorCode());
		exit(1);
	}

	printf("OK - Update succeeded, verifying...\n");

	if (isTx == true) {
		CTX->commitTransaction();
	}

	if (SECONDARY1->get(CTX, bOldSecondaryKey, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Secondary get succeeded after update, old secondary key still present: %d\n\n", oldSecondaryKey);
		exit(1);
	}

	printf("OK - Old secondary removed: %d\n", oldSecondaryKey);

	if (SECONDARY1->get(CTX, bNewSecondaryKey, &retvalue, &length, retkey, CT_DATASTORE_GET_EXACT) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Secondary get failed after update, new key not present: %d\n\n", newSecondaryKey);
		exit(1);
	}

	printf("OK - New secondary added: %d\n", newSecondaryKey);

	int returnKey = *((int*)retkey);

	free(oldvalue);
	free(newvalue);
	free(retvalue);
	free(retkey);
	free(value);

	return returnKey;
}


void testUpdateSecondaryKeyDuplicate(bool isTx, int primaryKey, int oldSecondaryKey, bool lockPrimary) {
	printf("\n\nTesting update of unique secondary, causing duplicate ( isTx = %d, primary = %d, secondary = %d, lockPrimary = %d)\n\n", isTx, primaryKey, oldSecondaryKey, lockPrimary);

	if (isTx == true) {
		CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
		CTX->associateTransaction(PRIMARY);
	}

	unsigned int length = DATA_SIZE;
	unsigned char* retkey = (unsigned char*) malloc(sizeof(int));

	unsigned char* oldvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(oldvalue, 0, DATA_SIZE);

	if (lockPrimary == true) {
		unsigned char bPrimaryKey[sizeof(int)];
		memcpy(bPrimaryKey, &primaryKey, sizeof(int));

		printf("ATTEMPT - Get on primary for update (with primary: %d)\n", primaryKey);

		if (PRIMARY->get(CTX, bPrimaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Primary get failed before update\n\n");
			exit(1);
		}

	} else {
		unsigned char bOldSecondaryKey[sizeof(int)];
		memcpy(bOldSecondaryKey, &oldSecondaryKey, sizeof(int));

		printf("ATTEMPT - Get on secondary for update (with secondary: %d)\n", oldSecondaryKey);

		if (SECONDARY1->get(CTX, bOldSecondaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Secondary get failed before update\n\n");
			exit(1);
		}
	}

	printf("OK - Got old value for update with primary: %d secondary: %d\n", *((int*)oldvalue), *((int*)(oldvalue + sizeof(int))));

	// create a secondary dup scenario
	int dupSecondaryKey = 1000;

	unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
	memcpy(newvalue, oldvalue, DATA_SIZE);
	memcpy(newvalue + sizeof(int), &dupSecondaryKey, sizeof(int));

	printf("ATTEMPT - Put (should cause dup on secondary with primary: %d secondary: %d)\n", *((int*)newvalue), *((int*)(newvalue + sizeof(int))));

	int status = PRIMARY->update(CTX, oldvalue, DATA_SIZE, newvalue, DATA_SIZE, !isTx);

	if (status == CT_DATASTORE_SUCCESS) {
		printf("FAILED - put succeeded should have failed due to secondary unique constraint\n\n");
		exit(1);

	} else {
		int errorCode = SECONDARY1->getErrorCode();

		if (errorCode == CT_DATASTORE_ERROR_DUP_KEY) {
			printf("OK - got DUPLICATE error code on secondary datastore\n");
			if (isTx == true) {
				CTX->rollbackTransaction();
			}

		} else {
			printf("FAILED - expected DUPLICATE error code on secondary datastore, error: %d\n\n", errorCode);
			exit(1);
		}
	}

	free(oldvalue);
	free(newvalue);
	free(retkey);
}

int testUpdatePrimaryKeyInSubTransaction(bool isTx, int primaryKey, int oldSecondaryKey, bool lockPrimary) {
	printf("\n\nTesting update of primary, in sub-transaction ( isTx = %d, primary = %d, secondary = %d, lockPrimary = %d)\n\n", isTx, primaryKey, oldSecondaryKey, lockPrimary);

	if (isTx == true) {
		CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
		CTX->associateTransaction(PRIMARY);
	}

	unsigned int length = DATA_SIZE;
	unsigned char* retkey = (unsigned char*) malloc(sizeof(int));

	unsigned char* oldvalue = (unsigned char*) malloc(DATA_SIZE);
	memset(oldvalue, 0, DATA_SIZE);

	unsigned char bPrimaryKey[sizeof(int)];
	memcpy(bPrimaryKey, &primaryKey, sizeof(int));

	unsigned char bOldSecondaryKey[sizeof(int)];
	memcpy(bOldSecondaryKey, &oldSecondaryKey, sizeof(int));

	if (lockPrimary == true) {
		printf("ATTEMPT - Get on primary for update (with primary: %d)\n", primaryKey);

		if (PRIMARY->get(CTX, bPrimaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Primary get failed before update\n\n");
			exit(1);
		}

	} else {
		printf("ATTEMPT - Get on secondary for update (with secondary: %d)\n", oldSecondaryKey);

		if (SECONDARY1->get(CTX, bOldSecondaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - Secondary get failed before update\n\n");
			exit(1);
		}
	}

	printf("OK - Got old value for update with primary: %d secondary: %d\n", *((int*)oldvalue), *((int*)(oldvalue + sizeof(int))));

	// create a secondary dup scenario
	int newPrimaryKey = primaryKey * 10;

	unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
	memcpy(newvalue, oldvalue, DATA_SIZE);
	memcpy(newvalue, &newPrimaryKey, sizeof(int));

	printf("ATTEMPT - Update in sub-transaction (should cause add/remove during update with primary: %d secondary: %d)\n", *((int*)newvalue), *((int*)(newvalue + sizeof(int))));

	// sub-transaction
	CTX->beginTransaction(DeepThreadContext::STATEMENT);
	CTX->associateTransaction(PRIMARY);

	int status = PRIMARY->update(CTX, oldvalue, DATA_SIZE, newvalue, DATA_SIZE, !isTx);

	if (status != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Update failed\n\n");
		exit(1);
	}

	printf("OK - Update succeeded\n");

	printf("ATTEMPT - Get on primary before sub-transaction commit (verify that old primary key is gone with primary: %d)\n", primaryKey);

	if (PRIMARY->get(CTX, bPrimaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Primary get succeeded, should have failed because old primary was removed in update\n\n");
		exit(1);
	}

	printf("OK - Old primary is gone before sub-transaction commit: %d\n", primaryKey);

	// commit sub-transaction
	CTX->commitTransaction(CTX->getTransactionLevel());

	printf("ATTEMPT - Get on primary after sub-transaction commit (verify that old primary key is gone with primary: %d)\n", primaryKey);

	if (PRIMARY->get(CTX, bPrimaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Primary get succeeded, should have failed because old primary was removed in update\n\n");
		exit(1);
	}

	printf("OK - Old primary is gone after sub-transaction commit: %d\n", primaryKey);

	// commit master transaction
	if (isTx == true) {
		CTX->commitTransaction();
	}

	printf("ATTEMPT - Get on primary after master transaction commit (verify that old primary key is gone with primary: %d)\n", primaryKey);

	if (PRIMARY->get(CTX, bPrimaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Primary get succeeded, should have failed because old primary was removed in update\n\n");
		exit(1);
	}

	printf("OK - Old primary is gone after master transaction commit: %d\n", primaryKey);

	printf("ATTEMPT - Get on old secondary (with secondary: %d)\n", oldSecondaryKey);

	if (SECONDARY1->get(CTX, bOldSecondaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Secondary get failed after update\n\n");
		exit(1);
	}

	printf("OK - Old secondary is still present: %d\n", newPrimaryKey);

	unsigned char bNewPrimaryKey[sizeof(int)];
	memcpy(bNewPrimaryKey, &newPrimaryKey, sizeof(int));

	printf("ATTEMPT - Get on primary (verify that new primary key is present with primary: %d)\n", newPrimaryKey);

	if (PRIMARY->get(CTX, bNewPrimaryKey, &oldvalue, &length, retkey, CT_DATASTORE_GET_EXACT, ((isTx == true) ? CT_DATASTORE_LOCK_WRITE : CT_DATASTORE_LOCK_NONE)) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Primary get failed, should succeeded because new key was added in update\n\n");
		exit(1);
	}

	printf("OK - New primary is present: %d\n", newPrimaryKey);

	free(oldvalue);
	free(newvalue);
	free(retkey);

	return newPrimaryKey;
}

int main(int argc, char** argv) {

	startup(true);

	setup();

	int primaryKey = 1;
	int secondaryKey = 10;

	// normal update of secondary key
	secondaryKey = testUpdateSecondaryKey(true, secondaryKey);

	// Deep 'like' on-duplicate update
	secondaryKey = testOnDuplicateSecondaryUpdate(true, secondaryKey);

	// update secondary key causing duplicate, TX
	testUpdateSecondaryKeyDuplicate(true, primaryKey, secondaryKey, true);
	testUpdateSecondaryKeyDuplicate(true, primaryKey, secondaryKey, false);

	// update primary key in sub transaction
	/* XXX: DATABASE-597
	primaryKey = testUpdatePrimaryKeyInSubTransaction(true, primaryKey, secondaryKey, true);
	primaryKey = testUpdatePrimaryKeyInSubTransaction(true, primaryKey, secondaryKey, false);
	*/

	testRollbackUpdateSecondaryKey();

	shutdown();

	return 0;
}
