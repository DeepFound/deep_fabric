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
#include <unistd.h>

#include "MessageManager.h"
#include "cxx/util/Logger.h"
#include "cxx/util/HashMap.cxx"
#include "LMReader.h"

using namespace cxx::lang;
using namespace cxx::util;

/** MessageCallback */

/**
 * MessageCallback
 * Constructor
 */
cxx::io::MessageCallback::MessageCallback()
	: m_pcMsgMgr(0),
	  m_pcResp(0),
	  m_iReplyCount(0),
	  m_piLast(0) {
}

/**
 * ~MessageCallback
 * Destructor
 */
cxx::io::MessageCallback::~MessageCallback() {
	m_pcMsgMgr    = 0;
	m_pcResp      = 0;
	m_iReplyCount = 0;
	m_piLast      = 0;
}

/**
 * MessageCallback::callback
 * This must be overloaded to handle the processing of a message
 * encoded with DEEP.
 * @param pcRxMsg [in] - a decoded message ready to be accessed. F/W
 * owns the reference.
 * @param sendId [in] - a send handle to use to respond back to the
 * sender.
 * @return just return 0. Has no special meaning just yet.
 */
uinttype cxx::io::MessageCallback::callback(cxx::io::encodeProtocol::reader* pcRxMsg,
					    voidptr                          sendId) {
	DEEP_LOG_ERROR(OTHER, "Not overloaded: cxx::io::MessageCallback::callback called.\n");
	return 0;
}

/**
 * MessageCallback::getManagerInstance
 * Returns a reference the the MessageManager that this
 * callback is registered with.
 * @return reference to a Message Manager instance.
 */
MessageManager* cxx::io::MessageCallback::getManagerInstance() {
	return m_pcMsgMgr;
}

/** MessageManager::sendMessageBlock */

/**
 * sendMessageBlock
 * Constructor
 * @param pcParent - the MessageManager parent instance.
 */
cxx::io::MessageManager::sendMessageBlock::sendMessageBlock(MessageManager* pcParent)
	: m_pcParent(pcParent) {
}

/**
 * ~sendMessageBlock
 * Destructor
 */
cxx::io::MessageManager::sendMessageBlock::~sendMessageBlock() {
	m_pcParent = 0;
}

/**
 * sendMessageBlock::canFree
 * Calls the MessageManager canFree handler.
 * @param pData - buffer reference that the service no longer needs.
 */
void cxx::io::MessageManager::sendMessageBlock::canFree(voidptr pData) {
	if (0 == m_pcParent) {
		return;
	}

	m_pcParent->canFree(pData);
}

/** MessageManager::socketEvent */

/**
 * socketEvent
 * Constructor.
 * @param pcSendBlk - reference to the sockets sendBlock
 * @param pcParent - reference the parent instance.
 */
cxx::io::MessageManager::socketEvent::socketEvent(sendMessageBlock* pcSendBlk,
						  MessageManager*   pcParent)
	: m_pcSocket(0),
	  m_pcSendBlk(pcSendBlk),
	  m_pcParent(pcParent),
	  m_iBundle(FIRST),
	  m_bREQInProgress(false),
	  m_pcCurrentIncomplete(0){
}

/**
 * ~socketEvent
 * Destructor
 */
cxx::io::MessageManager::socketEvent::~socketEvent() {
	flushQ();

	m_pcSocket       = 0;
	m_pcSendBlk      = 0;
	m_pcParent       = 0;
	m_iBundle        = 0;
	m_bREQInProgress = false;
	m_pcCurrentIncomplete = 0;
}

/**
 * flushQ
 * Flush the pending REQ queue for this connection. Indirectly calls
 * the parent objects canFree method.
 * @return 0 on success.
 */
inttype cxx::io::MessageManager::socketEvent::flushQ() {
	cxx::util::Iterator<pendingMessageQEntry* >* pcIt = m_cQueue.iterator();

	while (true == pcIt->hasNext()) {
		pendingMessageQEntry* pcQe = pcIt->next();
		pcIt->remove();

		if (0 == pcQe) {
			continue;
		}

		if (0 == m_pcParent) {
			abort();
		}

		m_pcParent->canFree(pcQe->m_pMsg);

		delete pcQe; pcQe = 0;
	}

	delete pcIt; pcIt = 0;

	m_pcCurrentIncomplete = 0;

	return 0;
}

