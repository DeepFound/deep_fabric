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
#include "cxx/util/PrettyPrint.h"
#include "cxx/util/Time.h"
#include "cxx/io/MessageManager.h"

#include <algorithm>
#include <sstream>
#include <iostream>
#include <queue>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <txtimer.h>
#include <txthrdmgr.h>

using namespace cxx::lang;
using namespace cxx::util;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace testOne {
	static const ulongtype PAYLOAD_SIZE = (4096);
	static const ulongtype MAX_SIZE     = (1024 * 1024 * 1024);
	static const ulongtype NUM_PAYLOADS = (MAX_SIZE / PAYLOAD_SIZE);
	static const ulongtype BATCH_SIZE   = ((ulongtype)(128));
	static const ulongtype FREE_LIST    = (BATCH_SIZE<<1);

	static const uinttype REQ_MSG_ID          = 0x80808080;
	static const uinttype REQ_UINT32_CMD      = 1;
	static const uinttype REQ_UINT32_CMD_REQ  = 1;
	static const uinttype REQ_UINT32_CMD_NEXT = 2;
	static const uinttype REQ_UINT32_CMD_END  = 3;

	static const uinttype REP_MSG_ID          = 0x80808081;
	static const uinttype REP_MSG_ZERO64      = 0;
	static const uinttype REP_MSG_UINT32_SEQ  = 1;
	static const uinttype REP_MSG_LAST        = 2;
	static const uinttype REP_MSG_RAW         = 3;

	class REPThread {
	public:
		REPThread();
		virtual ~REPThread();

		static void run(voidptr pThis);
		void run(void);

	private:
		// Data Structs/Methods
		class repCb : public cxx::io::MessageCallback {
		public:
			repCb(REPThread* parent);

			virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
						  voidptr                          sendId);

			REPThread* m_pcParent;
		};

		friend class repCb;

		struct delegate : public cxx::io::notifications {
			delegate(REPThread* parent);
			virtual void canFree(voidptr pData);

			REPThread* m_pcParent;
		}; /** delegate */

		friend class delegate;

		void terminate();
		void startBatch(cxx::io::encodeProtocol::reader* pcRxMsg);
		void nextBatch(cxx::io::encodeProtocol::reader* pcRxMsg);
		void freeBuffer(ubytetype* pcData);

		// Private Variables
		cxx::io::MessageManager m_cMgr;
		voidptr                 m_RepSock;
		repCb                   m_cRepCb;
		boolean                 m_bRun;
		uinttype                m_iCount;
		ubytetype               m_iBuffers[FREE_LIST][PAYLOAD_SIZE];
		std::queue<ubytetype* > m_iBuffers2;
		ulongtype               m_iAvail;
		delegate                m_cDelegate;

	}; // REPThread

	// REPThread::delegate
	inline REPThread::delegate::delegate(REPThread* parent) : m_pcParent(parent) {
	}

	inline void REPThread::delegate::canFree(voidptr pData) {
		ubytetype* pcBuffer = reinterpret_cast<ubytetype*>(pData);
		m_pcParent->freeBuffer(pcBuffer);
	}

	// REPThread::repCb

	inline REPThread::repCb::repCb(REPThread* parent) : m_pcParent(parent) {
	}

	inline uinttype REPThread::repCb::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
						   voidptr                          sendId) {
		uinttype iCmd = 0;

		pcRxMsg->getUint32Field(REQ_UINT32_CMD, iCmd);

		switch (iCmd) {
			case REQ_UINT32_CMD_REQ:
				DEEP_LOG_INFO(OTHER, "[RX] REQ Command.\n");
				m_pcParent->startBatch(pcRxMsg);
				break;
			case REQ_UINT32_CMD_NEXT:
				//DEEP_LOG_INFO(OTHER, "[RX] NEXT Command.\n");
				m_pcParent->nextBatch(pcRxMsg);
				break;
			case REQ_UINT32_CMD_END:
				DEEP_LOG_INFO(OTHER, "[RX] END Command.\n");
				m_pcParent->terminate();
				break;
			default:
				DEEP_LOG_ERROR(OTHER, "[RX] Unknown Command: %i\n", iCmd);
				break;
		}

		return 0;
	}

	// REPThread

	inline REPThread::REPThread() : m_RepSock(0),
					m_cRepCb(this),
					m_bRun(true),
					m_iCount(NUM_PAYLOADS),
					m_iAvail(FREE_LIST),
					m_cDelegate(this) {
		for (ulongtype i=0; i<FREE_LIST; i++) {
			ubytetype* piData = m_iBuffers[i];
			memset(piData, 0xfa, PAYLOAD_SIZE);
		}

		for (ulongtype i=0; i<FREE_LIST; i++) {
			ubytetype* piData = new ubytetype[PAYLOAD_SIZE << 1];
			m_iBuffers2.push(piData);
		}

		ulongtype status = m_cMgr.registerNotifications(&m_cDelegate);
		if (0 != status) {
			DEEP_LOG_ERROR(OTHER, "Unable to register the delegate callback.\n");
			abort();
		}

		m_cMgr.addRepSocket("tcp://127.0.0.1:10000", m_RepSock);

		status = m_cMgr.registerMessageId(REQ_MSG_ID, &m_cRepCb);
		if (0 != status) {
			DEEP_LOG_ERROR(OTHER, "Unable to register: %x message ID.\n", REQ_MSG_ID);
			abort();
		}
	}

	inline REPThread::~REPThread() {
	}

	inline void REPThread::run(voidptr pThis) {
		REPThread cRep;
		cRep.run();
	}

	inline void REPThread::run(void) {
		try {
			while (true == m_bRun) {
				m_cMgr.poll(10);
			}

		} catch (exception& e) {
			DEEP_LOG_ERROR(OTHER, "%s\n", e.what());
		}

		DEEP_LOG_INFO(OTHER, "REPThread has exited.\n");
	}

	inline void REPThread::terminate() {
		m_bRun = false;
	}

	inline void REPThread::startBatch(cxx::io::encodeProtocol::reader* pcRxMsg) {
		m_iCount = 0;
		nextBatch(pcRxMsg);
	}

	inline void REPThread::nextBatch(cxx::io::encodeProtocol::reader* pcRxMsg) {
		ulongtype iBatch = std::min(m_iAvail, BATCH_SIZE);

		for (ulongtype i = 0; i < iBatch; i++) {
			uinttype iLast = (i == iBatch-1) ? true : false;
			uinttype iSize =
				cxx::io::encodeProtocol::writer::sizeOfInt64(0) +
				cxx::io::encodeProtocol::writer::sizeOfUint32(m_iCount) +
				cxx::io::encodeProtocol::writer::sizeOfUint32(iLast)    +
				cxx::io::encodeProtocol::writer::sizeOfRaw(PAYLOAD_SIZE);

			cxx::io::encodeProtocol::writer block(iSize, REP_MSG_ID);

			inttype iStatus = block.setInt64Field(REP_MSG_ZERO64, 0);
			if (0 != iStatus) {
				DEEP_LOG_ERROR(OTHER, "Failed to write int64 zero value: %d\n", iStatus);
				abort();
			}

			block.setUint32Field(REP_MSG_UINT32_SEQ, m_iCount);
			block.setRawField(REP_MSG_RAW, m_iBuffers[i], PAYLOAD_SIZE);
			block.setUint32Field(REP_MSG_LAST, iLast);

			m_cMgr.sendREP(*pcRxMsg, block);

			m_iCount++;
			m_iAvail--;
		}
	}

	inline void REPThread::freeBuffer(ubytetype* pcData) {
		m_iAvail++;
		m_iBuffers2.push(pcData);
	}

	///////////////
	// REQTHREAD //
	///////////////
	class REQThread {
	public:
		REQThread();
		virtual ~REQThread();

		static void run(voidptr pThis);
		void run(void);

	private:
		// Data Structs/Methods
		class reqCb : public cxx::io::MessageCallback {
		public:
			reqCb(REQThread* parent);

			virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
						  voidptr                          sendId);

			REQThread* m_pcParent;
		};

		friend class reqCb;

		void terminate();
		void countPayload(cxx::io::encodeProtocol::reader* pcRxMsg);

		// Private Variables
		cxx::io::MessageManager m_cMgr;
		voidptr                 m_ReqSock;
		reqCb                   m_cReqCb;
		boolean                 m_bRun;
		ulongtype               m_iCount;
		ulongtype               m_iByteCount;
		ulongtype               m_iStartTime;

	}; // REQTHread

	// REQThread::reqCb

	inline REQThread::reqCb::reqCb(REQThread* parent) : m_pcParent(parent) {
	}

	inline uinttype REQThread::reqCb::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
						   voidptr                          sendId) {

		m_pcParent->countPayload(pcRxMsg);
		delete pcRxMsg; pcRxMsg = 0;
		return 0;
	}


	// REQThread
	inline REQThread::REQThread() : m_ReqSock(0),
					m_cReqCb(this),
					m_bRun(true),
					m_iCount(0),
					m_iByteCount(0),
					m_iStartTime(0) {

		ulongtype status = m_cMgr.addReqSocket("tcp://127.0.0.1:10000", m_ReqSock);
		if (0 != status) {
			DEEP_LOG_ERROR(OTHER, "Unable to create socket.\n");
			abort();
		}

		status = m_cMgr.registerMessageId(REP_MSG_ID, &m_cReqCb);
		if (0 != status) {
			DEEP_LOG_ERROR(OTHER, "Unable to register callback.\n");
			abort();
		}

		uinttype iSize =
			cxx::io::encodeProtocol::writer::sizeOfUint32(REQ_UINT32_CMD_REQ);

		cxx::io::encodeProtocol::writer block(iSize, REQ_MSG_ID);
		block.setUint32Field(REQ_UINT32_CMD, REQ_UINT32_CMD_REQ);

		status = m_cMgr.sendREQ(m_ReqSock, block);
		if (0 != status) {
			DEEP_LOG_ERROR(OTHER, "Unable to send REQ message.\n");
			abort();
		}
	}

	inline REQThread::~REQThread() {
	}

	inline void REQThread::run(voidptr pThis) {
		REQThread cReq;
		cReq.run();
	}

	inline void REQThread::run(void) {
		try {
			while (true == m_bRun) {
				m_cMgr.poll(10);
			}

		} catch (exception& e) {
			DEEP_LOG_ERROR(OTHER, "%s\n", e.what());
		}

		DEEP_LOG_ERROR(OTHER, "REQThread has exited.\n");
	}

	inline void REQThread::terminate() {
		m_bRun = false;
	}

	inline void REQThread::countPayload(cxx::io::encodeProtocol::reader* pcRxMsg) {

		if (false == pcRxMsg->isReply()) {
			DEEP_LOG_ERROR(OTHER, "REPly received but REPLY flag is not set!\n");
			abort();
		}

		uinttype         iCount   = 0;
		const ubytetype* piBuffer = 0;
		uinttype         iSize    = 0;
		uinttype         iLast    = false;
		longtype         iZero64  = -1;

		if (0 != pcRxMsg->getInt64Field(REP_MSG_ZERO64, iZero64)) {
			DEEP_LOG_ERROR(OTHER, "Failed to read int64 zero value\n");
			abort();
		}

		if (0 != iZero64) {
			DEEP_LOG_ERROR(OTHER, "Failed to int64 is not a zero value=%lld\n", iZero64);
			abort();
		}

		pcRxMsg->getUint32Field(REP_MSG_UINT32_SEQ, iCount);
		pcRxMsg->getUint32Field(REP_MSG_LAST, iLast);
		pcRxMsg->getRawFieldRef(REP_MSG_RAW, piBuffer, iSize);

		uinttype iCmd = REQ_UINT32_CMD_NEXT;

		if (NUM_PAYLOADS == iCount) {
			/** The first buffer. */
			ulongtype iEndTime = time::GetNanos<time::SteadyClock>();
			ulongtype iDelta   = iEndTime - m_iStartTime;

			doubletype fSeconds = (doubletype)iDelta/time::SECONDS;

			iCmd = REQ_UINT32_CMD_END;

			cxx::lang::String cPrettyValue;
			prettyprint::Bytes<ulongtype>(m_iByteCount, cPrettyValue);

			DEEP_LOG_INFO(OTHER,
				      "[RX] Complete: %llu Payloads, Size: %s in %fs\n",
				      m_iCount,
				      cPrettyValue.c_str(),
				      fSeconds);

			exit(0);

		} else {
			if (0 == iCount) {
				/** The last buffer. */
				m_iStartTime = time::GetNanos<time::SteadyClock>();
			}

			m_iCount++;
			m_iByteCount += iSize;
		}

		if (true == iLast) {
			uinttype iSize =
				cxx::io::encodeProtocol::writer::sizeOfUint32(REQ_UINT32_CMD_REQ);

			cxx::io::encodeProtocol::writer block(iSize, REQ_MSG_ID);
			block.setUint32Field(REQ_UINT32_CMD, iCmd);

			errno = 0;

			ulongtype status = m_cMgr.sendREQ(m_ReqSock, block);
			if (0 != status) {
				DEEP_LOG_ERROR(OTHER, "Unable to send REQ message.\n");
				abort();
			}
		}
	}

	// Test Run
	int run() {
		// Start the REP Thread
		txNativeThread cRepThread;
		cRepThread.start(REPThread::run, 0, "REPThread");

		// Start the REQ2 Thread
		REQThread cReq;
		cReq.run();

		return 0;
	}


} /** testThree */

int main(int argc, char **argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	testOne::run();

	return 0;
}
