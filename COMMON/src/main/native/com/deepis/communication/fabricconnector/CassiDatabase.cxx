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
#include "CassiDatabase.h"

#include "cxx/lang/System.h"
#include "cxx/util/Logger.h"

#include "com/deepis/communication/fabricconnector/FabricMessageService.cxx"
#include "com/deepis/communication/fabricconnector/FabricMessageServiceFactory.h"

using namespace cxx::lang;
using namespace cxx::util::concurrent::atomic;
using namespace com::deepis::datastore::api;
using namespace com::deepis::communication::fabricconnector;


//using namespace com::deepis::communication::common;

std::string getJSONvalue(json::value &the_json) 
{

	cxx::lang::String cCmdStr;
	the_json.str(cCmdStr);
	return cCmdStr;

	//subbuffer buff = the_json.raw_subbuffer();
	//std::string the_string_value(buff.begin(), buff.length());
	//return the_string_value;
}



std::string getJSON_CommandName(JSON_Command command)
{
	switch(command) {
		case GET: 				return std::string("get");
		case INSERT: 			return std::string("insert");
		case CREATE_DATABASE: 	return std::string("createDatabase");
		case USE_DATABASE: 		return std::string("useDatabase");
		case CREATE_TABLE: 		return std::string("createTable");
		case DOUPDATE:			return std::string("update");
		case DELETE:			return std::string("delete");
		case SUBSCRIBETO:		return std::string("subscribeTo");
		case ADDSUBSCRIBER:		return std::string("addSubscriber");
		default: 				return std::string("UNKNOWN");
	};
}

JSON_Command String_to_JSON_Command(std::string s)
{
	if (s == "get") {return GET;}
	if (s == "insert") {return INSERT;}
	if (s == "createDatabase") {return CREATE_DATABASE;}
	if (s == "useDatabase") {return USE_DATABASE;}
	if (s == "createTable") {return CREATE_TABLE;}
	if (s == "update") {return DOUPDATE;}
	if (s == "delete") {return DELETE;}
	if (s == "subscribeTo") {return SUBSCRIBETO;}
	if (s == "addSubscriber") {return ADDSUBSCRIBER;}
	return UNKNOWN;
}


Table::Table(FabricCassiMessageQueue* pcParent)
{
	this->m_pcParent = pcParent;
	this->ROW_SIZE = 0;
	this->CTX = new DeepThreadContext();
	this->json_get_return = new json_object;
	this->create_statement = json::value();
}

Table::~Table() 
{

}

void Table::unmount() 
{
	DEEP_LOG_DEBUG(OTHER, "Unmounting Primary for table %s.\n", this->sTableName.c_str());
	this->PRIMARY->close(this->CTX);
	for(std::map<string,DeepStore*>::const_iterator it = SECONDARYS.begin(); it != SECONDARYS.end(); it++)
	{
		this->SECONDARYS[it->first]->close(this->CTX);
	}
	for(std::map<string,DeepStore*>::const_iterator it = SECONDARYS.begin(); it != SECONDARYS.end(); it++)
	{
		delete this->SECONDARYS[it->first];
		this->SECONDARYS[it->first] = NULL;
	}	
	delete this->PRIMARY;
	this->PRIMARY = NULL;
}


void Table::addSubscriber()
{
	static boolean added = false;

	if (added == false) {
		this->Primary_MS->addSubscriber(new PeerInfo(2, "", DISTRIBUTED_HA_ASYNC_SLAVE));
		this->Primary_MS->addSubscriber(new PeerInfo(3, "", DISTRIBUTED_HA_ASYNC_SLAVE));
		added = true;
	}
}

void Table::subscribeTo()
{
	this->Primary_MS->subscribeTo(new PeerInfo(1, "", DISTRIBUTED_MASTER));
}

