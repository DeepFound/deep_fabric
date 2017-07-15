#include <iostream>
#include "subbuffer.h"
#include "json_parser.h"

int main(int argc, char** argv) {
	subbuffer json_text("{ \
    \"fields\": [ \
        { \
            \"type\": \"int\", \
            \"description\": \"an optional description.\", \
            \"name\": \"PersonID\" \
        }, \
        { \
            \"type\": \"varchar(255)\", \
            \"description\": \"an optional description.\", \
            \"name\": \"LastName\" \
        }, \
        { \
            \"type\": \"varchar(255)\", \
            \"description\": \"an optional description.\", \
            \"name\": \"FirstName\" \
        }, \
        { \
            \"type\": \"varchar(255)\", \
            \"description\": \"an optional description.\", \
            \"name\": \"Address\" \
        }, \
        { \
            \"type\": \"varchar(255)\", \
            \"description\": \"an optional description.\", \
            \"name\": \"City\" \
        } \
    ], \
    \"table_name\": \"Persons\" \
}");

	json::root root(json_text);
	json::object* pcRoot = root.to_object();

	if (true == pcRoot->operator[]("fields").is_array()) {
		std::cout << "Fields Array found!" << std::endl;
		size_t iFieldCount = pcRoot->operator[]("fields").size();

		for (size_t i=0; i<iFieldCount; i++) {
			const json::value& fieldVal = pcRoot->operator[]("fields")[i];
			if (true == fieldVal.is_object()) {
				std::cout << i << " is an object\n";
				subbuffer cFieldBuf = fieldVal.raw_subbuffer();
				std::string cFieldJson(cFieldBuf.begin(), cFieldBuf.length());
				std::cout << cFieldJson << std::endl;
			}
		}
	}

	json_object field;
	field.add(CONST_SUBBUF("type"), "varchar(255)");
	field.add(CONST_SUBBUF("description"), "an optional description.");
	field.add(CONST_SUBBUF("name"), "City");

	std::string jsonField;
	field.to_string(jsonField);

	std::cout << jsonField << std::endl;

	json_object VK;	
	pcRoot->to_json(VK);

	std::string VKOut;
	VK.to_string(VKOut);

	std::cout << VKOut << std::endl;

	return 0;
}
