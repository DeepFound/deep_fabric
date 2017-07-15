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
#include "cxx/io/LinkManager.h"
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

// Test 1 ////////////////////////////////////////////////////////////////////////////////

namespace testOne {

	class sendBlk : public messageBlock {
	public:
		virtual void canFree(voidptr pData) {
			DEEP_LOG_INFO(OTHER, "canFree: %p\n", pData);
		}
	};

	class reqCb : public cxx::io::socketCallback {
	public:
		reqCb() : m_cMsg("Yo!") {
		}
	
		virtual ~reqCb() {
		}
	
		void readEvent(voidptr                 pData,
			       ulongtype               iSize,
			       voidptr                 sockId,
			       cxx::io::messageBlock*& pcResp,
			       voidptr                 lmContext) {

			cxx::lang::String cMsg((const char *)pData, iSize);

			DEEP_LOG_INFO(OTHER, "Request read: %s\n", cMsg.c_str());

			m_cResp.addMessage(m_cMsg);
			m_cResp.addMessage(m_cMsg);
			m_cResp.addMessage(m_cMsg);
			m_cResp.addMessage(m_cMsg);

			pcResp = &m_cResp;
		}

		cxx::lang::String  m_cMsg;
		messageBlock m_cResp;
	};

	class respCb : public cxx::io::socketCallback {
	public:
		respCb() : m_iCount(0) {
		}

		void readEvent(voidptr                 pData,
			       ulongtype               iSize,
			       voidptr                 sockId,
			       cxx::io::messageBlock*& pcResp,
			       voidptr                 lmContext) {

			cxx::lang::String cMsg((const char *)pData, iSize);

			DEEP_LOG_INFO(OTHER, "Response read: %s\n", cMsg.c_str());
			m_iCount++;
		}

		int m_iCount;
	};

	class respCb2 : public cxx::io::socketCallback {
	public:
		void readEvent(voidptr                   pData,
			       ulongtype                  iSize,
			       voidptr                   sockId,
			       cxx::io::messageBlock*& pcResp) {

			cxx::lang::String cMsg((const char *)pData, iSize);

			DEEP_LOG_ERROR(OTHER, "Other Response Cb: %s\n", cMsg.c_str());
			exit(-1);
		}
	};

	int run() {
		cxx::io::LinkManager cMgr;
		reqCb    cReqCb;
		respCb   cRespCb;
		voidptr  serverSocket = 0;
		voidptr  clientSocket = 0;

		try {
			cMgr.addRepSocket("tcp://127.0.0.1:10000", &cReqCb,  serverSocket);
			cMgr.addReqSocket("tcp://127.0.0.1:10000", &cRespCb, clientSocket);

			sendBlk     cMsgBlk;
			cxx::lang::String cMsg = "Hello World!";

			cMsgBlk.addMessage(cMsg);
			cMgr.send(clientSocket, &cMsgBlk);

			int i=0;

			while (true) {
				cMgr.poll(1000);
				i++;

				if (4 <= cRespCb.m_iCount) {
					break;
				}

				if (10 <= i) {
					DEEP_LOG_ERROR(OTHER, "Stuck on the poll loop for too long!\n");
					exit(-1);
				}
			}

		} catch (exception& e) {
			std::cout << e.what() << std::endl;
		}

		return 0;
	}
} // test 1

// Test 2 ////////////////////////////////////////////////////////////////////////////////

namespace testTwo {

	static const ulongtype PAYLOAD_SIZE = (4096);
	static const ulongtype MAX_SIZE     = (1024 * 1024 * 1024);
	static const ulongtype NUM_PAYLOADS = (MAX_SIZE / PAYLOAD_SIZE);
	static const ulongtype BATCH_SIZE   = ((ulongtype)(32));
	static const ulongtype FREE_LIST    = (BATCH_SIZE<<1);

	class REPThread {
	public:
		REPThread();
		virtual ~REPThread();

		static void run(voidptr pThis);
		void run(void);

	private:
		// Data Structs/Methods
		class repCb : public cxx::io::socketCallback {
		public:
			repCb(REPThread* parent);

			void readEvent(voidptr                 pData,
				       ulongtype               iSize,
				       voidptr                 sockId,
				       cxx::io::messageBlock*& pcResp,
				       voidptr                 lmContext);

			REPThread* m_pcParent;
		};

		friend class repCb;

		class sendBlk : public cxx::io::messageBlock {
		public:
			sendBlk(REPThread* pcParent);
			virtual void canFree(voidptr pData);