void Table::create(json::value the_command, boolean del) 
{

	std::string sServerId = getJSONvalue(the_command["server_id"]);
	int serverId = atoi(sServerId.c_str());

	std::string sRole = getJSONvalue(the_command["role"]);

	com::deepis::db::store::relative::distributed::MapBehavior role;
	if (sRole == "MASTER") {
		role = DISTRIBUTED_MASTER;
	} else if (sRole == "SLAVE") {
		role = DISTRIBUTED_HA_ASYNC_SLAVE;
	} else {
		role = DISTRIBUTED_MASTER;
	}

	this->peerInfo = new PeerInfo(serverId /* serverId */, "", role);
	this->Primary_MS = FabricMessageServiceFactory::createMessageService(CT_DATASTORE_LONG_INT); //TODO make it more dynamic for the deep type

	this->create_statement = the_command;
	this->another_create_statement = the_command;

	DEEP_LOG_DEBUG(OTHER, "ooooooooooo. %lu  -- %p -- %p \n", this->create_statement["indexes"].size(),this , &create_statement);

	this->sTableName = getJSONvalue(the_command["table_name"]) ;

	int options = CT_DATASTORE_OPTION_CREATE;
	if (del) {
		options |= CT_DATASTORE_OPTION_DELETE;
	}

	std::string str1="./";
	str1.append(sTableName);
	//const char *cstr = str1.c_str();

	// figure out the size of the row for ROW_SIZE
	for (size_t i = 0; i < the_command["fields"].size(); i++) 	
    {
		//std::string s_field_name = getJSONvalue(the_command["fields"][i]["name"]) ;
		//std::string s_field_type = getJSONvalue(the_command["fields"][i]["type"]) ;
    	this->column_types.insert(std::make_pair(getJSONvalue(the_command["fields"][i]["name"]),getJSONvalue(the_command["fields"][i]["type"])));

		if (getJSONvalue(the_command["fields"][i]["type"]) == "int") 
		{
			this->column_sizes.insert(std::make_pair(getJSONvalue(the_command["fields"][i]["name"]),sizeof(int)));
			this->column_deeptypes.insert(std::make_pair(getJSONvalue(the_command["fields"][i]["name"]),CT_DATASTORE_ULONG_INT));
			this->ROW_SIZE = this->ROW_SIZE + sizeof(int);
			
		} else if (getJSONvalue(the_command["fields"][i]["type"]) == "double") {
			this->column_sizes.insert(std::make_pair(getJSONvalue(the_command["fields"][i]["name"]),sizeof(double)));
			this->column_deeptypes.insert(std::make_pair(getJSONvalue(the_command["fields"][i]["name"]),CT_DATASTORE_DOUBLE));
			this->ROW_SIZE = this->ROW_SIZE + sizeof(double);
		} else if (getJSONvalue(the_command["fields"][i]["type"]) == "char") {

			string s_char_size = getJSONvalue(the_command["fields"][i]["size"]);
			int char_size = atoi(s_char_size.c_str()) ;
			this->column_sizes.insert(std::make_pair(getJSONvalue(the_command["fields"][i]["name"]),char_size));
			this->column_deeptypes.insert(std::make_pair(getJSONvalue(the_command["fields"][i]["name"]),CT_DATASTORE_TEXT));
			this->ROW_SIZE = this->ROW_SIZE + char_size;
		}

    }


	this->PRIMARY = DeepStore::create(str1.c_str(), options, 
									"PRIMARY", 
									this->getIndexType(the_command["indexes"], "PRIMARY KEY",  ""), 
									this->getIndexSize(the_command["indexes"], "PRIMARY KEY",  ""), 
									this->ROW_SIZE,role);


	// add all the fields to the primary
	// foreach column -> addField
	std::string column_name;
	for (size_t i = 0; i < the_command["fields"].size(); i++)
	{
		column_name = getJSONvalue(the_command["fields"][i]["name"]);
		this->PRIMARY->addField(this->column_deeptypes[column_name],       // type
								this->column_deeptypes[column_name], 	   // real type
								this->column_sizes[column_name],      // packLength
								0, 			// rowPackLength
								0, 			// keyLength
								0, 			// lengthBytes
								i, 			// index
								0, 			// nullBit
								0, 			// nullOffset
								0, 			// valueOffset
								0, 			// characterSet
								false, 		// gcolVirtual
								column_name.c_str()  // fieldName
								);

		this->column_positions.insert(std::make_pair(column_name,i));
		this->positions_column.insert(std::make_pair(i,column_name));
	}

    for (size_t i = 0; i < the_command["indexes"].size(); i++) 
    {
    	std::string s_index_type = getJSONvalue(the_command["indexes"][i]["type"]) ;
		std::string s_index_desc = getJSONvalue(the_command["indexes"][i]["description"]) ;

		if (s_index_type == "PRIMARY KEY")  
		{
			// todo add the keyparts for the primary
			// for each column in the index

			for (size_t k = 0; k < the_command["indexes"][i]["columns"].size(); k++)
			{
				this->PRIMARY->addKeyPart(
						this->column_positions[getJSONvalue(the_command["indexes"][i]["columns"][k])],
						this->column_deeptypes[getJSONvalue(the_command["indexes"][i]["columns"][k])],        // type
						this->column_sizes[getJSONvalue(the_command["indexes"][i]["columns"][k])],            // size in key
						this->getColumnOffsetInValue(getJSONvalue(the_command["indexes"][i]["columns"][k])),  // offset in value
						0,                     // null offset in value
						0);                    // null bit
			}
			this->PRIMARY->initialize(this->Primary_MS, this->peerInfo);
    		this->PRIMARY->open(this->CTX);
    		
		}
	}

    for (size_t i = 0; i < the_command["indexes"].size(); i++) 
    {

		std::string s_index_type = getJSONvalue(the_command["indexes"][i]["type"]) ;
		std::string s_index_desc = getJSONvalue(the_command["indexes"][i]["description"]) ;

		if (s_index_type == "KEY" || s_index_type == "UNIQUE KEY") 
		{
			std::string str1="./";
			str1.append(s_index_desc);
			str1.append(".");
			str1.append(this->sTableName);
			const char *sec_name = str1.c_str();

			int secondary_size = this->getIndexSize(the_command["indexes"], "KEY",  s_index_desc) + this->getIndexSize(the_command["indexes"], "PRIMARY KEY",  "");

			this->SECONDARYS.insert(std::make_pair(s_index_desc, DeepStore::create(sec_name, options, s_index_desc.c_str(), CT_DATASTORE_COMPOSITE, secondary_size, this->ROW_SIZE,role) )); 

			for (size_t l = 0; l < the_command["fields"].size(); l++)
			{
				
				column_name = getJSONvalue(the_command["fields"][l]["name"]);
				this->SECONDARYS[s_index_desc]->addField(this->column_deeptypes[column_name], 
														 this->column_deeptypes[column_name], 
														 this->column_sizes[column_name], 
														 0, 0, 0, l, 0, 0, 0, 0, false, column_name.c_str());
			}

			for (size_t j = 0; j < the_command["indexes"][i]["columns"].size(); j++)
			{
				column_name = getJSONvalue(the_command["indexes"][i]["columns"][j]);
				this->SECONDARYS[s_index_desc]->addKeyPart(
						this->column_positions[column_name],     	// position
						this->column_deeptypes[column_name], 		// type
						this->column_sizes[column_name],           	// size in key
						this->getColumnOffsetInValue(column_name), 	// offset in value
						0,				// null offset in value
						0,				// null bit
						false,			// isIgnored
						false,			// is Reserved key
						-1,				// variablePosition
						-1);			// primaryPosition
			}

			// we need to add the primary key parts. 
			bool isIgnored;
			bool has_primary;

			if (s_index_type == "KEY") {
				// if it's non-unique, you add the primary to the end, but say that it is NOT ignored
				isIgnored = false;
				has_primary = true;

			} else if (s_index_type == "UNIQUE KEY") {
				// if it's unique, you add the primary to the end, but say that it's ignored
				isIgnored = true;
				has_primary = false;
			} 	

			for (size_t ii = 0; ii < the_command["indexes"].size(); ii++)
			{
				if (getJSONvalue(the_command["indexes"][ii]["type"]) == "PRIMARY KEY") 
				{
					for (size_t kk = 0; kk < the_command["indexes"][ii]["columns"].size(); kk++)
					{
						this->SECONDARYS[s_index_desc]->addKeyPart(
									this->column_positions[getJSONvalue(the_command["indexes"][ii]["columns"][kk])],     // position
									this->column_deeptypes[getJSONvalue(the_command["indexes"][ii]["columns"][kk])], 			// type
									this->column_sizes[getJSONvalue(the_command["indexes"][ii]["columns"][kk])],           // size in key
									this->getColumnOffsetInValue(getJSONvalue(the_command["indexes"][ii]["columns"][kk])),					// offset in value
									0,					// null offset in value
									0,					// null bit
									isIgnored,			//  isIgnored
									false,				// is Reserved key
									-1,					//variablePosition
									kk);				//primaryPosition
					}
				}
			} 
			this->SECONDARYS[s_index_desc]->initialize(this->Primary_MS, this->peerInfo);  //TODO need to make a MS for each seconday (map of them)
			this->PRIMARY->associate(this->SECONDARYS[s_index_desc],has_primary);
			this->SECONDARYS[s_index_desc]->open(this->CTX);
			this->SECONDARYS[s_index_desc]->recover(this->CTX, false);
		} 
    }

    this->PRIMARY->recover(this->CTX, false);

    this->m_pcParent->registerTableMessageService(this->Primary_MS);

}

