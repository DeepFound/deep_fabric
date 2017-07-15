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
#ifndef COM_DEEPIS_COMMUNICATION_COMMON_FABRICENVELOPE_H_
#define COM_DEEPIS_COMMUNICATION_COMMON_FABRICENVELOPE_H_

#include "cxx/io/EncodeProtocol.h"

namespace com { namespace deepis { namespace communication { namespace common {

class FabricEnvelope : public cxx::io::encodeProtocol::writer {
public:
	FabricEnvelope(uinttype iBufferLen, uinttype iMsgId, uinttype iPeerId) :
		writer(iBufferLen, iMsgId),
		m_iPeerId(iPeerId),
		m_iCmd(0),
		m_pCmdValue(0),
		m_iMsgId(iMsgId) {

	}

	FabricEnvelope(uinttype iCmd, voidptr pCmdValue) :
		writer(0, 0),
		m_iPeerId(0),
		m_iCmd(iCmd),
		m_pCmdValue(pCmdValue),
		m_iMsgId(0) {

	}

	virtual ~FabricEnvelope() {
	
	}

	void setPeerId(uinttype peerId) {
		m_iPeerId = peerId;
	}

	uinttype getPeerId() const {
		return m_iPeerId;
	}

	uinttype getCmd() const {
		return m_iCmd;
	}

	voidptr getCmdValue() const {
		return m_pCmdValue;
	}

	uinttype getMsgId(void) {
		return m_iMsgId;
	}

	void setMsgId(uinttype iMsgId) {
		m_iMsgId = iMsgId;
	}

private:
	uinttype m_iPeerId;
	uinttype m_iCmd;
	voidptr  m_pCmdValue;
	uinttype m_iMsgId;
};

} } } } // namespace

#endif /* COM_DEEPIS_COMMUNICATION_COMMON_FABRICENVELOPE_H_ */
