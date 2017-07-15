/**
 *    Copyright (C) 2010 Deep Software Foundation
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */
#ifndef COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_CASSIDATABASE_H_
#define COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_CASSIDATABASE_H_

#include <iostream>
#include <string>
#include <streambuf>
#include <map>

#include <subbuffer.h>
#include <json.h>
#include <json_parser.h>


#include "cxx/lang/Thread.h"
#include "cxx/lang/System.h"
#include "cxx/util/Logger.h"
#include "cxx/util/CommandLineOptions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "cxx/lang/System.h"
#include "cxx/lang/Thread.h"
#include "cxx/lang/Runnable.h"

#include "com/deepis/db/store/relative/distributed/MapBehavior.h"
#include "com/deepis/db/store/relative/distributed/Serializer.h"
#include "com/deepis/db/store/relative/distributed/PeerInfo.h"
#include "cxx/util/concurrent/atomic/AtomicInteger.h"

#include "com/deepis/datastore/api/DeepStore.h"

#include "com/deepis/db/store/relative/distributed/IMessageService.h"
//#include "com/deepis/communication/fabricconnector/FabricCassiMessageQueue.h"


//using namespace com::deepis::communication::common;

std::string getJSONvalue(json::value &the_json);

enum JSON_Command
{
    GET,
    INSERT,
    CREATE_DATABASE,
    USE_DATABASE,
    CREATE_TABLE,
    DOUPDATE,
    DELETE,
    SUBSCRIBETO,
    ADDSUBSCRIBER,
    UNKNOWN
};

std::string getJSON_CommandName(JSON_Command command);



JSON_Command String_to_JSON_Command(std::string s);


namespace com { namespace deepis { namespace communication { namespace fabricconnector {

using namespace cxx::lang;
using namespace cxx::util::concurrent::atomic;
using namespace com::deepis::datastore::api;

//define MAX_KEY_LENGTH 3072U;

class FabricCassiMessageQueue;

class Table
{

	public:

		PeerInfo* peerInfo;
		IMessageService* Primary_MS;
		DeepStore* PRIMARY;
		int ROW_SIZE, primary_type, primary_size;
		std::map<string,DeepStore*> SECONDARYS;
		std::map<string,unsigned int> column_positions;
		std::map<unsigned int,string> positions_column;
		std::map<string,unsigned int> column_sizes;
		std::map<string,unsigned int> column_deeptypes;
		std::map<string,string> column_types;
		std::string sTableName;
		json::value create_statement;
		json::value another_create_statement;
		//json_object json_header;

		//json_object json_data;

		json_object* json_get_return;

		json_array json_rows;
		json_array json_header_array;

		FabricCassiMessageQueue* m_pcParent;

		unsigned char scan_cursor[2 * 3072U];
		
		DeepThreadContext* CTX;

		Table(FabricCassiMessageQueue* pcParent);

		~Table();

		void addSubscriber();
		void subscribeTo();
		void create(json::value the_command, boolean del);
		unsigned int getColumnOffsetInValue(string column_name);
		int getIndexType(json::value &the_indexes, std::string the_type,  std::string the_description);
		int getIndexSize(json::value &the_indexes, std::string the_type,  std::string the_description);
		void insert(json::value &the_command);
		void get(json::value &the_command);
		void update(json::value &the_command);
		void remove(json::value &the_command);
		void printRow(unsigned char* row_value, json::value &columns_to_print);
		void printRowHeader(json::value &columns_to_print);
		void getRowHeader(json::value &columns_to_print);
		void unmount();
		void printDataStoreErrorCodes();


	protected:
		int num_tables;

};

class Database
{
	public:

		std::string database_name;
		std::map<string,Table*> Tables;
		FabricCassiMessageQueue* m_pcParent; 

		bool exists();
		void use(); 

		Database(std::string db_name, FabricCassiMessageQueue* pcParent);

		~Database();

	protected:


};

class DistributedCassiDatabase
{


	public:

		std::string current_database;
		std::map<string,Database*> Databases; 
		FabricCassiMessageQueue* m_pcParent;
		json_object* return_json;
		cxx::lang::String return_error_message;

		void processJSONCommand(json::value command, json::value payload);

		//DistributedCassiDatabase();
		DistributedCassiDatabase(FabricCassiMessageQueue* pcParent);


		~DistributedCassiDatabase();


	protected:

};


} // fabricconnector
} // communication
} // deepis
} // com

#endif /** COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_CASSIDATABASE_H_ */