unsigned int Table::getColumnOffsetInValue(string column_name) 
{
	unsigned int offset = 0;
	for (std::map<unsigned int, string>::const_iterator it = this->positions_column.begin(); it != this->positions_column.end(); it++)
	{
		unsigned int key = it->first;
		if (this->positions_column[key] == column_name) 
		{
			return offset;
		}
		offset += this->column_sizes[this->positions_column[key]];
	}
	return 0;  
}

int Table::getIndexType(json::value &the_indexes, std::string the_type,  std::string the_description) 
{
	int type = 0;
	for (size_t i = 0; i < the_indexes.size(); i++) 
	{
		if ((getJSONvalue(the_indexes[i]["type"]) == the_type) && (getJSONvalue(the_indexes[i]["description"]) == the_description)) 
		{
			if (the_indexes[i]["columns"].size() > 1) 
			{
				return CT_DATASTORE_COMPOSITE;
			}

			for (size_t k = 0; k < the_indexes[i]["columns"].size(); k++)
			{
				type = this->column_deeptypes[getJSONvalue(the_indexes[i]["columns"][k])];
			}
			return type;
		}
	}
	return type;
}

int Table::getIndexSize(json::value &the_indexes, std::string the_type,  std::string the_description) 
{
	int size = 0;
	for (size_t i = 0; i < the_indexes.size(); i++) 
	{
		if ((getJSONvalue(the_indexes[i]["type"]) == the_type) && (getJSONvalue(the_indexes[i]["description"]) == the_description)) 
		{
			for (size_t k = 0; k < the_indexes[i]["columns"].size(); k++)
			{
				size += this->column_sizes[getJSONvalue(the_indexes[i]["columns"][k])];
			}
		}
	}
	return size;
}

