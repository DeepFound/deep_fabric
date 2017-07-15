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
#ifndef CXX_REPLICATION_FILEREPLICATION_H_
#define CXX_REPLICATION_FILEREPLICATION_H_

#include <stdio.h>
#include <errno.h>

#include "cxx/lang/String.h"
#include "cxx/util/UUID.h"
#include "cxx/util/HashMap.h"

#include "cxx/io/EOFException.h"
#include "com/deepis/db/store/relative/distributed/IMessageService.h"
#include "com/deepis/communication/common/MsgIds.h"
#include "com/deepis/communication/fabricconnector/FabricCassiMessageQueue.h"

using namespace com::deepis::communication::common;

namespace cxx { namespace replication {

class FileReplicationBridge : public com::deepis::communication::fabricconnector::FabricCassiBridgeCb {
public:
	FileReplicationBridge(com::deepis::communication::fabricconnector::FabricCassiMessageQueue* pcCommon, voidptr pCtx);
	virtual ~FileReplicationBridge();

	virtual void remoteOverloaded(cxx::io::encodeProtocol::reader* pcMsg);
	virtual void shutdown();
	virtual void peerStateChanged(cxx::io::encodeProtocol::reader* pcMsg);
	virtual inttype dispatch(cxx::io::encodeProtocol::reader* pcMsg);
	virtual const uinttype* allMessageIds(void);

private:
	struct FILE_TX_REQ {
		static const uinttype MSG_ID             = MSG_ID(msgPrefix::FILE_REPLICATION, 1);
		static const uinttype SERVER_ID_FIELD    = 0; /** UINT32 */
		static const uinttype FILE_PATH_FIELD    = 1; /** STRING */
		static const uinttype FILE_SIZE_FIELD    = 2; /** UINT64 */
		static const uinttype FILE_SHA1SUM_FIELD = 3; /** STRING */
		static const uinttype FILE_UUIDLO_FIELD  = 4; /** INT64  */
		static const uinttype FILE_UUIDHI_FIELD  = 5; /** INT64  */
	};

	struct FILE_TX_REP {
		enum REPLY {
			OK_TO_SEND =  0,
			DUPLICATE  = -1,
			NO_SPACE   = -2,
			STOP_TX    = -3,
		};

		static const uinttype MSG_ID             = MSG_ID(msgPrefix::FILE_REPLICATION, 2);
		static const uinttype SERVER_ID_FIELD    = 0; /** UINT32 */
		static const uinttype FILE_UUIDLO_FIELD  = 1; /** INT64  */
		static const uinttype FILE_UUIDHI_FIELD  = 2; /** INT64  */
		static const uinttype FILE_REPLY_FIELD   = 3; /** INT32  */
	};

	struct FILE_BLOCK_REQ {
		static const uinttype MSG_ID             = MSG_ID(msgPrefix::FILE_REPLICATION, 3);
		static const uinttype SERVER_ID_FIELD    = 0; /** UINT32 */
		static const uinttype FILE_UUIDLO_FIELD  = 1; /** INT64  */
		static const uinttype FILE_UUIDHI_FIELD  = 2; /** INT64  */
		static const uinttype BLOCK_FIELD        = 3; /** RAW    */
		static const uinttype TIME_STAMP_FIELD   = 4; /** UINT64 */
	};

	struct FILE_BLOCK_REP {
		static const uinttype MSG_ID             = MSG_ID(msgPrefix::FILE_REPLICATION, 4);
		static const uinttype SERVER_ID_FIELD    = 0; /** UINT32 */
		static const uinttype FILE_UUIDLO_FIELD  = 1; /** INT64  */
		static const uinttype FILE_UUIDHI_FIELD  = 2; /** INT64  */
		static const uinttype FILE_REPLY_FIELD   = 3; /** INT32  */
	};

	//static const uinttype FILE_BUNDLE_SIZE = 16;
	//static const uinttype FILE_BLOCK_SIZE  = 4*1024*1024;
	static const uinttype FILE_BUNDLE_SIZE = 10000;
	static const uinttype FILE_BLOCK_SIZE  = 59;

	struct FileTxBlock {
		uinttype          m_iPeerId;
		cxx::lang::String m_cFileName;
		inttype           m_iFd;
		longtype          m_iBlockSize;
		longtype          m_iBlocksLeft;
		longtype          m_iBundleSize;
		long long         m_iStart;
		ulongtype         m_iBytesTx;
#if STAT_SIZES
		ubytetype         m_iBuffer[BUFSIZ];
#else
		ubytetype         m_iBuffer[FILE_BLOCK_SIZE];
#endif

		FileTxBlock() : m_iPeerId(0),
				m_iFd(-1),
				m_iBlockSize(0),
				m_iBlocksLeft(0),
				m_iBundleSize(0),
				m_iStart(0),
				m_iBytesTx(0) {
		}

		~FileTxBlock() {
			m_iPeerId = 0;
			if (0 != m_iFd) {
				close(m_iFd); m_iFd = -1;
			}
		}
	};

	struct FileRxBlock {
		uinttype          m_iPeerId;
		cxx::lang::String m_cFileName;
		inttype           m_iFd;
		long long         m_iStart;
		ulongtype         m_iBytesRx;

		FileRxBlock() : m_iPeerId(0),
				m_iFd(-1),
				m_iStart(0),
				m_iBytesRx(0) {
		}

		~FileRxBlock() {
			m_iPeerId = 0;
			if (0 != m_iFd) {
				close(m_iFd); m_iFd = -1;
			}
		}
	};

	inttype requestFileReplication(uinttype           iPeerId,
				       cxx::lang::String& cSrc,
				       cxx::lang::String& cDst);

	inttype processFileTxReq(cxx::io::encodeProtocol::reader* pcMsg);
	inttype processFileTxRep(cxx::io::encodeProtocol::reader* pcMsg);
	inttype processFileBlockReq(cxx::io::encodeProtocol::reader* pcMsg);
	inttype processFileBlockRep(cxx::io::encodeProtocol::reader* pcMsg);

	inttype renderFileBlockBundle(longtype iLo, longtype iHi, FileTxBlock* pcOut, boolean& bFinished);

	cxx::util::HashMap<cxx::util::UUID*, FileTxBlock* > m_cTxBlock;
	cxx::util::HashMap<cxx::util::UUID*, FileRxBlock* > m_cRxBlock;

	cxx::lang::String                                   m_cPaths[2];

}; // FileReplicationBridge

com::deepis::communication::fabricconnector::FabricCassiBridgeCb*
getFileReplicationBridgeInstance(com::deepis::communication::fabricconnector::FabricCassiMessageQueue* pcCommon,
				 voidptr pCtx);

} // replication
} // cxx

#endif // CXX_REPLICATION_FILEREPLICATION_H_
