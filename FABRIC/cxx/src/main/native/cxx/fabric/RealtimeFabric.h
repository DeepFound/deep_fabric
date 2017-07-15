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
#ifndef CXX_LANG_REALTIMEFABRIC_H_
#define CXX_LANG_REALTIMEFABRIC_H_

#include <stdio.h>
#include <errno.h>

#include "cxx/io/EOFException.h"
#include "cxx/io/MessageManager.h"
#include "cxx/lang/types.h"
#include "cxx/util/HashMap.h"
#include "cxx/util/HashMap.cxx"
#include "cxx/util/ArrayList.h"
#include "cxx/util/UUID.h"

#include "com/deepis/db/store/relative/distributed/IMessageService.h"
#include "com/deepis/communication/common/MsgIds.h"

#include "RealtimeFabricCommon.h"
#include "RealtimeFabricMatrix.h"

#include "com/deepis/communication/fabricconnector/FabricCassiMessageQueue.h"

#include <json_parser.h>

using namespace com::deepis::communication::fabricconnector;

namespace cxx { namespace fabric {

class RealtimeFabricException : public Exception {
public:
	RealtimeFabricException(void) {
	}

	RealtimeFabricException(const char* message)
		: Exception(message) {
	}
}; /** RealtimeFabricException */

class RealtimeFabric : public cxx::lang::Object {
public:
	RealtimeFabric(const uinttype          iServerId,
		       const cxx::lang::String cRepAddress,
		       void*                   pcLoaderList = 0,
		       ulongtype               iMemoryCap   = 1024*1024*8);

	FabricCassiMessageQueue* getFabricCassiMessageQueue();

	virtual ~RealtimeFabric();

	ulongtype addPeer(const cxx::lang::String cAddress,
			  cxx::util::UUID&        cUUID);
	
	ulongtype delPeer(const cxx::lang::String cAddress);
	ulongtype delPeer(cxx::util::UUID&        cUUID);

	ulongtype poll(ulongtype iTimeout);

	ulongtype stopService();

private:

	struct JOIN_REQ {
		static const uinttype MSG_ID = MSG_ID(com::deepis::communication::common::msgPrefix::REALTIME_FABRIC_INTERNAL, 1);

		/** Fields */
		static const uinttype SERVER_ID_FIELD = 1; /** UINT32 */
		static const uinttype SERVER_FIELD    = 2; /** STRING */

	}; /** PEER_HELLO_REQ */

	struct JOIN_REQ_CB : public cxx::io::MessageCallback {
		JOIN_REQ_CB(RealtimeFabric* pcParent);

		virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
					  voidptr                          sendId);

		RealtimeFabric* m_pcParent;

	}; /** JOIN_REQ_CB */

	friend class JOIN_REQ_CB;

	struct JOIN_REP {
		static const uinttype MSG_ID = MSG_ID(com::deepis::communication::common::msgPrefix::REALTIME_FABRIC_INTERNAL, 2);

		/** Fields */
		static const uinttype SERVER_ID_FIELD = 1; /** UINT32 */
		static const uinttype SERVER_FIELD    = 2; /** STRING */

	}; /** JOIN_REP */

	struct JOIN_REP_CB : public cxx::io::MessageCallback {
		JOIN_REP_CB(RealtimeFabric* pcParent);

		virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
					  voidptr                          sendId);

		RealtimeFabric* m_pcParent;

	}; /** JOIN_REP_CB */

	friend class JOIN_REP_CB;

	struct TOPO_REQ {
		static const uinttype MSG_ID = MSG_ID(com::deepis::communication::common::msgPrefix::REALTIME_FABRIC_INTERNAL, 3);

		/** Fields */
		static const uinttype SERVER_ID_FIELD = 1; /** UINT32 */
		static const uinttype MATRIX_FIELD    = 2; /** STRING */

	}; /** TOPO_REQ */

	struct TOPO_REQ_CB : public cxx::io::MessageCallback {
		TOPO_REQ_CB(RealtimeFabric* pcParent);

		virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
					  voidptr                          sendId);

