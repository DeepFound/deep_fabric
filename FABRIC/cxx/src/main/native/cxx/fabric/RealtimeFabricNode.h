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
#ifndef CXX_FABRIC_RTFABRICNODE_H_
#define CXX_FABRIC_RTFABRICNODE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cxx/util/Logger.h"
#include "cxx/lang/Runnable.h"
#include "cxx/lang/String.h"
#include "cxx/lang/Thread.h"
#include "cxx/util/concurrent/atomic/AtomicInteger.h"

#include "cxx/fabric/RealtimeFabric.h"
#include "cxx/replication/FileReplication.h"
#include "com/deepis/communication/fabricconnector/DummyBridge.h"
#include "com/deepis/communication/fabricconnector/CassiServiceBridge.h"
#include "com/deepis/communication/fabricconnector/FabricCassiMessageQueue.h"

using namespace com::deepis::communication::fabricconnector;
using namespace cxx::util::concurrent::atomic;

namespace cxx { namespace fabric {

class RealtimeFabricNode : public cxx::lang::Runnable, public cxx::lang::Object {
	struct peerList : public cxx::lang::Object {
		peerList(cxx::lang::String& cPeer) : m_cPeer(cPeer),
						     m_pcUUID(cxx::util::UUID::randomUUID()) {
		}

		virtual ~peerList() {
			delete m_pcUUID; m_pcUUID = 0;
		}

		cxx::lang::String            m_cPeer;
		cxx::util::UUID*             m_pcUUID;
	};

public:
	RealtimeFabricNode(uinttype      serverId,
			   const char*   ip,
			   const char*   peerIp,
			   loaderFunc    func,
			   voidptr       pCtx,
			   AtomicInteger* threadCounter = null) :
		   m_iServerId(serverId),
		   m_cIp(ip),
		   m_pcRtFabric(0),
		   m_bStop(false),
		   m_loaderFunc(func),
		   m_pCtx(pCtx),
		   m_threadCounter(threadCounter) {
		DEEP_LOG_INFO(OTHER, "Created Node: %s\n", m_cIp.c_str());

		if (0 != strlen(peerIp)) {
			addPeer(peerIp);
		}

		m_cPeers.setDeleteValue(true);
	}

	virtual ~RealtimeFabricNode() {
	}

	void run(void) {
		LoadListEntry cLoader[] = {
			m_loaderFunc, m_pCtx,
			0, 0
		};

		m_pcRtFabric = new cxx::fabric::RealtimeFabric(m_iServerId, m_cIp, cLoader);

		for (inttype i=0; i<m_cPeers.size(); i++) {
			peerList* pcPeer = m_cPeers.get(i);
			m_pcRtFabric->addPeer(pcPeer->m_cPeer, *(pcPeer->m_pcUUID));
		}

		if (m_loaderFunc == singletonCassiServiceBridge) {
			m_pcRtFabric->getFabricCassiMessageQueue()->registerTableMessageService((IMessageService*)m_pCtx);
		}

		while (false == m_bStop) {
			m_pcRtFabric->poll(100);
		}

		m_pcRtFabric ->stopService();
		delete m_pcRtFabric; m_pcRtFabric = 0;

		if (m_threadCounter != null) {
			m_threadCounter->decrementAndGet();
		}
	}

	void stopService(void) {
		m_bStop = true;
	}

	void addPeer(cxx::lang::String cPeer) {
		peerList* pcPeer = new peerList(cPeer);
		if (0 == pcPeer) {
			abort();
		}

		m_cPeers.add(pcPeer);
	}

private:
	uinttype 		     m_iServerId;
	cxx::lang::String            m_cIp;
	cxx::fabric::RealtimeFabric* m_pcRtFabric;
	boolean                      m_bStop;
	loaderFunc                   m_loaderFunc;
	voidptr                      m_pCtx;
	cxx::util::concurrent::atomic::AtomicInteger* m_threadCounter;
	cxx::util::ArrayList<peerList*> m_cPeers;

};


} } // namespace

#endif /* CXX_FABRIC_RTFABRICNODE_H_ */