void Table::insert(json::value &the_command) 
{
	std::string s_table_name = getJSONvalue(the_command["table_name"]) ;
	int transaction = this->CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	this->CTX->associateTransaction(this->PRIMARY);

	for (size_t i = 0; i < the_command["thedata"].size(); i++)
	{
		int cursor = 0;
		unsigned char* value = (unsigned char*) malloc(this->ROW_SIZE);
		memset(value, 0, this->ROW_SIZE);

		string column_name, datatype; 
		
		for (size_t j = 0; j < the_command["thedata"][i].size(); j++)
		{
			std::string sval = getJSONvalue(the_command["thedata"][i][j]);

			// given the position, whats the column name
			column_name = this->positions_column[j];
			// given the column name whats the datatype
			datatype = this->column_types[column_name];
			int the_size = this->column_sizes[column_name];

			if (datatype == "int") {
				int the_int_value = atoi(sval.c_str());
				memcpy(value + cursor, &the_int_value, sizeof(int));
				cursor += sizeof(int);				
			} else if ( datatype == "double" ) {
				double the_double_value = atof(sval.c_str());
				memcpy(value + cursor, &the_double_value, sizeof(double));
				cursor += sizeof(double);
			} else if ( datatype == "char" ) {
				char the_char_val[the_size];
				strcpy(the_char_val, sval.c_str());
				memcpy(value + cursor, &the_char_val, the_size);
				cursor += the_size;
			}

		}

		if (this->PRIMARY->put(this->CTX, value, this->ROW_SIZE) != CT_DATASTORE_SUCCESS)
		{

			DEEP_LOG_DEBUG(OTHER, "Put Failed for table %s.\n", s_table_name.c_str());
			this->printDataStoreErrorCodes();
			this->CTX->rollbackTransaction(transaction);
			return;
		}

		free(value);
	}
	this->CTX->commitTransaction();
}

void Table::printDataStoreErrorCodes()
{

	//DEEP_LOG_DEBUG(OTHER, "Error Code on PRIMARY: %s.\n", this->PRIMARY->getErrorCode()); 
	std::cout << "Error Code on PRIMARY is: " << this->PRIMARY->getErrorCode() << "\n";
	for(std::map<string,DeepStore*>::const_iterator it = SECONDARYS.begin(); it != SECONDARYS.end(); it++)
	{
		std::string key = it->first;
		//DEEP_LOG_DEBUG(OTHER, "Error Code on SECONDARY: %s.\n", this->SECONDARYS[key]->getErrorCode()); 
		std::cout << "Error Code on SECONDARY " << key << " is: " << this->SECONDARYS[key]->getErrorCode() << "\n";
	}

}

void Table::printRowHeader(json::value &columns_to_print)
{
	for (size_t i = 0; i < columns_to_print.size(); i++) 
	{
		string col_to_print = getJSONvalue(columns_to_print[i]);

		for (std::map<unsigned int,string>::const_iterator it = this->positions_column.begin(); it != this->positions_column.end(); it++)
		{
			unsigned int key = it->first;
			string column_name = this->positions_column[key];
			if ( column_name == col_to_print )
			{
				cout << column_name <<  ", ";
				break;
			} else if (col_to_print == "*") {
				cout << column_name <<  ", ";
			}
		}
	}
	cout << "\n";	
}

void Table::getRowHeader(json::value &columns_to_print)
{
	

	for (size_t i = 0; i < columns_to_print.size(); i++) 
	{
		string col_to_print = getJSONvalue(columns_to_print[i]);

		for (std::map<unsigned int,string>::const_iterator it = this->positions_column.begin(); it != this->positions_column.end(); it++)
		{
			unsigned int key = it->first;
			string column_name = this->positions_column[key];
			if ( column_name == col_to_print )
			{
				json_header_array.add(column_name);
				break;
			} else if (col_to_print == "*") {
				json_header_array.add(column_name);
			}
		}
	}
	//this->json_header.add("columns", json_header_array);
}


void Table::printRow(unsigned char* row_value, json::value &columns_to_print) 
{
	//json_array json_columns;
	

	unsigned int position = 0;

	for (size_t i = 0; i < columns_to_print.size(); i++) 
	{
		string col_to_print = getJSONvalue(columns_to_print[i]);
		position = 0;

		json_array json_columns;

		for (std::map<unsigned int,string>::const_iterator it = this->positions_column.begin(); it != this->positions_column.end(); it++)
		{
			unsigned int key = it->first;
			string column_name = this->positions_column[key];
			int the_size = this->column_sizes[column_name];

			if (this->column_types[column_name] == "int") {
				if ( column_name == col_to_print ) 
				{
					json_columns.add(*((int*) (row_value + position)));
					break;
				} else if (col_to_print == "*") {
					json_columns.add(*((int*) (row_value + position)));
				}
				position += sizeof(int);
			} else if (this->column_types[column_name] == "double") {
				if ( column_name == col_to_print )
				{
					json_columns.add(*((double*) (row_value + position)));
					break;
				} else if (col_to_print == "*") {
					json_columns.add(*((double*) (row_value + position)));
				}
				position += sizeof(double);			
			} else if (this->column_types[column_name] == "char") {
				if ( column_name == col_to_print )
				{
					json_columns.add((row_value + position));
					break;
				} else if (col_to_print == "*") {
					json_columns.add((row_value + position));
				}
				position += the_size;
			}
		}
		this->json_rows.add(json_columns);

	}
	
}

