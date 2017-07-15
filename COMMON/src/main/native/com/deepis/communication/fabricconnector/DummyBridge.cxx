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
#include "FabricCassiMessageQueue.h"
#include "cxx/util/Logger.h"
#include "cxx/lang/ThreadLocal.h"
#include "DummyBridge.h"

using namespace cxx::lang;
using namespace cxx::util;
using namespace com::deepis::communication::fabricconnector;
using namespace com::deepis::communication::common;

FabricCassiBridgeCb* getDummyBridgeInstance(FabricCassiMessageQueue* pcCommon, voidptr pCtx) {
	DEEP_LOG_DEBUG(OTHER, "Creating DummyBridge.\n");
	return new DummyBridge(pcCommon);
}

void DummyBridge::remoteOverloaded(cxx::io::encodeProtocol::reader* pcMsg) {
	DEEP_LOG_DEBUG(OTHER, "DummyBridge::remoteOverloaded\n");
}

void DummyBridge::shutdown() {
	DEEP_LOG_DEBUG(OTHER, "DummyBridge::shutdown\n");
}

void DummyBridge::peerStateChanged(cxx::io::encodeProtocol::reader* pcMsg) {
	DEEP_LOG_DEBUG(OTHER, "DummyBridge::peerStateChanged\n");
}

inttype DummyBridge::dispatch(cxx::io::encodeProtocol::reader* pcMsg) {
	DEEP_LOG_DEBUG(OTHER, "DummyBridge::dispatch MsgId=0x%x\n", pcMsg->getMsgId());
	return 0;
}