/**
 * socketEvent::readEvent
 * Handles a read event for this socket. We end up
 * calling the parents handler.
 * @param pData - message buffer to process
 * @param iSize - size of the message in bytes
 * @param sockId - the socket this arrived on
 * @param pcReq [out] - return a message block in needed.
 */
void cxx::io::MessageManager::socketEvent::readEvent(voidptr                 pData,
						     ulongtype               iSize,
						     voidptr                 sockId,
						     cxx::io::messageBlock*& pcReq,
						     voidptr                 lmContext) {
	if (0 == m_pcParent) {
		return;
	}

	m_pcParent->readEvent(pData,
			      iSize,
			      sockId,
			      this,
			      lmContext);

	pcReq = this->m_pcSendBlk;
}

/**
 * socketEvent::accepted
 * Processes the REP socket accepted connection event.
 * @param cAddress - the peer IP Address.
 */
void cxx::io::MessageManager::socketEvent::accepted(cxx::lang::String cAddress) {
	if (0 == m_pcParent) {
		return;
	}

	m_pcParent->accepted(m_pcSocket, cAddress);
}

/**
 * socketEvent::connected
 * Processes the REQ socket remote peer connected event.
 * @param cAddress - the peer IP Address.
 */
void cxx::io::MessageManager::socketEvent::connected(cxx::lang::String cAddress) {
	if (0 == m_pcParent) {
		return;
	}

	m_pcParent->connected(m_pcSocket, cAddress);
}

/**
 * socketEvent::disconnected
 * Processes the REQ socket remote peer disconnected event.
 * @param cAddress - the peer IP Address.
 */
void cxx::io::MessageManager::socketEvent::disconnected(cxx::lang::String cAddress) {
	if (0 == m_pcParent) {
		return;
	}

	m_pcParent->disconnected(m_pcSocket, cAddress);
}

/** MessageManager::nativeSocketEvent */

/**
 * nativeSocketEvent
 * Constructor.
 * @param pcParent - reference the parent instance.
 */
cxx::io::MessageManager::nativeSocketEvent::nativeSocketEvent(MessageManager* pcParent)
	: m_pcParent(pcParent) {
}

/**
 * ~nativeSocketEvent
 * Destructor
 */
cxx::io::MessageManager::nativeSocketEvent::~nativeSocketEvent() {
	m_pcParent = 0;
}

/**
 * nativeFdEvent
 */
void cxx::io::MessageManager::nativeSocketEvent::nativeFdEvent(inttype iFd, shorttype iEvents) {
	m_pcParent->nativeFdEvent(iFd, iEvents);
}

/** notifications */

/**
 * notifications::canFree
 * Default behavior is to delete the now unused buffer.
 * @param pData - the buffer to delete.
 */
void cxx::io::notifications::canFree(voidptr pData) {
	ubytetype* piData = reinterpret_cast<ubytetype*>(pData);
	delete [] piData;
}

/** MessageManager */

/**
 * MessageManager
 * Constructor. Nothing much to do.
 */
cxx::io::MessageManager::MessageManager() :
	m_pcLinkManager(0),
	m_pcCallbacks(0),
	m_pcSendMsgBlocks(0),
	m_pcSocketEvents(0),
	m_pcCb(0),
	m_pcCurrentCb(0),
	m_pcNotifCb(0),
	m_pcNativeCb(0),
	m_pcNativeCbs(0) {

	m_pcLinkManager = new cxx::io::LinkManager;
	if (0 == m_pcLinkManager) {
		throw MessageManagerException("Unable to create the LinkManager.");
	}

	m_pcCallbacks = new cxx::util::HashMap<uinttype, MessageCallback*>(16, false, false, false);
	if (0 == m_pcCallbacks) {
		throw MessageManagerException("Unable to create the Callbacks Map.");
	}

	m_pcSendMsgBlocks = new cxx::util::HashMap<voidptr, sendMessageBlock*>(16, false, true, false);
	if (0 == m_pcSendMsgBlocks) {
		throw MessageManagerException("Unable to create the send blocks map.");
	}

	m_pcSocketEvents = new cxx::util::HashMap<voidptr, socketEvent*>(16, false, true, false);
	if (0 == m_pcSocketEvents) {
		throw MessageManagerException("Unable to create the socket events map.");
	}

	m_pcNotifCb = new mmlmNotifications(this);
	if (0 == m_pcNotifCb) {
		throw MessageManagerException("Unable to create mmlmNotifications.");
	}

	if (0 != m_pcLinkManager->registerNotifications(m_pcNotifCb)) {
		throw MessageManagerException("Unable to register mmlmNotifications.");
	}

	m_pcNativeCb = new nativeSocketEvent(this);
	if (0 == m_pcNativeCb) {
		throw MessageManagerException("Unable to create a general purpose native socket callback.");
	}

	m_pcNativeCbs = new cxx::util::HashMap<inttype, NativeSocketCallback*>(16, false, false, false);
	if (0 == m_pcNativeCbs) {
		throw MessageManagerException("Unable to create a native socket map.");
	}
}