		RealtimeFabric* m_pcParent;

	}; /** TOPO_REQ_CB */

	friend class TOPO_REQ_CB;

	struct TOPO_REP {
		static const uinttype MSG_ID = MSG_ID(com::deepis::communication::common::msgPrefix::REALTIME_FABRIC_INTERNAL, 4);

		/** Fields */
		static const uinttype SERVER_ID_FIELD = 1; /** UINT32 */
		static const uinttype MATRIX_FIELD    = 2; /** STRING */

	}; /** TOPO_REQ */

	struct TOPO_REP_CB : public cxx::io::MessageCallback {
		TOPO_REP_CB(RealtimeFabric* pcParent);

		virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
					  voidptr                          sendId);

		RealtimeFabric* m_pcParent;

	}; /** TOPO_REP_CB */

	friend class TOPO_REP_CB;

	struct SHIM_REQ {
		static const uinttype MSG_ID = MSG_ID(com::deepis::communication::common::msgPrefix::REALTIME_FABRIC_INTERNAL, 5);
	}; /** SHIM_REQ */

	struct SHIM_REQ_CB : public cxx::io::MessageCallback {
		SHIM_REQ_CB(RealtimeFabric* pcParent);

		virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
					  voidptr                          sendId);

		RealtimeFabric* m_pcParent;

	}; /** SHIM_REQ_CB */

	struct SHIM_REP_CB : public cxx::io::MessageCallback {
		SHIM_REP_CB(RealtimeFabric* pcParent);

		virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
					  voidptr                          sendId);

		RealtimeFabric* m_pcParent;

	}; /** SHIM_REP_CB */

	friend class SHIM_REP_CB;

	struct PACK_BUNDLE_REQ {
		static const uinttype MSG_ID = MSG_ID(com::deepis::communication::common::msgPrefix::REALTIME_FABRIC_INTERNAL, 6);
	}; /** PACK_BUNDLE_REQ */

	struct PACK_BUNDLE_CB : public cxx::io::MessageCallback {
		PACK_BUNDLE_CB(RealtimeFabric* pcParent);

		virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
					  voidptr                          sendId);

		RealtimeFabric* m_pcParent;

	}; /** PACK_BUNDLE_CB */

	struct inBound : public cxx::lang::Object {
		inBound();
		~inBound();

		cxx::lang::String m_cServerAddr;
		uinttype          m_iServerId;

	}; /** inBound */

	struct outBound : public cxx::lang::Object {
		outBound();
		~outBound();

		void purge();

		cxx::lang::String   m_cServerAddr;
		uinttype            m_iServerId;
		voidptr             m_reqSocket;
		uinttype            m_iCurrentId;
		cxx::util::UUID*    m_pcUUID;

	}; /** outBound */

	/** MessageManager Notifications */
	struct msgMgrNotifications : public cxx::io::notifications {
		msgMgrNotifications(RealtimeFabric* pcParent);
		virtual ~msgMgrNotifications();

		virtual void socketAccept(voidptr            socket,
					  cxx::lang::String& cPeerAddress);

		virtual void socketConnected(voidptr            socket,
					     cxx::lang::String& cPeerAddress);

		virtual void socketDisconnected(voidptr            socket,
						cxx::lang::String& cPeerAddress);

		virtual void overloadWarningRecv(ulongtype iTimeLimit);

		virtual void overloadWarningSend(ulongtype iTimeLimit);

		RealtimeFabric* m_pcParent;
	};

	friend class msgMgrNotifications;

	struct cassiFabQCb : public cxx::io::NativeSocketCallback {
		cassiFabQCb(RealtimeFabric* pcParent);
		virtual ~cassiFabQCb();
		virtual inttype callback(inttype iFd, shorttype iEvents);

		RealtimeFabric* m_pcParent;
	};

	friend class cassiFabQCb;

	ulongtype encodeJoinReq(cxx::io::encodeProtocol::writer*& pcWriter);

	ulongtype processJoinReq(cxx::io::encodeProtocol::reader* pcRxMsg);
	ulongtype processJoinRep(cxx::io::encodeProtocol::reader* pcRxMsg);

	ulongtype rejoinReq(outBound*          pcOut,
			    cxx::lang::String& cAddress);

	ulongtype disconnect(cxx::lang::String& cAddress);

	ulongtype sendOutTopoReq();
	ulongtype sendOutTopoRep(cxx::io::messageBlock*           pcResp,
				 boolean                          bChanged);

	ulongtype processTopoReq(cxx::io::encodeProtocol::reader* pcRxMsg);
	ulongtype processTopoRep(cxx::io::encodeProtocol::reader* pcRxMsg);
	ulongtype processShimReq(cxx::io::encodeProtocol::reader* pcRxMsg);
	ulongtype processShimRep(cxx::io::encodeProtocol::reader* pcRxMsg);
	ulongtype processPackReq(cxx::io::encodeProtocol::reader* pcRxMsg);

	ulongtype addAdj(uinttype iPeerId);
	ulongtype delAdj(uinttype iPeerId);

	ulongtype consumeWorkItems();

	ulongtype processFabricCommands(FabricEnvelope* pcMsg);
	ulongtype processBundleCommand(FabricEnvelope* pcMsg);

	ulongtype sendMsg(outBound*                        pcOut,
			  cxx::io::encodeProtocol::writer* pcMsg,
			  boolean                          bLast);

	ulongtype sendLastRep(void);

	static const uinttype MIN_MESSAGE_SIZE      = 10;
	static const inttype  BUNDLE_ARRAY_INIT_CAP = 2048;

	/** Private Members */
	uinttype                                           m_iServerId;
	cxx::lang::String                                  m_cRepAddress;
	cxx::io::MessageManager*                           m_pcMsgMgr;
	voidptr                                            m_repSock;

	JOIN_REQ_CB                                        m_cJoinReqCb;
	JOIN_REP_CB                                        m_cJoinRepCb;
	TOPO_REQ_CB                                        m_cTopoReqCb;
	TOPO_REP_CB                                        m_cTopoRepCb;
	SHIM_REQ_CB                                        m_cShimReqCb;
	SHIM_REP_CB                                        m_cShimRepCb;
	PACK_BUNDLE_CB                                     m_cPackRepCb;

	cxx::util::HashMap<uinttype,           inBound*>*  m_pcInBoundPeers;
	cxx::util::HashMap<cxx::lang::String*, inBound*>*  m_pcInBoundPeersByName;

	cxx::util::HashMap<uinttype,           outBound*>* m_pcOutBoundPeers;
	cxx::util::HashMap<cxx::lang::String*, outBound*>* m_pcOutBoundPeersByName;
	cxx::util::HashMap<cxx::util::UUID*,   outBound*>* m_pcOutBoundPeersByUUID;

	msgMgrNotifications                                m_cMsgMgrNotifications;

	cxx::fabric::RealtimeFabricAdjMatrix               m_cTopology;

	/** CASSI Shim */
	com::deepis::communication::fabricconnector::
	FabricCassiMessageQueue*                           m_pcCassiShim;
	cxx::lang::Thread*                                 m_pcCassiThread;
	cassiFabQCb*                                       m_pcCassiFabQCb;

	/** Bundle Checks */
	uinttype                                           m_iBundleCount;
	ulongtype                                          m_iTxBundleTimeStart;
	ulongtype                                          m_iRxBundleTimeStart;

	/** Bundle to FCQ batching */
	cxx::util::ArrayList<cxx::io::encodeProtocol::reader*>* m_pcBatch;

}; /** RealtimeFabric */

} /** fabric */

} /** cxx */

