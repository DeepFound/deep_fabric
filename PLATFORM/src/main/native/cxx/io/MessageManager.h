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
#ifndef CXX_LANG_IO_MESSAGEMANAGER_H_
#define CXX_LANG_IO_MESSAGEMANAGER_H_

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <memory>
#include <functional>

#include "cxx/io/EOFException.h"
#include "cxx/io/LinkManager.h"
#include "cxx/io/EncodeProtocol.h"
#include "cxx/lang/Object.h"

namespace cxx { namespace io {

class MessageManager;

/**
 * Exceptions to handle errors beyond the LinkManager API
 */
class MessageManagerException : public Exception {
public:
	MessageManagerException(void) {
	}

	MessageManagerException(const char* message)
		: Exception(message) {
	}
}; // MessageManagerException

/** Delegate Callback to learn and act on various events. */
class notifications : public cxx::lang::Object {
public:
	virtual void canFree(voidptr pData);

	virtual void socketAccept(voidptr            socket,
				  cxx::lang::String& cPeerAddress) {}

	virtual void socketConnected(voidptr            socket,
				     cxx::lang::String& cPeerAddress) {}

	virtual void socketDisconnected(voidptr            socket,
					cxx::lang::String& cPeerAddress) {}

	virtual void overloadWarningRecv(ulongtype iTimeLimit) {}

	virtual void overloadWarningSend(ulongtype iTimeLimit) {}

}; /** notifications */

class MessageCallback : public cxx::lang::Object {
private:
	/** Private Members */
	MessageManager*        m_pcMsgMgr;
	cxx::io::messageBlock* m_pcResp;
	uinttype               m_iReplyCount;
	ubytetype*             m_piLast;

	friend class MessageManager;

public:
	MessageCallback();
	virtual ~MessageCallback();

	virtual uinttype callback(cxx::io::encodeProtocol::reader* pcRxMsg,
				  voidptr                          sendId);

	virtual boolean keepReader();

	MessageManager* getManagerInstance();
};

class NativeSocketCallback : public cxx::lang::Object {
public:
	NativeSocketCallback() {}
	virtual ~NativeSocketCallback() {}
	virtual inttype callback(inttype iFd, shorttype iEvents) { return 0; }
};

class MessageManager : public cxx::lang::Object {
private:
	struct sendMessageBlock : public cxx::io::messageBlock {
		sendMessageBlock(MessageManager* pcParent);
		virtual ~sendMessageBlock();

		virtual void canFree(voidptr pData);

		MessageManager* m_pcParent;
	};

	friend class sendMessageBlock;

	static const uinttype MIDDLE = 0;
	static const uinttype FIRST  = 1;
	static const uinttype LAST   = 2;

	struct pendingMessageQEntry : public cxx::lang::Object {
		pendingMessageQEntry();
		virtual ~pendingMessageQEntry();

		uinttype m_iFlags;
		voidptr  m_pMsg;
		uinttype m_iMsgLen;
	};

	struct socketEvent : public cxx::io::socketCallback {
		socketEvent(sendMessageBlock* pcSendBlk,
			    MessageManager*   pcParent);

		virtual ~socketEvent();

		virtual void readEvent(voidptr                 pData,
				       ulongtype               iSize,
				       voidptr                 sockId,
				       cxx::io::messageBlock*& pcReq,
				       voidptr                 lmContext);

		virtual void accepted(cxx::lang::String cAddress);
		virtual void connected(cxx::lang::String cAddress);
		virtual void disconnected(cxx::lang::String cAddress);

		inttype flushQ();

		voidptr           m_pcSocket;
		sendMessageBlock* m_pcSendBlk;
		MessageManager*   m_pcParent;

		uinttype          m_iBundle;
		boolean           m_bREQInProgress;
		cxx::util::ArrayList<pendingMessageQEntry* > m_cQueue;
		pendingMessageQEntry* m_pcCurrentIncomplete;
	};

	friend class socketEvent;

	struct nativeSocketEvent : public cxx::io::socketCallback {
		nativeSocketEvent(MessageManager* pcParent);
		virtual ~nativeSocketEvent();

		virtual void nativeFdEvent(inttype iFd, shorttype iEvents);

	private:
		virtual void readEvent(voidptr        pData,
				       ulongtype      iSize,
				       voidptr        sockId,
				       messageBlock*& pcResp,
				       voidptr        lmContext) {
		}

		MessageManager*   m_pcParent;
	};

	friend class nativeSocketEvent;

	struct mmlmNotifications : public lmNotifications {
		mmlmNotifications(MessageManager*   pcParent);
		virtual ~mmlmNotifications();

		virtual void overloadWarningRecv(ulongtype iTimeLimit);
		virtual void overloadWarningSend(ulongtype iTimeLimit);

		MessageManager*   m_pcParent;
	};

	friend class mmlmNotifications;