/**
 * ~MessageManager
 * Destructor, clean up all the callbacks, event and send blocks.
 */
cxx::io::MessageManager::~MessageManager() {
	stopService();
}

/**
 * stopService
 * This permanently stops the MessageManager service. Lots of cleanup
 * and subsystem shutdown.
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::stopService(void) {
	if (0 != m_pcCallbacks) {
		delete m_pcCallbacks; m_pcCallbacks = 0;
	}

	if (0 != m_pcSendMsgBlocks) {
		delete m_pcSendMsgBlocks; m_pcSendMsgBlocks= 0;
	}

	if (0 != m_pcSocketEvents) {
		delete m_pcSocketEvents; m_pcSocketEvents = 0;
	}

	m_pcCb        = 0;
	m_pcCurrentCb = 0;

	if (0 != m_pcLinkManager) {
		m_pcLinkManager->stopService();
		delete m_pcLinkManager; m_pcLinkManager = 0;
	}

	if (0 != m_pcNotifCb) {
		delete m_pcNotifCb; m_pcNotifCb = 0;
	}

	if (0 != m_pcNativeCb) {
		delete m_pcNativeCb; m_pcNativeCb = 0;
	}

	if (0 != m_pcNativeCbs) {
		delete m_pcNativeCbs; m_pcNativeCbs = 0;
	}

	return 0;
}

/**
 * MessageManager::registerMessageId
 * Register a MessageCallback with a specific message id.
 * @param iMsgId - the unique message ID
 * @param pcCb - The callback instance to associate.
 * @return 0 if successfull.
 */
ulongtype cxx::io::MessageManager::registerMessageId(uinttype                  iMsgId,
						     cxx::io::MessageCallback* pcCb) {
	if (0 != m_pcCallbacks->get(iMsgId)) {
		return 1;
	}

	m_pcCallbacks->put(iMsgId, pcCb);

	pcCb->m_pcMsgMgr = this;
	
	return 0;
}

/**
 * MessageManager::unRegisterMessageId
 * Unregisters the given message Id and an associated callback object.
 * The caller owns the callback instance and is assumed to handle
 * its possible destruction.
 * @param iMsgId - the message ID.
 * @return 0 if successful.
 */
ulongtype cxx::io::MessageManager::unRegisterMessageId(uinttype iMsgId) {
	m_pcCallbacks->remove(iMsgId);

	return 0;
}