void Table::get(json::value &the_command) 
{


	this->json_header_array = json_array();
	this->json_rows = json_array();

	int transaction = this->CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	this->CTX->associateTransaction(this->PRIMARY);

	unsigned char* value = (unsigned char*) malloc(this->ROW_SIZE);
	unsigned int length = this->ROW_SIZE;

	// if where clause does not exists 
	if (the_command["where"].is_unset()) {
		//this->printRowHeader(the_command["columns"]);
		this->getRowHeader(the_command["columns"]);
		if (PRIMARY->get(this->CTX, this->scan_cursor, &value, &length, this->scan_cursor, CT_DATASTORE_GET_FIRST) == CT_DATASTORE_SUCCESS) 
		{
			this->printRow(value,the_command["columns"]);
			while (PRIMARY->get(this->CTX, this->scan_cursor, &value, &length, this->scan_cursor, CT_DATASTORE_GET_NEXT) == CT_DATASTORE_SUCCESS) 
			{
				this->printRow(value,the_command["columns"]);
			}
		} 

	} else {

		//DEEP_LOG_DEBUG(OTHER, "Getting a specific row. %d\n", this->create_statement["indexes"].size());
		//DEEP_LOG_DEBUG(OTHER, "Getting a specific row. %d  -- %p\n", this->create_statement["indexes"].size(),this );
		DEEP_LOG_DEBUG(OTHER, "Getting a specific row. %lu  -- %p -- %p \n", this->another_create_statement["indexes"].size(),this , &create_statement);
		// assuming only 1 item in the  where clause for now.
		// which column is it? 
		// loop through exiting columns within the primary or secondary and see if its exists in the where clause

		long long count = 0;

		count = this->PRIMARY->size(this->CTX);
		DEEP_LOG_DEBUG(OTHER, "COUNT: %lld \n",count);
		char str[20];
		sprintf(str, "%lld", count);
		this->json_rows.add(str);


		for (size_t i = 0; i < this->create_statement["indexes"].size(); i++) 
		{
		//	DEEP_LOG_DEBUG(OTHER, "here1 %d\n", i);

			for (size_t j = 0; j < this->create_statement["indexes"][i]["columns"].size(); j++)
			{
		//		DEEP_LOG_DEBUG(OTHER, "here2 %d\n", j);
				if (!(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])].is_unset())) 
				{

					string column_name = getJSONvalue(this->create_statement["indexes"][i]["columns"][j]);
					string column_type = this->column_types[column_name];
					DEEP_LOG_DEBUG(OTHER, "get using index %s.\n", column_name.c_str());
					int column_size = this->column_sizes[column_name];
					unsigned char key[column_size];
					unsigned char retkey[column_size];

					if (column_type == "int") {
						string s_val = getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]);
						int id = atoi(s_val.c_str());
						memcpy(key, &id, sizeof(int));
					} else if (column_type == "double") {
						string s_val = getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]);
						double id = std::atof(s_val.c_str());
						memcpy(key, &id, sizeof(double));
					} else if (column_type == "char") {
						string s_val = getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]);
						char id[column_size];
						strcpy(id, s_val.c_str());
						memcpy(key, &id, column_size);
					}

					if (PRIMARY->get(this->CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != 0) {
						DEEP_LOG_DEBUG(OTHER, "Get Failed .\n");

						this->CTX->rollbackTransaction(transaction);

					} else {
						//this->printRowHeader(the_command["columns"]);
						DEEP_LOG_DEBUG(OTHER, "Get Worked?\n");
						this->getRowHeader(the_command["columns"]);
						this->printRow(value,the_command["columns"]);

					}

				}

			}

		} 

	}

//	while (this->SECONDARYS["column1"]->get(this->CTX, key, &value, &length, retkey, CT_DATASTORE_GET_NEXT_MATCH) == CT_DATASTORE_SUCCESS) 
//	{
//		printf("key: %d value: %d %d \n", id, *((int*) (value)), *((int*) (value + sizeof(int))));
//	}

	/*	
	if (PRIMARY->get(this->CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT) != 0) {
		printf("FAILED - Primary get %d\n", id);
		//exit(1);
	} else {

		cout << " get worked?! " << "\n";
		printf("primary key: %d value: %d %d \n", id, *((int*) (value)), *((int*) (value + sizeof(int))));

	}
	
	*/
	free(value);

	free(this->json_get_return);
	this->json_get_return = new json_object;

	this->json_get_return->add("columns", this->json_header_array);
	this->json_get_return->add("the_data", this->json_rows);


	CTX->commitTransaction();

}