			REPThread* m_pcParent;
		};

		void terminate();
		void startBatch();
		void nextBatch();
		void freeBuffer(ubytetype* pcData);

		// Private Variables
		cxx::io::LinkManager    m_cMgr;
		voidptr                 m_RepSock;
		repCb                   m_cRepCb;
		bool                    m_bRun;
		ulongtype               m_iCount;
		ubytetype               m_iBuffers[FREE_LIST][PAYLOAD_SIZE];
		std::queue<ubytetype* > m_iBuffers2;
		ubytetype               m_iFinish;
		sendBlk                 m_cDataMsg;
		ulongtype               m_iAvail;

	}; // REPThread


	// REPThread::sendBlk
	inline REPThread::sendBlk::sendBlk(REPThread* pcParent) : m_pcParent(pcParent) {
	}

	inline void REPThread::sendBlk::canFree(voidptr pData) {
		ubytetype* piData = static_cast<ubytetype*>(pData);
		m_pcParent->freeBuffer(piData);
	}

	// REPThread::repCb

	inline REPThread::repCb::repCb(REPThread* parent) : m_pcParent(parent) {
	}

	inline void REPThread::repCb::readEvent(voidptr                 pData,
						ulongtype               iSize,
						voidptr                 sockId,
						cxx::io::messageBlock*& pcResp,
						voidptr                 lmContext) {
		cxx::lang::String cMsg((const char *)pData, iSize);
		if (0 == cMsg.compare("REQ")) {
			DEEP_LOG_INFO(OTHER, "[RX] REQ Command.\n");
			m_pcParent->startBatch();
		} else if (0 == cMsg.compare("NEXT")) {
			//DEEP_LOG_INFO(OTHER, "[RX] NEXT Command.\n");
			m_pcParent->nextBatch();
		} else if (0 == cMsg.compare("END")) {
			DEEP_LOG_INFO(OTHER, "[RX] END Command.\n");
			m_pcParent->terminate();
		} else {
			DEEP_LOG_ERROR(OTHER, "[RX] Unknown Command: %s\n", cMsg.c_str());
		}

		cxx::io::closeLmContext(lmContext);
	}

	// REPThread