	/** Private methods. */
	void readEvent(voidptr      pData,
		       ulongtype    iSize,
		       voidptr      sockId,
		       socketEvent* pcEventBlk,
		       voidptr      lmContext);

	inttype processBundle(cxx::io::encodeProtocol::reader* pcRxMsg);

	inttype nativeFdEvent(inttype iFd, shorttype iEvents);

	/** Private Notification methods. */
	void canFree(voidptr pData);
	void accepted(voidptr        socket,
		      cxx::lang::String& cPeerAddress);
	void connected(voidptr        socket,
		       cxx::lang::String& cPeerAddress);
	void disconnected(voidptr        socket,
			  cxx::lang::String& cPeerAddress);

	ulongtype unblockREQ(socketEvent* pcEventBlk);

	void overloadWarningRecv(ulongtype iTimeLimit);
	void overloadWarningSend(ulongtype iTimeLimit);

	cxx::io::LinkManager*                               m_pcLinkManager;
	cxx::util::HashMap<uinttype,  MessageCallback*>*    m_pcCallbacks;
	cxx::util::HashMap<voidptr,   sendMessageBlock*>*   m_pcSendMsgBlocks;
	cxx::util::HashMap<voidptr,   socketEvent*>*        m_pcSocketEvents;
	notifications*                                      m_pcCb;
	MessageCallback*                                    m_pcCurrentCb;
	mmlmNotifications*                                  m_pcNotifCb;

	nativeSocketEvent*                                  m_pcNativeCb;
	cxx::util::HashMap<inttype, NativeSocketCallback*>* m_pcNativeCbs;

public:

	MessageManager();
	virtual ~MessageManager();

	ulongtype registerMessageId(uinttype                  iMsgId,
				    cxx::io::MessageCallback* pcCb);

	ulongtype unRegisterMessageId(uinttype iMsgId);

	ulongtype addRepSocket(const cxx::lang::String cServer,
			       voidptr&                sendId);

	ulongtype addReqSocket(const cxx::lang::String cOutgoing,
			       voidptr&                sendId);

	ulongtype sendREQ(voidptr                          sendId,
			  cxx::io::encodeProtocol::writer& cRequest,
			  boolean                          bLast=true);

	ulongtype sendREP(cxx::io::encodeProtocol::reader& cRxMsg,
			  cxx::io::encodeProtocol::writer& cReply);

	ulongtype sendREP(cxx::io::encodeProtocol::writer& cReply,
			  boolean                          bBlockPending = false);

	ulongtype unblockREQ(voidptr sockId);

	ulongtype close(voidptr& sendId);

	ulongtype poll(ulongtype iTimeout);

	ulongtype registerNotifications(notifications* pcCb) { m_pcCb = pcCb; return 0; }
	ulongtype unregisterNotifications(notifications* pcCb) { m_pcCb = 0; return 0; }

	ulongtype registerNativeCallback(inttype iFd,
					 NativeSocketCallback* pcCb);
	ulongtype unRegisterNativeCallback(inttype iFd);

	ulongtype stopService(void);

}; /** MessageManager */


} /** io */

} /** cxx */

/** pendingMessageQEntry */
FORCE_INLINE cxx::io::MessageManager::
pendingMessageQEntry::pendingMessageQEntry() : m_iFlags(0),
					       m_pMsg(0),
					       m_iMsgLen(0) {
}

FORCE_INLINE cxx::io::MessageManager::
pendingMessageQEntry::~pendingMessageQEntry() {
	m_iFlags  = 0;
	m_pMsg    = 0;
	m_iMsgLen = 0;
}

/** MessageCallback */

/**
 * keepReader
 * By default this is false. The MessageManager Framework keeps
 * ownership of the reader. This is for efficiency reasons. Overload
 * to return true, and the callback will have a reader that it will
 * own. This includes a copy of the message buffer. The callback is
 * the responsible for deleting unneeded readers.
 */
FORCE_INLINE boolean cxx::io::MessageCallback:: keepReader() {
	return false;
}

/** mmlmNotifications */

FORCE_INLINE cxx::io::MessageManager::
mmlmNotifications::mmlmNotifications(MessageManager* pcParent) : m_pcParent(pcParent) {
}

FORCE_INLINE cxx::io::MessageManager::
mmlmNotifications::~mmlmNotifications() {
	m_pcParent = 0;
}

FORCE_INLINE void cxx::io::MessageManager::
mmlmNotifications::overloadWarningRecv(ulongtype iTimeLimit) {
	m_pcParent->overloadWarningRecv(iTimeLimit);
}

FORCE_INLINE void cxx::io::MessageManager::
mmlmNotifications::overloadWarningSend(ulongtype iTimeLimit) {
	m_pcParent->overloadWarningSend(iTimeLimit);
}

#endif /** CXX_LANG_IO_MESSAGEMANAGER_H_ */