void Table::update(json::value &the_command) {

	int transaction = this->CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	this->CTX->associateTransaction(this->PRIMARY);

	unsigned char* value = (unsigned char*) malloc(this->ROW_SIZE);

	unsigned char* nvalue = (unsigned char*) malloc(this->ROW_SIZE);

	unsigned int length = this->ROW_SIZE;

	// if where clause exists 
	if (the_command["where"].is_unset()) {
		PRIMARY->get(this->CTX, this->scan_cursor, &value, &length, this->scan_cursor, CT_DATASTORE_GET_FIRST, CT_DATASTORE_LOCK_WRITE);
		memcpy(nvalue, value, this->ROW_SIZE);
		// put the new values in.
		int cursor = 0;
		for (size_t k = 0; k < this->create_statement["fields"].size(); k++) 
		{
			string a_column =  getJSONvalue(this->create_statement["fields"][k]["name"]);
			// if a_column is in the list of columns to be updated then update the nvalue with it.
			for (size_t l = 0; l < the_command["set"].size(); l++) {
				if (a_column == getJSONvalue(the_command["set"][l][0]))
				{
					// lets update the nvalue ;
					string s_val = getJSONvalue(the_command["set"][l][1]);
					if (this->column_types[a_column] == "int") {
						int val = atoi(s_val.c_str());
						memcpy(nvalue + cursor, &val, this->column_sizes[a_column]);
					} else if (this->column_types[a_column] == "double") {
						double val = atof(s_val.c_str());
						memcpy(nvalue + cursor, &val, this->column_sizes[a_column]);										

					} else if (this->column_types[a_column] == "char") {
						char val[this->column_sizes[a_column]];
						strcpy(val, s_val.c_str());
						memcpy(nvalue + cursor, &val, this->column_sizes[a_column]);
					}
					break;
				}

			}
			cursor += this->column_sizes[a_column];

		}

		if (PRIMARY->update(this->CTX, value, this->ROW_SIZE, nvalue, this->ROW_SIZE, false /*already locked*/) != CT_DATASTORE_SUCCESS) {
			DEEP_LOG_DEBUG(OTHER, "Update Failed .\n");
		}

		while (PRIMARY->get(this->CTX, this->scan_cursor, &value, &length, this->scan_cursor, CT_DATASTORE_GET_NEXT, CT_DATASTORE_LOCK_WRITE) == CT_DATASTORE_SUCCESS) 
		{
			memcpy(nvalue, value, this->ROW_SIZE);
			// put the new values in.
			int cursor = 0;
			for (size_t k = 0; k < this->create_statement["fields"].size(); k++) 
			{
				string a_column =  getJSONvalue(this->create_statement["fields"][k]["name"]);
				// if a_column is in the list of columns to be updated then update the nvalue with it.
				for (size_t l = 0; l < the_command["set"].size(); l++) {
					if (a_column == getJSONvalue(the_command["set"][l][0]))
					{
						// lets update the nvalue ;
						string s_val = getJSONvalue(the_command["set"][l][1]);
						if (this->column_types[a_column] == "int") {
							int val = atoi(s_val.c_str());
							memcpy(nvalue + cursor, &val, this->column_sizes[a_column]);
						} else if (this->column_types[a_column] == "double") {
							double val = atof(s_val.c_str());
							memcpy(nvalue + cursor, &val, this->column_sizes[a_column]);										
						} else if (this->column_types[a_column] == "char") {
							char val[this->column_sizes[a_column]];
							strcpy(val, s_val.c_str());
							memcpy(nvalue + cursor, &val, this->column_sizes[a_column]);
						}
						break;
					}

				}
				cursor += this->column_sizes[a_column];
			}

			if (PRIMARY->update(this->CTX, value, this->ROW_SIZE, nvalue, this->ROW_SIZE, false /*already locked*/) != CT_DATASTORE_SUCCESS) {
				DEEP_LOG_DEBUG(OTHER, "Update Failed .\n");
				this->printDataStoreErrorCodes();
				this->CTX->rollbackTransaction(transaction);
				return;
			} 
		}
	} else {
		
		// assuming only 1 item in the  where clause for now.
		// which column is it? 
		//loop through exiting columns within the primary or secondary and see if its exists in the where clause

		for (size_t i = 0; i < this->create_statement["indexes"].size(); i++) 
		{

			for (size_t j = 0; j < this->create_statement["indexes"][i]["columns"].size(); j++)
			{

				if (!(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])].is_unset())) 
				{
					//cout << " we have " << getJSONvalue(this->create_statement["indexes"][i]["columns"][j]) << " in an index.\n";
					//cout << " value: " << getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]) << "\n";
				
					string column_name = getJSONvalue(this->create_statement["indexes"][i]["columns"][j]);
					string column_type = this->column_types[column_name];
					int column_size = this->column_sizes[column_name];
					unsigned char key[column_size];
					unsigned char retkey[column_size];

					if (column_type == "int") {
						string s_val = getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]);
						int id = atoi(s_val.c_str());
						memcpy(key, &id, sizeof(int));
					} else if (column_type == "double") {
						string s_val = getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]);
						double id = std::atof(s_val.c_str());
						memcpy(key, &id, sizeof(double));
					} else if (column_type == "char") {
						string s_val = getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]);
						char id[this->column_sizes[column_name]];
						strcpy(id, s_val.c_str());
						memcpy(key, &id, column_size);
					}

					if (PRIMARY->get(this->CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE) != 0) {
						DEEP_LOG_DEBUG(OTHER, "Get Failed .\n");
						//exit(1);
					} else {
						memcpy(nvalue, value, this->ROW_SIZE);

						// we need to figure out what exactly what we are updating...
						int cursor = 0;
						for (size_t k = 0; k < this->create_statement["fields"].size(); k++) 
						{
							string a_column =  getJSONvalue(this->create_statement["fields"][k]["name"]);
							// if a_column is in the list of columns to be updated then update the nvalue with it.
							
							for (size_t l = 0; l < the_command["set"].size(); l++) {
								if (a_column == getJSONvalue(the_command["set"][l][0]))
								{
									// lets update the nvalue ;
									string s_val = getJSONvalue(the_command["set"][l][1]);
									if (this->column_types[a_column] == "int") {
										int val = atoi(s_val.c_str());
										memcpy(nvalue + cursor, &val, this->column_sizes[a_column]);
									} else if (this->column_types[a_column] == "double") {
										double val = atof(s_val.c_str());
										memcpy(nvalue + cursor, &val, this->column_sizes[a_column]);										

									} else if (this->column_types[a_column] == "char") {
										char val[column_size];
										strcpy(val, s_val.c_str());
										memcpy(nvalue + cursor, &val, this->column_sizes[a_column]);
									}
									break;

								}

							}
							cursor += this->column_sizes[a_column];

						}

						if (PRIMARY->update(this->CTX, value, this->ROW_SIZE, nvalue, this->ROW_SIZE, false /*already locked*/) != CT_DATASTORE_SUCCESS) {
							DEEP_LOG_DEBUG(OTHER, "Update Failed .\n");
							this->printDataStoreErrorCodes();
							this->CTX->rollbackTransaction(transaction);
							return;
						}

					}

				}

			}

		} 

	}

	free(value);

	this->CTX->commitTransaction();

}