	inline REPThread::REPThread() : m_RepSock(0),
					m_cRepCb(this),
					m_bRun(true),
					m_iCount(0),
					m_iFinish(0xde),
					m_cDataMsg(this),
					m_iAvail(FREE_LIST) {
		for (ulongtype i=0; i<FREE_LIST; i++) {
			ubytetype* piData = m_iBuffers[i];
			memset(piData, 0xfa, PAYLOAD_SIZE);
		}

		for (ulongtype i=0; i<FREE_LIST; i++) {
			ubytetype* piData = new ubytetype[PAYLOAD_SIZE << 1];
			m_iBuffers2.push(piData);
		}

		ulongtype status = m_cMgr.addRepSocket("tcp://127.0.0.1:10000", &m_cRepCb, m_RepSock);
		if (0 != status) {
			DEEP_LOG_ERROR(OTHER, "unable to create the REP socket\n");
			abort();
		}

#if 0
		status = m_cMgr.close(m_RepSock);
		if (0 != status) {
			DEEP_LOG_ERROR(OTHER, "unable to close the REP socket\n");
			abort();
		}

		status = m_cMgr.addRepSocket("tcp://127.0.0.1:10000", &m_cRepCb, m_RepSock);
		if (0 != status) {
			DEEP_LOG_ERROR(OTHER, "unable to create the REP socket\n");
			abort();
		}
#endif
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

	inline void REPThread::startBatch() {
		m_iCount         = 0;

		nextBatch();
	}

	inline void REPThread::nextBatch() {
		if (NUM_PAYLOADS <= m_iCount) {
			m_cDataMsg.addMessage(&m_iFinish, 1);

		} else {
			ulongtype iBatch = std::min(m_iAvail, BATCH_SIZE);
			for (ulongtype i=0; i<iBatch; i++) {
				ubytetype* piBlock = m_iBuffers2.front();
				m_iBuffers2.pop();

				if (i == iBatch-1) {
					piBlock[0] = 0xcc;
				} else {
					piBlock[0] = 0xfa;
				}

				m_cDataMsg.addMessage(piBlock, PAYLOAD_SIZE);

				m_iCount++;
				m_iAvail--;
			}
		}

		m_cMgr.send(m_RepSock, &m_cDataMsg);
	}

	inline void REPThread::freeBuffer(ubytetype* pcData) {
		m_iAvail++;
		m_iBuffers2.push(pcData);
	}

	// ReqThread
	class REQThread {
	public:
		REQThread();
		virtual ~REQThread();

		static void run(voidptr pThis);
		void run(void);

	private:
		// Data Structs/Methods
		class reqCb : public cxx::io::socketCallback {
		public:
			reqCb(REQThread* parent);

			void readEvent(voidptr                 pData,
				       ulongtype               iSize,
				       voidptr                 sockId,
				       cxx::io::messageBlock*& pcResp,
				       voidptr                 lmContext);

			REQThread* m_pcParent;
		};

		friend class reqCb;

		class sendBlk : public cxx::io::messageBlock {
		public:
			virtual void canFree(voidptr pData) {
				//DEEP_LOG_INFO(OTHER, "canFree: %p\n", pData);
			}
		};

		void terminate();
		void countPayload(voidptr pData, ulongtype iSize);

		// Private Variables
		cxx::io::LinkManager  m_cMgr;
		voidptr               m_ReqSock;
		reqCb                 m_cReqCb;
		bool                  m_bRun;
		sendBlk               m_cCmd;
		cxx::lang::String     m_cCmdStr;
		ulongtype             m_iCount;
		ulongtype             m_iByteCount;
		ulongtype             m_iStartTime;

	}; // REQTHread

	// REQThread::reqCb

	inline REQThread::reqCb::reqCb(REQThread* parent) : m_pcParent(parent) {
	}

	inline void REQThread::reqCb::readEvent(voidptr                 pData,
						ulongtype               iSize,
						voidptr                 sockId,
						cxx::io::messageBlock*& pcReq,
						voidptr                 lmContext) {

		m_pcParent->countPayload(pData, iSize);

		cxx::io::closeLmContext(lmContext);
	}


	// REQThread
	inline REQThread::REQThread() : m_ReqSock(0),
					m_cReqCb(this),
					m_bRun(true),
					m_cCmdStr("REQ"),
					m_iCount(0),
					m_iByteCount(0),
					m_iStartTime(0) {
		m_cMgr.addReqSocket("tcp://127.0.0.1:10000", &m_cReqCb, m_ReqSock);

		m_cCmd.addMessage(m_cCmdStr);
		m_cMgr.send(m_ReqSock, &m_cCmd);

		m_cCmdStr = "NEXT";
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

	inline void REQThread::countPayload(voidptr pData, ulongtype iSize) {
		ubytetype* piBuffer = static_cast<ubytetype*>(pData);

		if (0xde == piBuffer[0]) {
			// End command
			ulongtype iEndTime = time::GetNanos<time::SteadyClock>();
			ulongtype iDelta   = iEndTime - m_iStartTime;

			doubletype fSeconds = (doubletype)iDelta/time::SECONDS;

			m_cCmdStr = "END";

			cxx::lang::String cPrettyValue;
			prettyprint::Bytes<ulongtype>(m_iByteCount, cPrettyValue);

			DEEP_LOG_INFO(OTHER,
				      "[RX] Complete: %llu Payloads, Size: %s in %fs\n",
				      m_iCount,
				      cPrettyValue.c_str(),
				      fSeconds);

			exit(0);
		} else {
			if (0 == m_iCount) {
				m_iStartTime = time::GetNanos<time::SteadyClock>();
			}

			m_iCount++;
			m_iByteCount += iSize;
		}

		if (0xcc ==  piBuffer[0]) {
			m_cCmd.addMessage(m_cCmdStr);
			m_cMgr.send(m_ReqSock, &m_cCmd);
		}
	}

	// Test Run
	int run() {
		// Start the REP Thread
		txNativeThread cRepThread;
		cRepThread.start(REPThread::run, 0, "REPThread");

		//txNativeThread cReqThread;
		//cRepThread.start(REQThread::run, 0, "REQThread1");

		// Start the REQ2 Thread
		REQThread cReq;
		cReq.run();

		return 0;
	}

}

int main(int argc, char **argv) {
	cxx::util::Logger::enableLevel(cxx::util::Logger::INFO);
	cxx::util::Logger::enableLevel(cxx::util::Logger::DEBUG);
	cxx::util::Logger::enableLevel(cxx::util::Logger::WARN);
	cxx::util::Logger::enableLevel(cxx::util::Logger::ERROR);

	/** Unit test 1 */
	testOne::run();
	testTwo::run();

	return 0;
}