/**
 * MessageManager::addRepSocket
 * Create a send handle for a local server (REsPonder) address.
 * @param cServer [in] - a address string like: "tcp://1.1.1.1:8081"
 * @param sendId [out] - returns a referemce to a send handle.
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::addRepSocket(const cxx::lang::String cServer,
						voidptr&                 sendId) {

	voidptr pcSocket = 0;

	sendMessageBlock* pcSendBlk = new sendMessageBlock(this);
	if (0 == pcSendBlk) {
		DEEP_LOG_ERROR(OTHER, "Unable to create a send block.\n");
		return 1;
	}

	socketEvent* pcEventBlk = new socketEvent(pcSendBlk, this);
	if (0 == pcEventBlk) {
		delete pcSendBlk; pcSendBlk = 0;

		DEEP_LOG_ERROR(OTHER, "Unable to create an event block.\n");
		return 2;
	}

	ulongtype status = m_pcLinkManager->addRepSocket(cServer,
						      pcEventBlk,
						      pcSocket);
	if ((0 != status) || (0 == pcSocket)) {
		delete pcSendBlk;  pcSendBlk  = 0;
		delete pcEventBlk; pcEventBlk = 0;

		return 3;
	}

	pcEventBlk->m_pcSocket = pcSocket;

	m_pcSocketEvents->put(pcSocket, pcEventBlk);
	m_pcSendMsgBlocks->put(pcSocket, pcSendBlk);

	sendId = reinterpret_cast<voidptr>(pcEventBlk);

	return 0;
}

/**
 * MessageManager::addReqSocket
 * Create a send handle to a server (REQuester) address.
 * @param cOutgoing - the address of the server to connect to.
 * @param sendId [out] - returns a referemce to s send handle
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::addReqSocket(const cxx::lang::String cOutgoing,
						voidptr&                sendId) {
	voidptr pcSocket = 0;

	sendMessageBlock* pcSendBlk = new sendMessageBlock(this);
	if (0 == pcSendBlk) {
		DEEP_LOG_ERROR(OTHER, "Unable to create a send block.\n");
		return 1;
	}

	socketEvent* pcEventBlk = new socketEvent(pcSendBlk, this);
	if (0 == pcEventBlk) {
		delete pcSendBlk; pcSendBlk = 0;

		DEEP_LOG_ERROR(OTHER, "Unable to create an event block.\n");
		return 2;
	}

	ulongtype status = m_pcLinkManager->addReqSocket(cOutgoing,
						      pcEventBlk,
						      pcSocket);
	if ((0 != status) || (0 == pcSocket)) {
		delete pcSendBlk;  pcSendBlk  = 0;
		delete pcEventBlk; pcEventBlk = 0;

		return 3;
	}

	pcEventBlk->m_pcSocket = pcSocket;

	m_pcSocketEvents->put(pcSocket, pcEventBlk);
	m_pcSendMsgBlocks->put(pcSocket, pcSendBlk);

	sendId = reinterpret_cast<voidptr>(pcEventBlk);

	return 0;
}

/**
 * MessageManager::sendREQ
 * Send a REQuest encoded message to the sendId handle. Assumes that
 * if we bundle many sendREQ calls (bLast==False) then we will supply
 * a last REQ before the end of the end of the invoked callback or
 * thread of execution ends. Otherwise no messages will be sent for
 * the bundle.
 * @param sendId - the send handle.
 * @param cRequest - the request writer object with the encoded message.
 * @param bLast - if false there are more messages to come in this
 * batch and so this message not sent until we get the last message in
 * the batch. Whereby all messages in the batch are sent down to the
 * link manager for transmission.
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::sendREQ(voidptr                          sendId,
					   cxx::io::encodeProtocol::writer& cRequest,
					   boolean                          bLast) {
	if (0 == sendId) {
		return 1;
	}

	socketEvent* pcEventBlk = reinterpret_cast<socketEvent*>(sendId);
	if (0 == pcEventBlk) {
		return 2;
	}

	if (0 == pcEventBlk->m_pcSendBlk) {
		abort();
	}

	uinttype iFlags = MIDDLE;

	if ((FIRST == pcEventBlk->m_iBundle) && (true == bLast)) {
		cRequest.setFlags(cxx::io::encodeProtocol::header::ONE_MSG);
		iFlags = FIRST | LAST;
	} else if (FIRST == pcEventBlk->m_iBundle) {
		cRequest.setFlags(cxx::io::encodeProtocol::header::FIRST_FLAG);
		iFlags = FIRST;
	} else if (true == bLast) {
		cRequest.setFlags(cxx::io::encodeProtocol::header::LAST_FLAG);
		iFlags = LAST;
	}

	ubytetype* piBuffer    = 0;
	uinttype   iBufferSize = 0;

	cRequest.getMutableBuffer(piBuffer, iBufferSize);

	if (false == pcEventBlk->m_bREQInProgress) {
		/** Attach this message into the sending block */
		if (false == pcEventBlk->m_cQueue.isEmpty()) {
			DEEP_LOG_ERROR(OTHER, "Pending Queue is not empty and no REQ in progress!\n");
			throw MessageManagerException("Pending Queue is not empty and no REQ in progress!");
		}

		pcEventBlk->m_pcSendBlk->addMessage(piBuffer, iBufferSize);

		if (true == bLast) {
			ulongtype status = m_pcLinkManager->send(pcEventBlk->m_pcSocket,
								 pcEventBlk->m_pcSendBlk);

			pcEventBlk->m_bREQInProgress = true;
			pcEventBlk->m_iBundle        = FIRST;

			if (0 != status) {
				status |= 0x00010000;
			}

			return status;
		}
	} else {
		pendingMessageQEntry* pcQe = new pendingMessageQEntry;
		if (0 == pcQe) {
			throw MessageManagerException("Unable to allocaye a pending QEntry.");
		}

		if (FIRST == (pcQe->m_iFlags & FIRST)) {
			pcEventBlk->m_pcCurrentIncomplete = pcQe;
		}

		pcQe->m_pMsg    = piBuffer;
		pcQe->m_iMsgLen = iBufferSize;
		pcQe->m_iFlags  = iFlags;

		pcEventBlk->m_cQueue.add(pcQe);

		if (true == bLast) {
			pcEventBlk->m_pcCurrentIncomplete = 0;
			pcEventBlk->m_iBundle = FIRST;
			return 0;
		}
	}

	if (FIRST == pcEventBlk->m_iBundle) {
		pcEventBlk->m_iBundle = MIDDLE;
	}

	return 0;
}

