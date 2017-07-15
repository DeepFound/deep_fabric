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
#ifndef CXX_UTIL_CONCURRENT_LOCKS_REENTRANTUSERSPACELOCK_H_
#define CXX_UTIL_CONCURRENT_LOCKS_REENTRANTUSERSPACELOCK_H_ 

#include "cxx/util/concurrent/locks/Lock.h"

namespace cxx { namespace util { namespace concurrent { namespace locks {

class ReentrantUserSpaceLock /* : public Lock */ {

	private:
		volatile boolean m_lock;
		int m_refcnt;

		pthread_t m_owner;

	public:
		ReentrantUserSpaceLock(void):
			m_lock(false),
			m_refcnt(0),
			m_owner(null) {
		}

		FORCE_INLINE void lock() {
			pthread_t owner = pthread_self();
			if (m_owner != owner) {
				uinttype state = 1;
				while (__sync_lock_test_and_set(&m_lock, true)) {
					Lock::yield(&state);
				}

				m_owner = owner;
			}

			m_refcnt++;
		}

		FORCE_INLINE boolean tryLock() {
			boolean result = true;

			pthread_t owner = pthread_self();
			if (m_owner != owner) {
				result = (__sync_lock_test_and_set(&m_lock, true) == 0);
			}

			if (result == true) {
				m_owner = owner;
				m_refcnt++;
			}

			return result;
		}

		FORCE_INLINE void unlock() {
			if (--m_refcnt == 0) {
				m_owner = null;

				// XXX: __sync_lock_release(&m_lock, false);

				(void) __sync_lock_test_and_set(&m_lock, false);
			}
		}
};

} } } } // namespace

#endif /*CXX_UTIL_CONCURRENT_LOCKS_REENTRANTUSERSPACELOCK_H_*/