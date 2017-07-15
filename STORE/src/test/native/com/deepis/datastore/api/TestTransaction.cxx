#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cxx/lang/Thread.h"
#include "cxx/lang/System.h"
#include "cxx/lang/Runnable.h"

//#include "cxx/util/Logger.h"
#include "cxx/util/concurrent/atomic/AtomicInteger.h"

#ifdef DEEP_DISTRIBUTED
#include "com/deepis/db/store/relative/distributed/MapBehavior.h"
using namespace com::deepis::db::store::relative::distributed;
#endif

#include "com/deepis/datastore/api/DeepStore.h"

using namespace cxx::lang;
using namespace cxx::util;
using namespace cxx::util::concurrent::atomic;
using namespace com::deepis::datastore::api;

static int TRANSACTION_COUNT = 10000;

static int DATA_SIZE = sizeof(inttype);

static DeepStore* PRIMARY;

int put(DeepThreadContext* ctx, int key) {
	unsigned char* newvalue = (unsigned char*) malloc(DATA_SIZE);
	memcpy(newvalue, &key, sizeof(inttype));

	int status = PRIMARY->put(ctx, newvalue, DATA_SIZE);

	free(newvalue);

	return status;
}

int get(DeepThreadContext* ctx, int key) {
	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);

	unsigned char bkey[sizeof(int)];
	memcpy(bkey, &key, sizeof(int));

	unsigned char retkey[sizeof(int)];

	unsigned int length = DATA_SIZE;

	int status = PRIMARY->get(ctx, bkey, &value, &length, retkey, CT_DATASTORE_GET_EXACT);

	free(value);

	return status;
}

int remove(DeepThreadContext* ctx, int key) {
	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	memcpy(value, &key, sizeof(inttype));

	unsigned int length = DATA_SIZE;

	int status = PRIMARY->remove(ctx, value, length);

	free(value);

	return status;
}

int testMasterRollback(DeepThreadContext* ctx) {
	printf("TEST - Master rollback\n");

	int masterTrx = ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
	//ctx->associateTransaction(PRIMARY);

	int statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	inttype key = 1;

	if (put(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - put\n");
		exit(1);
	}

	ctx->commitTransaction(statementTrx);

	if (get(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Statement commit\n");
		exit(1);
	}

	ctx->rollbackTransaction(masterTrx);

	if (get(ctx, key) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Master rollback\n");
		exit(1);
	}

	printf("SUCCESS - Master rollback\n\n");

	return 0;
}

int testStatementRollback(DeepThreadContext* ctx) {
	printf("TEST - Statement rollback\n");

	int masterTrx = ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
	//ctx->associateTransaction(PRIMARY);

	int statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	inttype key = 1;

	if (put(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - put\n");
		exit(1);
	}

	ctx->rollbackTransaction(statementTrx);

	if (get(ctx, key) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Statement rollback\n");
		exit(1);
	}

	ctx->rollbackTransaction(masterTrx);

	if (get(ctx, key) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Master rollback\n");
		exit(1);
	}

	printf("SUCCESS - Statement rollback\n\n");

	return 0;
}

int testSavepointRollback(DeepThreadContext* ctx) {
	printf("TEST - Savepoint rollback\n");

	int masterTrx = ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
	//ctx->associateTransaction(PRIMARY);

	int statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	inttype key = 1;

	if (put(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - put\n");
		exit(1);
	}

	ctx->commitTransaction(statementTrx);

	if (get(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - 1. Statement commit\n");
		exit(1);
	}

	int savepointTrx = ctx->beginTransaction(DeepThreadContext::SAVEPOINT);
	//ctx->associateTransaction(PRIMARY);

	statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	key = 2;

	if (put(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - put\n");
		exit(1);
	}

	ctx->commitTransaction(statementTrx);

	if (get(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - 2. Statement commit\n");
		exit(1);
	}

	ctx->rollbackTransaction(savepointTrx);

	if (get(ctx, key) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Savepoint rollback\n");
		exit(1);
	}

	ctx->rollbackTransaction(masterTrx);

	if (get(ctx, key) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - Master rollback\n");
		exit(1);
	}

	printf("SUCCESS - Savepoint rollback\n\n");

	return 0;
}

int testRemove(DeepThreadContext* ctx, bool extraRemove) {
	printf("TEST - Remove (extra: %d)\n", extraRemove);

	inttype key = 1;

	int statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	if (put(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - 1st put\n");
		exit(1);
	}

	ctx->commitTransaction(statementTrx);

	statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	if (remove(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - 1st Remove\n");
		exit(1);
	}

	ctx->commitTransaction(statementTrx);

	statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	if (get(ctx, key) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - 1st Get succeeded, should have failed\n");
		exit(1);
	}

	ctx->commitTransaction(statementTrx);

	// EXTRA REMOVE CAUSES PROBLEM
	if (extraRemove == true) {
		statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
		ctx->associateTransaction(PRIMARY);

		remove(ctx, key);

		ctx->commitTransaction(statementTrx);
	}

	statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	if (get(ctx, key) == CT_DATASTORE_SUCCESS) {
		printf("FAILED - 2nd Get succeeded, should have failed\n");
		exit(1);
	}

	ctx->commitTransaction(statementTrx);

	statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	if (put(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - 2nd put\n");
		exit(1);
	}

	ctx->commitTransaction(statementTrx);

	statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);

	if (get(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - 3rd Get failed\n");
		exit(1);
	}

	// cleanup
	if (remove(ctx, key) != CT_DATASTORE_SUCCESS) {
		printf("FAILED - Cleanup Remove\n");
		exit(1);
	}

	ctx->commitTransaction(statementTrx);

	printf("SUCCESS - Remove\n\n");

	return 0;
}

int testLoadData(DeepThreadContext* ctx) {
	inttype numRows = TRANSACTION_COUNT * 5;

	printf("TEST - Exceed Transaction Count: %d, inserting: %d rows\n\n", TRANSACTION_COUNT, numRows);

	int statementTrx = ctx->beginTransaction(DeepThreadContext::STATEMENT);
	ctx->associateTransaction(PRIMARY);
	ctx->setLoadData(PRIMARY, true);

	for (int key=1; key<=numRows; key++) {
		if (put(ctx, key) != CT_DATASTORE_SUCCESS) {
			printf("FAILED - 1\n");
			exit(1);
		}
	}

	ctx->commitTransaction(statementTrx);

	printf("SUCCESS - Exceed Transaction Count\n\n");

	return 0;
}

void startup(DeepThreadContext* ctx, bool del) {

	DeepStore::setDebugEnabled(true);

	DeepStore::setFileSize(4294967295);
	DeepStore::setCacheSize(1073741824);
	DeepStore::setTransactionChunk(TRANSACTION_COUNT);

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	#ifdef DEEP_DISTRIBUTED
	PRIMARY = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, DISTRIBUTED_STANDALONE, true);
	#else
	PRIMARY = DeepStore::create("./datastore", options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE, true);
	#endif
	PRIMARY->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
	PRIMARY->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
	PRIMARY->initialize();

	PRIMARY->open(ctx);
	PRIMARY->recover(ctx, false);
}

void shutdown() {

	PRIMARY->close();
	delete PRIMARY;
}

int main(int argc, char** argv) {

	DeepThreadContext* ctx = new DeepThreadContext();

	startup(ctx,true);

	testMasterRollback(ctx);

	testStatementRollback(ctx);

	testSavepointRollback(ctx);

	testRemove(ctx, false);
	testRemove(ctx, true);

	testLoadData(ctx);

	delete ctx;

	shutdown();

	return 0;
}