/**
 * sendREP
 * Send a REPly. Must be used in a REQuest callback.
 * @param [in] cRxMsg - reference to the original received message.
 * @param [in] cReply - reference to a writer object. Caller owns this
 * instance. postReply will pull the buffer and invalidate the object.
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::sendREP(cxx::io::encodeProtocol::reader& cRxMsg,
					   cxx::io::encodeProtocol::writer& cReply) {
	if (true == cRxMsg.isReply()) {
		DEEP_LOG_ERROR(OTHER, "Can not reply to a reply message.\n");
		return 1;
	}

	if (0 == m_pcCurrentCb) {
		DEEP_LOG_ERROR(OTHER, "Must reply in the context of a REQ message callback\n");
		return 2;
	}

	if (0 == m_pcCurrentCb->m_pcResp) {
		DEEP_LOG_ERROR(OTHER, "No valid reply blocks supplied.\n");
		return 3;
	}

	cReply.setIsReply();

	if (0 == m_pcCurrentCb->m_iReplyCount) {
		cReply.setFlags(cxx::io::encodeProtocol::header::FIRST_FLAG);
	}

	ubytetype* piBuffer    = 0;
	uinttype   iBufferSize = 0;

	cReply.getMutableBuffer(piBuffer, iBufferSize);

	m_pcCurrentCb->m_pcResp->addMessage(piBuffer, iBufferSize);

	m_pcCurrentCb->m_piLast = piBuffer;
	m_pcCurrentCb->m_iReplyCount++;

	return 0;
}

/**
 * sendREP
 * Send a REPly. Must be used in a REQuest callback. Less safe as
 * assumes call knows not to reply to a reply.
 * @param [in] cReply - reference to a writer object. Caller owns
 * this.
 * @param [in] bBlockPending - true and any pending next
 * instance. postReply will pull the buffer and invalidate the object.
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::sendREP(cxx::io::encodeProtocol::writer& cReply,
					   boolean                          bBlockPending) {
	if (0 == m_pcCurrentCb) {
		DEEP_LOG_ERROR(OTHER, "Must reply in the context of a REQ message callback\n");
		return 2;
	}

	if (0 == m_pcCurrentCb->m_pcResp) {
		DEEP_LOG_ERROR(OTHER, "No valid reply blocks supplied.\n");
		return 3;
	}

	if (true == bBlockPending) {
		cReply.setFlags(cxx::io::encodeProtocol::header::REPLY_FLAG |
				cxx::io::encodeProtocol::header::BLOCK_FLAG);
	} else {
		cReply.setFlags(cxx::io::encodeProtocol::header::REPLY_FLAG);
	}

	if (0 == m_pcCurrentCb->m_iReplyCount) {
		cReply.setFlags(cxx::io::encodeProtocol::header::FIRST_FLAG);
	}

	ubytetype* piBuffer    = 0;
	uinttype   iBufferSize = 0;

	cReply.getMutableBuffer(piBuffer, iBufferSize);

	m_pcCurrentCb->m_pcResp->addMessage(piBuffer, iBufferSize);

	m_pcCurrentCb->m_piLast = piBuffer;
	m_pcCurrentCb->m_iReplyCount++;

	return 0;
}

/**
 * unblockREQ
 * Allows the use of a deferred REPly context to the last message of a
 * received bundle. This is intended to send one message and is a
 * onehot deal.
 * @param sockId - [in] Use the socket ID to located the pending REQ
 * Queue and trigger any next pending sendREQ..
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::unblockREQ(voidptr sockId) {
	socketEvent* pcEventBlk = m_pcSocketEvents->get(sockId);
	if (0 == pcEventBlk) {
		DEEP_LOG_ERROR(OTHER, "REQuest Context is NULL.\n");
		return 1;
	}

	return unblockREQ(pcEventBlk);
}

/**
 * unblockREQ
 * Allows the use of a deferred REPly context to the last message of a
 * received bundle. This is intended to send one message and is a
 * onehot deal.
 * @param pcEventBlk - [in] Use the Event Block for the pending REQ
 * Queue, and trigger any next pending sendREQ.
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::unblockREQ(socketEvent* pcEventBlk) {
	if (0 == pcEventBlk) {
		DEEP_LOG_ERROR(OTHER, "socketEvent Context is NULL.\n");
		return 1;
	}

	if (0 == pcEventBlk->m_cQueue.size()) {
		/** Nothing is pending, nothing to do. */
		return 0;
	}

	pcEventBlk->m_bREQInProgress = true;

	/** We have pending REQuest messages. */
	uinttype iFlags = MIDDLE;

	do {
		pendingMessageQEntry* pcQe = 0;

		pcQe = pcEventBlk->m_cQueue.get(0);
		if (0 == pcQe) {
			abort();
		}

		if (pcEventBlk->m_pcCurrentIncomplete == pcQe) {
			DEEP_LOG_ERROR(OTHER, "We have an incomplete pending bundle.\n");
			abort();
		}

		pcEventBlk->m_cQueue.remove(0);

		voidptr  pcMsg   = pcQe->m_pMsg;
		uinttype iMsgLen = pcQe->m_iMsgLen;

		pcEventBlk->m_pcSendBlk->addMessage(pcMsg, iMsgLen);
		iFlags = pcQe->m_iFlags;

		delete pcQe; pcQe=0;

		if ((LAST != (LAST & iFlags)) &&
		    (0 == pcEventBlk->m_cQueue.size())) {
			DEEP_LOG_ERROR(OTHER, "No more items to send but no last flag found! %p\n",
				       pcEventBlk->m_pcCurrentIncomplete);

			cxx::io::encodeProtocol::reader cBug((ubytetype*)pcMsg, iMsgLen);
			cBug.decodeFields();

			uinttype iMsgId   = cBug.getMsgId();
			boolean  bIsFirst = cBug.isFirst();
			boolean  bIsLast  = cBug.isLast();
			boolean  bIsReply = cBug.isReply();

			DEEP_LOG_ERROR(OTHER, "MSG_ID: %X, First:%d, Last:%d, Reply:%d, ptr=%p, len=%u\n",
				       iMsgId,
				       bIsFirst,
				       bIsLast,
				       bIsReply,
				       pcMsg,
				       iMsgLen);
			abort();
		}

	} while (LAST != (LAST & iFlags));

	ulongtype status = m_pcLinkManager->send(pcEventBlk->m_pcSocket,
						 pcEventBlk->m_pcSendBlk);

	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Unable to send a pending bundle.\n");
		pcEventBlk->flushQ();
	}

	return 0;
}

