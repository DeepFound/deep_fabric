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
#ifndef CXX_UTIL_TIME
#define CXX_UTIL_TIME

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

namespace cxx { namespace util { namespace time {

const uint64_t SECONDS      = 1000000000;
const uint64_t TENTHS       = 100000000;
const uint64_t HUNDRETHS    = 10000000;
const uint64_t MILLISECONDS = 1000000;
const uint64_t MICROSECONDS = 1000;
const uint64_t NANOSECONDS  = 1;

/**
 * Defines a way to get the current Monotonic clock tick.
 * This clock will never be altered by NTP, or any changes to the
 * system wide clock, like setting a new time, changing timezone or
 * leapseconds changes.
 */
class SteadyClock {
public:
	static inline uint64_t Now() {
		struct timespec cTimeSpec = { 0 };

		clock_gettime(CLOCK_MONOTONIC_RAW,
			      &cTimeSpec);

		return (static_cast<uint64_t>(cTimeSpec.tv_sec) * SECONDS +
			static_cast<uint64_t>(cTimeSpec.tv_nsec));
	}

}; // SteadyClock

/**
 * Defines a way to get the current System-wide Realtime clock tick.
 * This clock *IS* effected by NTP, or changes like setting a new
 * time, changing timezone or leapseconds changes.
 * Do not use in realtime algorithms where accurate ticks are critical.
 */
class SystemClock {
public:
	static inline uint64_t Now() {
		struct timespec cTimeSpec = { 0 };

		clock_gettime(CLOCK_REALTIME,
			      &cTimeSpec);

		return (static_cast<uint64_t>(cTimeSpec.tv_sec) *
			SECONDS +
			static_cast<uint64_t>(cTimeSpec.tv_nsec));
	}

}; // SystemClock

template <typename T>
inline uint64_t GetNanos() {
	static T clock;
	return clock.Now() / NANOSECONDS;
}

template <typename T>
inline uint64_t GetMillis() {
	static T clock;
	return clock.Now() / MILLISECONDS;
}

template <typename T>
inline uint64_t GetMicros() {
	static T clock;
	return clock.Now() / MICROSECONDS;
}

template <typename T>
inline uint64_t GetSeconds() {
	static T clock;
	return clock.Now() / SECONDS;
}

} // time
} // util
} // cxx

#endif /** CXX_UTIL_TIME */