void Table::remove(json::value &the_command) 
{

	int transaction = this->CTX->beginTransaction(DeepThreadContext::SAVEPOINT);
	this->CTX->associateTransaction(this->PRIMARY);

	unsigned char* value = (unsigned char*) malloc(this->ROW_SIZE);
	unsigned int length = this->ROW_SIZE;

	// if where clause exists 
	if (the_command["where"].is_unset()) {
		PRIMARY->get(this->CTX, this->scan_cursor, &value, &length, this->scan_cursor, CT_DATASTORE_GET_FIRST, CT_DATASTORE_LOCK_WRITE);
		if (PRIMARY->remove(this->CTX, value, this->ROW_SIZE) != CT_DATASTORE_SUCCESS) {
			DEEP_LOG_DEBUG(OTHER, "Delete Failed .\n");
			this->printDataStoreErrorCodes();
			this->CTX->rollbackTransaction(transaction);
			return;
		}

		while (PRIMARY->get(this->CTX, this->scan_cursor, &value, &length, this->scan_cursor, CT_DATASTORE_GET_NEXT, CT_DATASTORE_LOCK_WRITE) == CT_DATASTORE_SUCCESS) 
		{
			if (PRIMARY->remove(this->CTX, value, this->ROW_SIZE) != CT_DATASTORE_SUCCESS) {
				DEEP_LOG_DEBUG(OTHER, "Delete Failed .\n");
				this->printDataStoreErrorCodes();
				this->CTX->rollbackTransaction(transaction);
				return;
			} 
		}
	} else {
		
		// assuming only 1 item in the  where clause for now.
		// which column is it? 
		//loop through exiting columns within the primary or secondary and see if its exists in the where clause

		for (size_t i = 0; i < this->create_statement["indexes"].size(); i++) 
		{
			for (size_t j = 0; j < this->create_statement["indexes"][i]["columns"].size(); j++)
			{
				if (!(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])].is_unset())) 
				{
					string column_name = getJSONvalue(this->create_statement["indexes"][i]["columns"][j]);
					string column_type = this->column_types[column_name];
					int column_size = this->column_sizes[column_name];
					unsigned char key[column_size];
					unsigned char retkey[column_size];

					if (column_type == "int") {
						string s_val = getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]);
						int id = atoi(s_val.c_str());
						memcpy(key, &id, sizeof(int));
					} else if (column_type == "double") {
						string s_val = getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]);
						double id = std::atof(s_val.c_str());
						memcpy(key, &id, sizeof(double));
					} else if (column_type == "char") {
						string s_val = getJSONvalue(the_command["where"][getJSONvalue(this->create_statement["indexes"][i]["columns"][j])]["="]);
						char id[this->column_sizes[column_name]];
						strcpy(id, s_val.c_str());
						memcpy(key, &id, column_size);
					}

					if (PRIMARY->get(this->CTX, key, &value, &length, retkey, CT_DATASTORE_GET_EXACT, CT_DATASTORE_LOCK_WRITE) != 0) {
						DEEP_LOG_DEBUG(OTHER, "Get in Delete Failed .\n");
						this->printDataStoreErrorCodes();
						this->CTX->rollbackTransaction(transaction);
					} else {
						if (PRIMARY->remove(this->CTX, value, this->ROW_SIZE) != CT_DATASTORE_SUCCESS) {
							DEEP_LOG_DEBUG(OTHER, "Delete Failed .\n");
							this->printDataStoreErrorCodes();
							this->CTX->rollbackTransaction(transaction);
							return;
						}

					}

				}

			}

		} 

	}

	free(value);
	this->CTX->commitTransaction();

}