/**
 * MessageManager::close
 * Close the connection for the given send handle. This closes out
 * the undelying socket. If this is a server, then all connections
 * will be destroyed.
 * @param sendId - the session to close. The handle is then
 * invalidated.
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::close(voidptr& sendId) {
	if (0 == sendId) {
		return 1;
	}

	socketEvent* pcEventBlk = reinterpret_cast<socketEvent*>(sendId);
	if (0 == pcEventBlk) {
		return 2;
	}

	voidptr socket = pcEventBlk->m_pcSocket;

	ulongtype status = m_pcLinkManager->close(pcEventBlk->m_pcSocket);
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "LinkManager error closing socket\n");
	}

	sendMessageBlock* pcCb = m_pcSendMsgBlocks->get(socket);
	if (0 != pcCb) {
		delete pcCb; pcCb = 0;
		m_pcSendMsgBlocks->remove(socket);
	} else {
		DEEP_LOG_ERROR(OTHER, "Socket Send Block not found!\n");
	}

	socketEvent* pcEvent = m_pcSocketEvents->get(socket);
	if (0 != pcEvent) {
		m_pcSocketEvents->remove(socket);
	} else {
		DEEP_LOG_ERROR(OTHER, "Socket Event Block not found!\n");
	}

	delete pcEventBlk; pcEventBlk = 0;

	sendId = 0;

	return 0;
}

/**
 * MessageManager::poll
 * Called to process the connections. This will block for
 * iTimeout milliseconds or unblock before if there are
 * events to service.
 * @prarm iTimeout - milliseconds to wait blocking for an event
 * @return 0 in success.
 */