/** inBound */
FORCE_INLINE cxx::fabric::RealtimeFabric::
inBound::inBound() : m_iServerId(0) {
}

FORCE_INLINE cxx::fabric::RealtimeFabric::
inBound::~inBound() {
	m_iServerId = 0;
}

/** outBound */
FORCE_INLINE cxx::fabric::RealtimeFabric::
outBound::outBound() : m_iServerId(0),
		       m_reqSocket(0),
		       m_iCurrentId(0),
		       m_pcUUID(0) {
}

FORCE_INLINE cxx::fabric::RealtimeFabric::
outBound::~outBound() {
	m_iServerId  = 0;
	m_reqSocket  = 0;
	m_iCurrentId = 0;
	m_pcUUID     = 0;
}

/** JOIN_REQ_CB */
FORCE_INLINE cxx::fabric::RealtimeFabric::
JOIN_REQ_CB::JOIN_REQ_CB(RealtimeFabric* pcParent) : m_pcParent(pcParent) {
}

/** JOIN_REP_CB */
FORCE_INLINE cxx::fabric::RealtimeFabric::
JOIN_REP_CB::JOIN_REP_CB(RealtimeFabric* pcParent) : m_pcParent(pcParent) {
}

/** TOPO_REQ_CB */
FORCE_INLINE cxx::fabric::RealtimeFabric::
TOPO_REQ_CB::TOPO_REQ_CB(RealtimeFabric* pcParent) : m_pcParent(pcParent) {
}

/** TOPO_REP_CB */
FORCE_INLINE cxx::fabric::RealtimeFabric::
TOPO_REP_CB::TOPO_REP_CB(RealtimeFabric* pcParent) : m_pcParent(pcParent) {
}

/** SHIM_REQ_CB */
FORCE_INLINE cxx::fabric::RealtimeFabric::
SHIM_REQ_CB::SHIM_REQ_CB(RealtimeFabric* pcParent) : m_pcParent(pcParent) {
}

/** SHIM_REP_CB */
FORCE_INLINE cxx::fabric::RealtimeFabric::
SHIM_REP_CB::SHIM_REP_CB(RealtimeFabric* pcParent) : m_pcParent(pcParent) {
}

/** PACK_BUNDLE_CB */
FORCE_INLINE cxx::fabric::RealtimeFabric::
PACK_BUNDLE_CB::PACK_BUNDLE_CB(RealtimeFabric* pcParent) : m_pcParent(pcParent) {
}

#endif /** CXX_LANG_REALTIMEFABRIC_H_ */