Database::Database(std::string db_name, FabricCassiMessageQueue* pcParent)
{
	this->m_pcParent = pcParent;
	this->database_name = db_name;
	//see if the db already exists on disk.
	// if not create a directory for it.
	// and make it the cwd?	
	if (! this->exists()) {
		int result = mkdir(this->database_name.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH);
		if (result != 0) {
			//fprintf(stderr, "ERROR %d: unable to mkdir; %s\n", errno, strerror(errno));
			DEEP_LOG_DEBUG(OTHER, "ERROR %d: unable to mkdir; %s\n", errno, strerror(errno));
		}
	}

}

Database::~Database() 
{

}

bool Database::exists()
{
	struct stat sb;
	if (stat(this->database_name.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
	{
	    return true;
	}
	return false;
}

void Database::use() 
{
	if (chdir(this->database_name.c_str()) == -1) {
		//fprintf(stderr, "ERROR %d: unable to chdir; %s\n", errno, strerror(errno));
		DEEP_LOG_DEBUG(OTHER, "ERROR %d: unable to chdir; %s\n", errno, strerror(errno));
	} else {
		DEEP_LOG_DEBUG(OTHER,"Using database %s.\n" , this->database_name.c_str());
	}
}

DistributedCassiDatabase::DistributedCassiDatabase(FabricCassiMessageQueue* pcParent)
{
	this->m_pcParent = pcParent;
	current_database = "";
	DeepStore::setCacheSize(8073741824);
}

DistributedCassiDatabase::~DistributedCassiDatabase()
{


}

void DistributedCassiDatabase::processJSONCommand(json::value command, json::value payload)
{

	this->return_json = new json_object();
	this->return_error_message = "--";

	//json::value payloadc = new json::value(payload);


	switch(String_to_JSON_Command(getJSONvalue(command)))
	{
		case GET : 	{
					if (this->current_database != "") {
						for (size_t k = 0; k < payload["tables"].size(); k++) 
						{
							cout << "going get on table: " << getJSONvalue(payload["tables"][k])  << "ttt:" << this->Databases[this->current_database]->Tables[getJSONvalue(payload["tables"][k])]->positions_column[0]  << "\n";
							this->Databases[this->current_database]->Tables[getJSONvalue(payload["tables"][k])]->get(payload);
							this->return_json = this->Databases[this->current_database]->Tables[getJSONvalue(payload["tables"][k])]->json_get_return;
						}
					} else {
						DEEP_LOG_DEBUG(OTHER, "No database selected.\n");
					}
				}break;
		case DOUPDATE : 	{
					if (this->current_database != "") {
						this->Databases[this->current_database]->Tables[getJSONvalue(payload["table_name"])]->update(payload);
					} else {
						DEEP_LOG_DEBUG(OTHER, "No database selected.\n");
					}
				}break;						
		case DELETE : 	{
					if (this->current_database != "") {
						this->Databases[this->current_database]->Tables[getJSONvalue(payload["table_name"])]->remove(payload);
					} else {
						DEEP_LOG_DEBUG(OTHER, "No database selected.\n");
					}
				}break;						
		case INSERT :	{
					if (this->current_database != "") {
						this->Databases[this->current_database]->Tables[getJSONvalue(payload["table_name"])]->insert(payload);
					} else {
						DEEP_LOG_DEBUG(OTHER, "No database selected.\n");
					}
				}break;
		case CREATE_DATABASE : {
					std::string s_db_name = getJSONvalue(payload);
					this->Databases.insert(make_pair(s_db_name, new Database(s_db_name, this->m_pcParent)));
					} break;
		case USE_DATABASE : {
					std::string s_db_name = getJSONvalue(payload);
					// see if it exists in the map TODO
					this->Databases[s_db_name]->use();
					this->current_database = s_db_name;
					} break;							
		case CREATE_TABLE : {
					if (this->current_database != "")
					{
						std::string s_table_name = getJSONvalue(payload["table_name"]);
						this->Databases[this->current_database]->Tables.insert(std::make_pair(s_table_name, new Table(this->m_pcParent)));
						DEEP_LOG_DEBUG(OTHER, "Creating table %s\n", s_table_name.c_str());
						this->Databases[this->current_database]->Tables[s_table_name]->create(payload, false);
					} else {
						this->return_error_message = "No database selected.";
						DEEP_LOG_DEBUG(OTHER, "No database selected.\n");
					}
					
				}break;
		case SUBSCRIBETO : {
					if (this->current_database != "")
					{
						std::string s_table_name = getJSONvalue(payload["table_name"]);
						this->Databases[this->current_database]->Tables[s_table_name]->subscribeTo();
					} else {
						DEEP_LOG_DEBUG(OTHER, "No database selected.\n");
					}	
				}break;
		case ADDSUBSCRIBER : {
					if (this->current_database != "")
					{
						std::string s_table_name = getJSONvalue(payload["table_name"]);
						this->Databases[this->current_database]->Tables[s_table_name]->addSubscriber();
					} else {
						DEEP_LOG_DEBUG(OTHER, "No database selected.\n");
					}							
				}break;		
		default :
			DEEP_LOG_DEBUG(OTHER, "UNKNOWN Command.\n");
			break;
	};

}