ulongtype cxx::io::MessageManager::poll(ulongtype iTimeout) {
	return m_pcLinkManager->poll(iTimeout);
}

/**
 * MessageManager::canFree
 * Called when the LinkManager has finished with this buffer and
 * it is safe to delete or reuse this buffer.
 * @param pData - reference to the buffer.
 */
void cxx::io::MessageManager::canFree(voidptr pData) {
	if (0 == m_pcCb) {
		ubytetype* piData = reinterpret_cast<ubytetype*>(pData);
		delete [] piData;
	} else {
		m_pcCb->canFree(pData);
	}
}

/**
 * MessageManager::accepted
 * Invokes a notification callback with the REP socket handle
 * and the IP Address of the Peer for an accepted connection.
 * @param socket - the rep socket.
 * @param cPeerAddress - the peers IP Address tcp://x.x.x.x:y
 */
void cxx::io::MessageManager::accepted(voidptr            socket,
				       cxx::lang::String& cPeerAddress) {
	if (0 == m_pcCb) {
		return;
	}

	socketEvent* pcEvent = m_pcSocketEvents->get(socket);
	if (0 == pcEvent) {
		return;
	}

	voidptr msgSock = reinterpret_cast<voidptr>(pcEvent);

	m_pcCb->socketAccept(msgSock, cPeerAddress);
}

/**
 * MessageManager::connected
 * Invokes a notification callback with the REQ socket handle
 * and the IP Address of the Peer for a connection established.
 * @param socket - the rep socket.
 * @param cPeerAddress - the peers IP Address tcp://x.x.x.x:y
 */
void cxx::io::MessageManager::connected(voidptr            socket,
					cxx::lang::String& cPeerAddress) {
	if (0 == m_pcCb) {
		return;
	}

	socketEvent* pcEvent = m_pcSocketEvents->get(socket);
	if (0 == pcEvent) {
		return;
	}

	voidptr msgSock = reinterpret_cast<voidptr>(pcEvent);

	m_pcCb->socketConnected(msgSock, cPeerAddress);
}

/**
 * MessageManager::disconnected
 * Invokes a notification callback with the REQ socket handle
 * and the IP Address of the Peer for a disconnection.
 * @param socket - the rep socket.
 * @param cPeerAddress - the peers IP Address tcp://x.x.x.x:y
 */
void cxx::io::MessageManager::disconnected(voidptr            socket,
					   cxx::lang::String& cPeerAddress) {
	if (0 == m_pcCb) {
		return;
	}

	socketEvent* pcEvent = m_pcSocketEvents->get(socket);
	if (0 == pcEvent) {
		return;
	}

	/** If we have any pending messages in the Q, then purge them. */
	pcEvent->flushQ();

	voidptr msgSock = reinterpret_cast<voidptr>(pcEvent);

	/** Now inform the callback associated with this socket. */
	m_pcCb->socketDisconnected(msgSock, cPeerAddress);
}

/**
 * MessageManager::readEvent
 * Handles a read event for a connection. We process the
 * message (assume it is always a DEEP message. We extract
 * the message ID and use that to find a suitable message
 * callback to process the message futher.
 * @param pData - message buffer
 * @param iSize - byte size of the buffer.
 * @param sockId - the socket this message arrived upon.
 * @param pcEventBlk - The handler registered for this socket.
 */
