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
#ifndef COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_QUICKLIST_H_
#define COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_QUICKLIST_H_

#include "cxx/lang/types.h"
#include "cxx/util/concurrent/locks/UserSpaceLock.h"

using namespace cxx::util::concurrent::locks;

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

template<typename T>
class QuickListEntry {
	private:
		T m_entry;
		QuickListEntry<T>* m_next;

	public:
		QuickListEntry(T entry):
			m_entry(entry),
			m_next(null) {
		}

		virtual ~QuickListEntry() {
			delete m_entry;
		}

		FORCE_INLINE T getEntry() const {
			return m_entry;
		}

		FORCE_INLINE void setNext(QuickListEntry<T>* next) {
			m_next = next;
		}
	
		FORCE_INLINE QuickListEntry<T>* getNext() const {
			return m_next;
		}
};

template<typename T>
class QuickList {
	private:
		UserSpaceLock m_lock;

		QuickListEntry<T>* m_head;
		QuickListEntry<T>* m_tail;

		uinttype m_size;
	public:
		QuickList():
			m_lock(),
			m_head(null),
			m_tail(null),
			m_size(0) { 

		}

		FORCE_INLINE void disown() {
			m_head = null;
			m_tail = null;
			m_size = 0;
		}

		FORCE_INLINE void clear() {
			m_lock.lock(); 
			{              
				while (m_head != null) {
					m_tail = m_head->getNext(); 
					delete m_head;

					m_head = m_tail;
				}

				m_head = null;
				m_tail = null;
			}
			m_lock.unlock();
		}

		FORCE_INLINE QuickListEntry<T>* getHead() const {
			return m_head;
		}

		FORCE_INLINE QuickListEntry<T>* getTail() const {
			return m_tail;
		}

		FORCE_INLINE uinttype getSize() const {
			return m_size;
		}

		FORCE_INLINE boolean advanceHead() {
			if (m_head->getNext() != null) {
				QuickListEntry<T>* node = m_head;
				m_head = m_head->getNext();
				delete node;

				m_size--;
				return true;
			}
			
			return false;
		}

		FORCE_INLINE void addEntry(T data) {
			QuickListEntry<T>* node = new QuickListEntry<T>(data);

			m_lock.lock();	
			{
				if (m_head == null) {
					m_head = node;
					m_tail = node;
				}

				if (m_tail != node) {
					m_tail->setNext(node);
					m_tail = node;
				}

				m_size++;
			}
			m_lock.unlock();
		}
	
		FORCE_INLINE void append(QuickList<T>* list) {
			if (m_head == null) {
				m_lock.lock();
				{
					if (m_head == null) {
						m_head = list->getHead();
						m_tail = list->getTail();
						m_size = list->getSize();

						m_lock.unlock();
						return;
					}
				}
				m_lock.unlock();
			}

			m_lock.lock();
			{
				QuickListEntry<T>* head = list->getHead();
				QuickListEntry<T>* tail = list->getTail();

				m_tail->setNext(head);
				m_tail = tail;

				m_size += list->getSize();
			}
			m_lock.unlock();
		}

		virtual ~QuickList() {
			clear();
		}
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DB_STORE_RELATIVE_DISTRIBUTED_QUICKLIST_H_ */
