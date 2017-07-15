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
#ifndef CXX_LANG_IO_MSGQUEUE_H_
#define CXX_LANG_IO_MSGQUEUE_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include "cxx/io/EOFException.h"
#include "cxx/lang/types.h"

#include "q.h"
#include "cxx/io/SocketCommon.h"

namespace cxx { namespace io {

/**
 * Exceptions to handle errors beyond the LinkManager API
 */
class MsgQueueException : public Exception {
public:
	MsgQueueException(void) {
	}

	MsgQueueException(const char* message)
		: Exception(message) {
	}
}; /** MsgQueueException */

class MsgQueue : public cxx::lang::Object {
public:
	MsgQueue(const uinttype iQueueSize,
		 boolean        bEnableEventFd = true);
	virtual ~MsgQueue();

	inttype eventFd() { return m_iEventFd; }

	inttype enqueue(voidptr pMsg);
	inttype dequeue(voidptr& pMsg);

	boolean isEmpty();
	boolean isFull();

	inttype enablePoller(void);
	inttype poller(ulongtype iTimeout, boolean& bHasWork);

private:
	inttype            m_iEventFd;
	q_t*               m_pcQueue;
	inttype            m_iEpollFd;
	struct epoll_event m_stEvents ;

}; /** MsgQueue */

} /** io */
} /** cxx */

/**
 * MsgQueue
 * Creates a Message Queue of the max size specified. The
 * size must be a power of 2.
 * @param iQueueSize - The size of the Q.
 * @param bEnableEventFd - By default (true) creates an Event FD to signal
 * items that are enqueued.
 * Can throw an exception on errors encountered.
 */
FORCE_INLINE cxx::io::MsgQueue::MsgQueue(const uinttype iQueueSize,
					 boolean        bEnableEventFd) :
	m_iEventFd(-1),
	m_pcQueue(0),
	m_iEpollFd(-1) {

	memset(&m_stEvents, 0, sizeof(m_stEvents));

	qerr_t iStatus = q_create(&m_pcQueue, iQueueSize);
	if (QERR_OK != iStatus) {
		if (QERR_BADSIZE == iStatus) {
			throw MsgQueueException("Bad size (not a power of 2 or too big).");
		} else {
			throw MsgQueueException("Error creating the Queue.");
		}
	}

	if (true == bEnableEventFd) {
		errno = 0;

		m_iEventFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
		if (-1 == m_iEventFd) {
			throw MsgQueueException("Unable to create the eventFD.");
		}
	}
}

/**
 * MsgQueue
 * Destructor.
 */
FORCE_INLINE cxx::io::MsgQueue::~MsgQueue() {
	if (-1 != m_iEventFd) {
		close(m_iEventFd);
		m_iEventFd = -1;
	}

	if (-1 != m_iEpollFd) {
		close(m_iEpollFd);
		m_iEpollFd = -1;
	}

	qerr_t iStatus = q_delete(m_pcQueue);
	if (QERR_OK != iStatus) {
		throw MsgQueueException("Error trying to delete the Queue.");
	}

	m_pcQueue = 0;
}

/**
 * enqueue
 * Enqueue the message pointer into the Queue. This will
 * tap the eventfd to ensure the receiver knows to attempt a
 * dequeue.
 * @param pMsg - reference to a message to enqueue.
 * @return 0 on success, -1 Queue is full, anything else is an error.
 * Can throw execeptions on internal errors.
 */
FORCE_INLINE inttype cxx::io::MsgQueue::enqueue(voidptr pMsg) {
	if (1 == q_is_full(m_pcQueue)) {
		/** Queue is full */
		return -1;
	}

	/** Enqueue the message first */
	if (QERR_OK != q_enq(m_pcQueue, pMsg)) {
		throw MsgQueueException("Issues with queueing the message.");
	}

	/** Now tap the eventFD */
	if (-1 != m_iEventFd) {
		errno = 0;

		ulongtype iTapFd  = 1;
		size_t    iRetVal = write(m_iEventFd, &iTapFd, sizeof(ulongtype));
		if (sizeof(ulongtype) != iRetVal) {
			throw MsgQueueException("Unable to tap the eventFd.");
		}
	}

	return 0;
}

/**
 * dequeue
 * Dequeues exactly one message from the queue.
 * @param pMsg - reference to a message. Will not be null.
 * @return 0 on success, -1 on queue empty, anything else is an error.
 * Can throw execeptions on internal errors.
 */
FORCE_INLINE inttype cxx::io::MsgQueue::dequeue(voidptr& pMsg) {
	if (-1 != m_iEventFd) {
		/** First consume the eventFd tap. */
		errno = 0;

		ulongtype iTapFd = 0;
		size_t iRead = read(m_iEventFd, &iTapFd, sizeof(ulongtype));
		(void) iRead;
	}

	/**
	 * Regardless of the value check for empty and dequeue the
	 * message.
	 */
	if (1 == q_is_empty(m_pcQueue)) {
		return -1;
	}

	if (QERR_OK != q_deq(m_pcQueue, &pMsg)) {
		throw MsgQueueException("Issues with dequeueing the message.");
	}

	return 0;
}

/**
 * isEmpty
 * Returns true if the queue is empty.
 * @return  true is the queue is empty.
 */
FORCE_INLINE boolean cxx::io::MsgQueue::isEmpty() {
	return (1 == q_is_empty(m_pcQueue)) ? true : false;
}

/**
 * isFull
 * Returns true if the queue is full.
 * @return true if the queue is full.
 */
FORCE_INLINE boolean cxx::io::MsgQueue::isFull() {
	return (1 == q_is_full(m_pcQueue)) ? true : false;
}

/**
 * enablePoller
 * Enables epolling of events on this queue.
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::MsgQueue::enablePoller(void) {
	inttype iFd = eventFd();
	errno = 0;

	m_iEpollFd = epoll_create(cxx::io::MAXCONN);
	if (-1 == m_iEpollFd) {
		return -1;
	}

	errno = 0;

	inttype iStatus = cxx::io::modifyEpollContext(m_iEpollFd, EPOLL_CTL_ADD, iFd, EPOLLIN, 0);
	if (0 != iStatus) {
		return -2;
	}

	return 0;
}

/**
 * poller
 * Poll for (iTimeout) milliseconds for any work in the queue. If
 * @param iTimeout - number of milliseconds to block waiting for an
 * event. 0 returns immediately. MaxINT will block forever.
 * @param bHasWork - true if the queue is not empty
 * @return 0 on success.
 */
FORCE_INLINE inttype cxx::io::MsgQueue::poller(ulongtype iTimeout, boolean& bHasWork) {
	if (-1 == m_iEpollFd) {
		/** Run hot */
		bHasWork = (false == isEmpty()) ? true : false;
		return 0;
	}

	errno = 0;

	inttype iEventCount = epoll_wait(m_iEpollFd, &m_stEvents, 1, iTimeout);
	if (-1 == iEventCount) {
		if (EINTR != errno) {
			return -1;
		}
	}

	/** Whether we have any events or not, check the Queue. */
	bHasWork = (false == isEmpty()) ? true : false;;

	return 0;
}

#endif /** CXX_LANG_IO_MSGQUEUE_H_ */