void cxx::io::MessageManager::readEvent(voidptr      pData,
					ulongtype    iSize,
					voidptr      sockId,
					socketEvent* pcEventBlk,
					voidptr      lmContext) {
	ubytetype* piData = reinterpret_cast<ubytetype*>(pData);

	cxx::io::lmReader* pcRxMsg = new cxx::io::lmReader(piData, iSize, lmContext);
	inttype status = pcRxMsg->decodeFields();
	if (0 != status) {
		DEEP_LOG_ERROR(OTHER, "Failed to decode message (status=%d, size=%llu).\n", status, iSize);

		if (0 != pcRxMsg) {
			delete pcRxMsg; pcRxMsg = 0;
		}

		return;
	}

	/** We should have a message ID to dispatch upon. */
	const uinttype iMsgId = pcRxMsg->getMsgId();

	/** Dispatch the message to the callback message ID handler.*/
	m_pcCurrentCb = m_pcCallbacks->get(iMsgId);
	if (0 == m_pcCurrentCb) {
		DEEP_LOG_ERROR(OTHER, "No callback found for the received message ID: %x\n",
			       iMsgId);

		if (0 != pcRxMsg) {
			delete pcRxMsg; pcRxMsg = 0;
		}

		return;
	}

	if ((0 == pcEventBlk) || (0 == pcEventBlk->m_pcSendBlk)) {
		DEEP_LOG_ERROR(OTHER, "NULL block for this socket!\n");

		if (0 != pcRxMsg) {
			delete pcRxMsg; pcRxMsg = 0;
		}

		return;
	}

	m_pcCurrentCb->m_pcResp = pcEventBlk->m_pcSendBlk;

	boolean bIsReply = pcRxMsg->isReply();
	boolean bIsLast  = pcRxMsg->isLast();
	boolean bToBlock = pcRxMsg->toBlock();

	m_pcCurrentCb->callback(pcRxMsg, sockId);

	if (0 != m_pcCurrentCb->m_piLast) {
		cxx::io::encodeProtocol::writer::setFlags(m_pcCurrentCb->m_piLast,
							  cxx::io::encodeProtocol::header::LAST_FLAG);
	}

	m_pcCurrentCb->m_pcResp      = 0;
	m_pcCurrentCb->m_iReplyCount = 0;
	m_pcCurrentCb->m_piLast      = 0;

	/**
	 * Now process the pending Queue. If and only if this was a
	 * reply on this socket and the reply was the last in the bundle.
	 */
	if ((true == bIsReply) && (true == bIsLast)) {
		if (false == bToBlock) {
			if (0 != pcEventBlk->m_cQueue.size()) {
				unblockREQ(pcEventBlk);

			} else {
				pcEventBlk->m_bREQInProgress = false;
			}
		}
	}
}

/**
 * overloadWarningRecv
 * Called when the poll blocks for excessive time receiving messages.
 * @param iTimeLimit - the number of millis blocked receiving a
 * message.
 */
void cxx::io::MessageManager::overloadWarningRecv(ulongtype iTimeLimit) {
	m_pcCb->overloadWarningRecv(iTimeLimit);
}

/**
 * overloadWarningSend
 * Called when the send call blocks for excessive time.
 * @param iTimeLimit - the number of millis blocked.
 */
void cxx::io::MessageManager::overloadWarningSend(ulongtype iTimeLimit) {
	m_pcCb->overloadWarningSend(iTimeLimit);
}

/**
 * nativeFdEvent
 * Handle any Native FD events for the specified FD.
 * @param iFd - the native FD
 * @param iEvents - the events associated with this FD.
 * @return 0 on success.
 */
inttype cxx::io::MessageManager::nativeFdEvent(inttype iFd, shorttype iEvents) {
	NativeSocketCallback* pcCb = m_pcNativeCbs->get(iFd);
	if (0 == pcCb) {
		throw MessageManagerException("No Native Callback handler was found!");
	}

	pcCb->callback(iFd, iEvents);

	return 0;
}

/**
 * registerNativeCallback
 * Register a callback to track events on a native FD.
 * @param iFd - native FD.
 * @param pcCb - FD event callback.
 */
ulongtype cxx::io::MessageManager::registerNativeCallback(inttype               iFd,
							  NativeSocketCallback* pcCb) {
	if (0 != m_pcNativeCbs->get(iFd)) {
		return 1;
	}

	if (0 != m_pcLinkManager->addNativeSocket(iFd, m_pcNativeCb)) {
		return 2;
	}

	m_pcNativeCbs->put(iFd, pcCb);

	return 0;
}

/**
 * unRegisterNativeCallback
 * Unregister a native FD and cease getting events about it.
 * @param iFd - The Native FD.
 * @return 0 on success.
 */
ulongtype cxx::io::MessageManager::unRegisterNativeCallback(inttype iFd) {
	if (0 == m_pcNativeCbs->get(iFd)) {
		/** Nothing to do. */
		return 0;
	}

	m_pcNativeCbs->remove(iFd);

	if (0 != m_pcLinkManager->removeNativeSocket(iFd)) {
		return 1;
	}

	return 0;
}
