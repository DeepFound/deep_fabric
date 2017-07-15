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
#include "cxx/lang/nbyte.h"
#include "cxx/lang/Memory.h"
#include "cxx/lang/Runtime.h"
#include "cxx/util/Logger.h"
#include "cxx/util/PrettyPrint.h"
#include "cxx/lang/String.h"
#include "cxx/lang/types.h"

using namespace cxx::lang;
using namespace cxx::util;

nbyte* allocateMB(int mb) {
        DEEP_LOG(INFO, OTHER, "Allocate %d MB\n", mb);

        return new nbyte(mb * 1024 * 1024);
}

void printStats() {
	ulongtype totalMemory = Runtime::getRuntime()->totalMemory();
	ulongtype freeMemory  = Runtime::getRuntime()->freeMemory();
	ulongtype availProc   = Runtime::getRuntime()->availableProcessors();
	ulongtype pageSize    = Runtime::getRuntime()->getPageSize();
	ulongtype rss         = Runtime::getRuntime()->getRSS();

	cxx::lang::String cPretty;
	
	prettyprint::Bytes<ulongtype>(totalMemory, cPretty);
	DEEP_LOG(INFO, OTHER, "RUNTIME (totalMemory): %s %lluB\n", cPretty.c_str(), totalMemory);

	prettyprint::Bytes<ulongtype>(freeMemory, cPretty);
	DEEP_LOG(INFO, OTHER, "RUNTIME (freeMemory):  %s %lluB\n", cPretty.c_str(), freeMemory);

	DEEP_LOG(INFO, OTHER, "RUNTIME (availProc):   %llu cores\n", availProc);

	prettyprint::Bytes<ulongtype>(pageSize, cPretty);
	DEEP_LOG(INFO, OTHER, "RUNTIME (pageSize):    %s\n", cPretty.c_str());

	prettyprint::Bytes<ulongtype>(rss, cPretty);
	DEEP_LOG(INFO, OTHER, "RUNTIME (rss):         %s\n", cPretty.c_str());

	DEEP_LOG(INFO, OTHER, "\n");
}

int main(int argc, char** argv) {

	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	const int iMaxPass = 5;

	for (int i=1; i<=iMaxPass; i++) {
		DEEP_LOG(INFO, OTHER, "Pass %d of %d\n", i, iMaxPass);

		printStats();
	
		DEEP_LOG(INFO, OTHER, "Deleting RUNTIME instance.\n");

		cxx::lang::Runtime* pInstance = Runtime::getRuntime();
		DEEP_LOG(INFO, OTHER, "RUNTIME instance: %p\n\n", pInstance);

		delete pInstance; pInstance = 0;
	}

	return 0;
}
