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
#ifndef COM_DEEPIS_COMMUNICATION_COMMON_THREADPOOL_H_
#define COM_DEEPIS_COMMUNICATION_COMMON_THREADPOOL_H_

#include "cxx/lang/Thread.h"
#include "cxx/lang/Runnable.h"
#include "cxx/util/concurrent/locks/UserSpaceLock.h"
#include "cxx/util/concurrent/atomic/AtomicInteger.h"

using namespace cxx::lang;
using namespace cxx::util::concurrent::locks;
using namespace cxx::util::concurrent::atomic;

namespace com { namespace deepis { namespace communication { namespace common {

// TODO: this is just a place holder for thread mangement for now

class ThreadPool {
	public:
		static const uinttype DEFAULT_THREADS = 100;
		/* TODO
                static UserSpaceLock s_instanceLock;
		static AtomicInteger s_position;
		*/

		static ThreadPool* getInstance(uinttype numThreads = DEFAULT_THREADS) {
			// XXX: this is only thread safe in g++ / c++ 11
			static ThreadPool theInstance(numThreads);
			return &theInstance;
		}

		Thread* allocThread(Runnable* runnable) const {
			Thread* thread = new Thread(runnable);
			return thread;
		}

		void freeThread(Thread* thread) {
			thread->join();
			delete thread;
		}

	private:
		ThreadPool(uinttype numThreads) {

		}

		virtual ~ThreadPool() {

		}
};

/* TODO
UserSpaceLock ThreadPool::s_instanceLock = UserSpaceLock();
AtomicInteger ThreadPool::s_position = AtomicInteger(0);
*/

} } } } // namespace

#endif /* COM_DEEPIS_COMMUNICATION_COMMON_THREADPOOL_H_ */
