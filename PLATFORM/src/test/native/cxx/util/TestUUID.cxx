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
#include "cxx/lang/String.h"
#include "cxx/lang/System.h"
#include "cxx/util/Logger.h"
#include "cxx/util/UUID.h"

using namespace cxx::lang;
using namespace cxx::util;

int main(int argc, char** argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	cxx::lang::String cUUID1("b10bfc88-6f97-4498-93ad-d76b97261d4e");

	cxx::util::UUID* pcUUID1 = cxx::util::UUID::fromString(cUUID1);
	cxx::util::UUID* pcUUID2 = cxx::util::UUID::randomUUID();

	int     iCompare = pcUUID1->compareTo(pcUUID2);
	boolean bEquals  = pcUUID1->equals(pcUUID2);

	DEEP_LOG_INFO(OTHER, "pcUUID1->compareTo(pcUUID2) -> %d\n", iCompare);
	DEEP_LOG_INFO(OTHER, "pcUUID1->equals(pcUUID2) -> %d\n", bEquals);

	iCompare = pcUUID1->compareTo(pcUUID1);
	if (0 == iCompare) {
		DEEP_LOG_INFO(OTHER, "pcUUID1->compareTo(pcUUID1) -> %d\n", iCompare);
	} else {
		DEEP_LOG_ERROR(OTHER, "pcUUID1->compareTo(pcUUID1) -> %d\n", iCompare);
		return -1;
	}

	bEquals = pcUUID1->equals(pcUUID1);
	if (true == bEquals) {
		DEEP_LOG_INFO(OTHER, "pcUUID1->equals(pcUUID1) -> %d\n", bEquals);
	} else {
		DEEP_LOG_ERROR(OTHER, "pcUUID1->equals(pcUUID1) -> %d\n", bEquals);
		return -1;
	}

	cxx::lang::String cStr = pcUUID1->toString();
	DEEP_LOG_INFO(OTHER, "pcUUID1 = %s\n", cStr.c_str());

	if (cStr == cUUID1) {
		DEEP_LOG_INFO(OTHER, "%s == %s\n", cUUID1.c_str(), cStr.c_str());
	} else {
		DEEP_LOG_ERROR(OTHER, "%s != %s\n", cUUID1.c_str(), cStr.c_str());
		return -1;
	}

	DEEP_LOG_INFO(OTHER, "pcUUID1 hashcode = %lX\n", pcUUID1->hashCode());

	DEEP_LOG_INFO(OTHER, "pcUUID1 MSBytes  = %llx\n", pcUUID1->getMostSignificantBits());

	DEEP_LOG_INFO(OTHER, "pcUUID1 LSBytes  = %llx\n", pcUUID1->getLeastSignificantBits());

	inttype iVersion = pcUUID1->version();
	if (4 != iVersion) {
		DEEP_LOG_ERROR(OTHER, "Should have returned 4 but returned %d\n", iVersion);
		return -1;
	} else {
		DEEP_LOG_INFO(OTHER, "pcUUID1 version  = %d (RANDOM Generated UUID)\n", iVersion);
	}

	inttype iVariant = pcUUID1->variant();
	if (2 != iVariant) {
		DEEP_LOG_ERROR(OTHER, "Should have returned 2 but returned %d\n", iVariant);
	} else {
		DEEP_LOG_INFO(OTHER, "pcUUID1 variant  = %d\n", iVariant);
	}

	cStr =  pcUUID2->toString();
	DEEP_LOG_INFO(OTHER, "pcUUID2 = %s\n", cStr.c_str());

	DEEP_LOG_INFO(OTHER, "pcUUID2 hashcode = %lX\n", pcUUID2->hashCode());

	DEEP_LOG_INFO(OTHER, "pcUUID2 MSBytes  = %llx\n", pcUUID2->getMostSignificantBits());

	DEEP_LOG_INFO(OTHER, "pcUUID2 LSBytes  = %llx\n", pcUUID2->getLeastSignificantBits());

	iVersion = pcUUID2->version();
	if (4 != iVersion) {
		DEEP_LOG_ERROR(OTHER, "Should have returned 4 but returned %d\n", iVersion);
		return -1;
	} else {
		DEEP_LOG_INFO(OTHER, "pcUUID2 version  = %d (RANDOM Generated UUID)\n", iVersion);
	}

	iVariant = pcUUID2->variant();
	if (2 != iVariant) {
		DEEP_LOG_ERROR(OTHER, "Should have returned 2 but returned %d\n", iVariant);
	} else {
		DEEP_LOG_INFO(OTHER, "pcUUID2 variant  = %d\n", iVariant);
	}

	try {
		DEEP_LOG_INFO(OTHER, "pcUUID1 node      = %llx\n", pcUUID1->node());
	}
	catch(...) {
		DEEP_LOG_INFO(OTHER, "[OK] Caught calling node() on a non-time based UUID!\n");
	}

	try {
		DEEP_LOG_INFO(OTHER, "pcUUID1 timestamp = %llx\n", pcUUID1->timestamp());
	}
	catch(...) {
		DEEP_LOG_INFO(OTHER, "[OK] Caught calling timestamp () on a non-time based UUID!\n");
	}

	try {
		DEEP_LOG_INFO(OTHER, "pcUUID2 node      = %llx\n", pcUUID2->node());
	}
	catch(...) {
		DEEP_LOG_INFO(OTHER, "[OK] Caught calling node() on a non-time based UUID!\n");
	}

	try {
		DEEP_LOG_INFO(OTHER, "pcUUID2 timestamp = %llx\n", pcUUID2->timestamp());
	}
	catch(...) {
		DEEP_LOG_INFO(OTHER, "[OK] Caught calling timestamp () on a non-time based UUID!\n");
	}

	delete pcUUID1; pcUUID1 = 0;
	delete pcUUID2; pcUUID2 = 0;

	cxx::lang::String cUUIDTimeBase("6998173e-31a3-11e6-9f80-000c29589848");
	cxx::util::UUID*  pcUUID1TimeBased = cxx::util::UUID::fromString(cUUIDTimeBase);

	iVersion = pcUUID1TimeBased->version();
	if (1 != iVersion) {
		DEEP_LOG_ERROR(OTHER, "Should have returned 1 but returned %d\n", iVersion);
		return -1;
	} else {
		DEEP_LOG_INFO(OTHER, "pcUUIDTimeBased version   = %d (Time Based UUID)\n", iVersion);
	}

	iVariant = pcUUID1TimeBased->variant();
	if (2 != iVariant) {
		DEEP_LOG_ERROR(OTHER, "Should have returned 2 but returned %d\n", iVariant);
	} else {
		DEEP_LOG_INFO(OTHER, "pcUUIDTimeBased variant   = %d\n", iVariant);
	}

	cStr = pcUUID1TimeBased->toString();
	DEEP_LOG_INFO(OTHER, "pcUUIDTimeBased           = %s\n", cStr.c_str());

	DEEP_LOG_INFO(OTHER, "pcUUIDTimeBased hashcode  = %lX\n", pcUUID1TimeBased->hashCode());

	DEEP_LOG_INFO(OTHER, "pcUUIDTimeBased MSBytes   = %llx\n", pcUUID1TimeBased->getMostSignificantBits());

	DEEP_LOG_INFO(OTHER, "pcUUIDTimeBased LSBytes   = %llx\n", pcUUID1TimeBased->getLeastSignificantBits());

	DEEP_LOG_INFO(OTHER, "pcUUIDTimeBased node      = %llx\n", pcUUID1TimeBased->node());

	DEEP_LOG_INFO(OTHER, "pcUUIDTimeBased timestamp = %llx\n", pcUUID1TimeBased->timestamp());

	return 0;
}
