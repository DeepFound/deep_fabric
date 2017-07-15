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
#include "cxx/util/Logger.h"
#include "cxx/lang/types.h"
#include "cxx/lang/String.h"
#include "cxx/util/CommandLineOptions.h"

#include "RealtimeFabric.h"
#include "RealtimeFabricMatrix.h"

#include "com/deepis/communication/fabricconnector/CassiServiceBridge.h"
using namespace com::deepis::communication::fabricconnector;

using namespace cxx::lang;
using namespace cxx::util;

static const ulongtype GIB_MULTIPLIER = 1024*1024*8;

int main(int argc, char** argv) {
        cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
        cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
        cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
        cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	cxx::fabric::RealtimeFabricAdjMatrix cAdj1(1);
	cxx::fabric::RealtimeFabricAdjMatrix cAdj2(2);

	DEEP_LOG_INFO(OTHER, "%s initializing.\n", argv[0]);

	com::deepis::core::util::CommandLineOptions options(argc, argv);

	uinttype iServerId = 1;

        iServerId = options.getInteger("-server", iServerId);

	cxx::lang::String cServerIp = "tcp://127.0.0.1:10001";
	cServerIp = options.getString("-sip", cServerIp);

	ulongtype iGiB = 0;
	iGiB = options.getInteger("-gib", 1);

	iGiB *= GIB_MULTIPLIER;

	LoadListEntry cLoader[] = {
		singletonCassiServiceBridge, 0,
		0, 0
	};

	cxx::fabric::RealtimeFabric rtFabric(iServerId,
					     cServerIp,
					     cLoader,
					     iGiB);

	while (true) {
		rtFabric.poll(10);
	}

	DEEP_LOG_INFO(OTHER, "%s connector created.\n", argv[0]);
}
