#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

#include "cxx/lang/Thread.h"
#include "cxx/lang/System.h"
#include "cxx/lang/Runnable.h"

#include "com/deepis/db/store/relative/core/Properties.h"
#include "com/deepis/datastore/api/DeepStore.h"

using namespace cxx::lang;
using namespace com::deepis::datastore::api;
using namespace com::deepis::db::store::relative::core;

const int MAX_TABLES = 100;
const int MAX_TRIALS = 10;

const int DATA_SIZE = (2 * sizeof(int));
unsigned char KEY[sizeof(int)];
DeepStore* TABLES[MAX_TABLES];

DeepThreadContext* CTX;
pid_t childPid;

void runTrials();
void verify();
void runTransactions();
void associate(DeepThreadContext* ctx);
void startup(boolean initial);
void shutdown();

int main(int argc, char** argv) {
	Properties::setTrtFileSize(100000000);
	runTrials();

	return 0;
}

void runTrials() {
	for (int x = 0; x < MAX_TRIALS; x++) {
		printf("Starting trial %d of %d\n", x + 1, MAX_TRIALS);

		// child process will run the transactions
		childPid = fork();

		if (childPid > 0) {
			printf("Child pid %d starting...\n", childPid);

			// wait 10 - 15 seconds
			longtype waitTime = 10000 + (rand() % 5000);
			Thread::sleep(waitTime);

			printf("Killing child with pid %d\n", childPid);

			kill(childPid, SIGKILL);

			Thread::sleep(3000);

			printf("Recovering %d tables \n", MAX_TABLES);

			startup(false /* initial */);
			
			verify();

			shutdown();

		} else if (childPid == 0) {
			runTransactions();

		} else {
			printf("FAILED - fork failed\n");
			exit(1);
		}
	}
}

void verify() {
	long long count = TABLES[0]->size(CTX);
	printf("VERIFY - verify tables sizes %lld\n", count);

	for (int x = 1; x < MAX_TABLES; x++) {
		DeepStore* table = TABLES[x];
		long long count2 = table->size(CTX);
		
		if (count != count2) {
			printf("VERIFY FAILED - cross table size mismatch after recovery (%lld != %lld)\n", count, count2);
			exit(1);
		} else {
			printf("VERIFY MATCH - table %d count %lld\n", x, count2);
		}
	}
}

void runTransactions() {
	startup(true /* initial */);
	unsigned char* value = (unsigned char*) malloc(DATA_SIZE);
	
	DeepThreadContext* ctx = new DeepThreadContext();

	printf("Child running transactions...\n");

	// process to be killed from main
	for (int key = 0; true; key++) {

		memset(value, 0, DATA_SIZE);
		memcpy(value, &key, sizeof(int));
		memcpy(value + sizeof(int), &key, sizeof(int));

		ctx->beginTransaction();
		associate(ctx);

		for (int x = 0; x < MAX_TABLES; x++) {
			DeepStore* table = TABLES[x];

			if (table->put(ctx, value, DATA_SIZE) != CT_DATASTORE_SUCCESS) {
				printf("FAILED - put %d - error code: %d\n", key, table->getErrorCode());
				exit(1);
			}
		}

		ctx->commitTransaction();
	}

	free(value);
}

void associate(DeepThreadContext* ctx) {
	for (int x = 0; x < MAX_TABLES; x++) {
		DeepStore* table = TABLES[x];
		ctx->associateTransaction(table);
	}
}

void startup(boolean initial) {
	DeepStore::setDebugEnabled(true);
	int options = CT_DATASTORE_OPTION_CREATE | CT_DATASTORE_STATIC_CONTEXT;

	if (initial == true) {
		printf("Creating %d tables...\n", MAX_TABLES);
		options |= CT_DATASTORE_OPTION_DELETE;
	} else {
		printf("Reopening %d tables...\n", MAX_TABLES);
	}

	CTX = new DeepThreadContext();
	char tableName[15] = {0};

	for (int x = 0; x < MAX_TABLES; x++) {
		sprintf(tableName, "./datastore%d", x);
		DeepStore* table = DeepStore::create(tableName, options, "PRIMARY", CT_DATASTORE_LONG_INT, sizeof(int), DATA_SIZE);

		table->addField(CT_DATASTORE_LONG_INT, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0, 0, 0, 0, 0, 0, false);
		table->addKeyPart(0u, CT_DATASTORE_LONG_INT, sizeof(int), 0, 0, 0);
		table->initialize();

		table->open(CTX);
		table->recover(CTX, (initial == false) /* rebuild */);

		TABLES[x] = table;
	}
}

void shutdown() {
	printf("Tearing down %d tables...\n", MAX_TABLES);

	for (int x = 0; x < MAX_TABLES; x++) {
		TABLES[x]->close();
		delete TABLES[x];
	}

	delete CTX;
}

