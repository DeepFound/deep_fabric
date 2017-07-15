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
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "cxx/lang/Long.h"
#include "cxx/lang/String.h"
#include "cxx/lang/System.h"

#include "cxx/util/Logger.h"

#include "cxx/util/Time.h"

using namespace cxx::lang;
using namespace cxx::util;

int main(int argc, char** argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	uint64_t iStart = time::GetNanos<time::SteadyClock>();
	uint64_t iEnd   = time::GetNanos<time::SteadyClock>();
	uint64_t iDelta = (iEnd - iStart);

	DEEP_LOG(INFO, OTHER, "time::GetNanos<cxx::util::time::SteadyClock>() Elapsed: %"PRIu64" nanoseconds\n", iDelta);

	iStart = time::GetSeconds<time::SteadyClock>();
	sleep(1);
	iEnd   = time::GetSeconds<time::SteadyClock>();
	iDelta = (iEnd - iStart);

	DEEP_LOG(INFO, OTHER, "(sleep 1sec) time::GetSeconds<cxx::util::time::SteadyClock>() Elapsed: %"PRIu64" seconds\n", iDelta);

	iStart = time::GetMillis<time::SteadyClock>();
	sleep(1);
	iEnd   = time::GetMillis<time::SteadyClock>();
	iDelta = (iEnd - iStart);

	DEEP_LOG(INFO, OTHER, "(sleep 1sec) time::GetMillis<cxx::util::time::SteadyClock>() Elapsed: %"PRIu64" ms\n", iDelta);

	iStart = time::GetMicros<time::SteadyClock>();
	sleep(1);
	iEnd   = time::GetMicros<time::SteadyClock>();
	iDelta = (iEnd - iStart);

	DEEP_LOG(INFO, OTHER, "(sleep 1sec) time::GetMicros<cxx::util::time::SteadyClock>() Elapsed: %"PRIu64" us\n", iDelta);


	iStart = time::GetSeconds<time::SystemClock>();
	sleep(1);
	iEnd   = time::GetSeconds<time::SystemClock>();
	iDelta = (iEnd - iStart);

	DEEP_LOG(INFO, OTHER, "(sleep 1sec) time::GetSeconds<cxx::util::time::SystemClock>() Elapsed: %"PRIu64" seconds\n", iDelta);

	iStart = time::GetMillis<time::SystemClock>();
	sleep(1);
	iEnd   = time::GetMillis<time::SystemClock>();
	iDelta = (iEnd - iStart);

	DEEP_LOG(INFO, OTHER, "(sleep 1sec) time::GetMillis<cxx::util::time::SystemClock>() Elapsed: %"PRIu64" ms\n", iDelta);

	iStart = time::GetMicros<time::SystemClock>();
	sleep(1);
	iEnd   = time::GetMicros<time::SystemClock>();
	iDelta = (iEnd - iStart);

	DEEP_LOG(INFO, OTHER, "(sleep 1sec) time::GetMicros<cxx::util::time::SystemClock>() Elapsed: %"PRIu64" us\n", iDelta);


	return 0;
}
