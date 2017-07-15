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
#include "FabricCassiJSONAPI.h"
#include "cxx/util/Logger.h"
#include "cxx/util/HashMap.cxx"
#include <sstream>

using namespace cxx::lang;
using namespace cxx::util;
using namespace com::deepis::communication::fabricconnector;
using namespace com::deepis::communication::common;

/**
 * JsonApi
 * Constructor. Set up the domain socket to listen upon.
 * @param iServerId - The Server ID number for this Fabric.
 */
JsonApi::JsonApi(uinttype iServerId) :
	m_iServerId(iServerId),
	m_bStop(false),
	m_cCassiToApi(16, true),
	m_cApiToCassi(16, true),
	m_iApiEpollFd(-1) {

	std::stringstream ss;
	ss << "/tmp/RealtimeFabric." << iServerId;
	m_cSocketName = ss.str();

	createDomainSocket(m_cSocketName);

	memset(&m_stApiEvents, 0, sizeof(m_stApiEvents));

	if (0 != enablePoller()) {
		throw JsonApiException("Unable to enable the poller.");
	}

	if (0 != m_cApiToCassi.enablePoller()) {
		throw JsonApiException("Unable to enable the API to API Consumer poller.");
	}
}

/**
 * JsonApi
 * Constructor. Set up the domain socket to listen upon.
 * @param iServerId - The Server ID number for this Fabric.
 */
JsonApi::JsonApi(uinttype iServerId, ushorttype iPort) :
	m_iServerId(iServerId),
	m_bStop(false),
	m_cCassiToApi(16, true),
	m_cApiToCassi(16, true),
	m_iApiEpollFd(-1) {

	createTcpSocket(iPort);

	memset(&m_stApiEvents, 0, sizeof(m_stApiEvents));

	if (0 != enablePoller()) {
		throw JsonApiException("Unable to enable the poller.");
	}

	if (0 != m_cApiToCassi.enablePoller()) {
		throw JsonApiException("Unable to enable the API to API Consumer poller.");
	}
}

/**
 * ~JsonApi
 * Destructor
 */
JsonApi::~JsonApi() {
	m_bStop = false;
	unlink(m_cSocketName.c_str());

	// XXX: clean up the queues.
}

/**
 * run
 * The primary JSON API thread run loop.
 */
void JsonApi::run(void) {
	DEEP_LOG_DEBUG(OTHER, "JSON API Thread has started.\n");

	while (false == m_bStop) {
		this->poll(10);
	}

	DEEP_LOG_DEBUG(OTHER, "JSON API Thread has exited.\n");
}

/**
 * handler
 * Processes the received payload and attempts to parse
 * the JSON data contained within.
 * @param piReadBuffer - The JSON Buffer
 * @param iReadSize - The size of the buffer.
 * @return 0 if no issues.
 */
inttype JsonApi::handler(bytetype* piReadBuffer,
			 inttype   iReadSize) {

	cxx::lang::String cRead((const char *)piReadBuffer, (size_t)iReadSize);

	/**
	 * Example of a API Client Request:
	 * {"path": "/fabric/peer", "cmd": "POST",
	 * "data": {"attributes": {"address": "tcp://127.0.0.1:10000"},
	 * "type": "peer", "id": "ff5380c2-38a8-11e6-be00-000c29589848"}}
	 */
	inttype           iWriteSize = 0;

	JsonApiRequest*   pcReq = new JsonApiRequest;
	if (0 == pcReq) {
		throw JsonApiException("Unable to create JSON request block.");
	}

	pcReq->m_pcJson = new json::root(cRead);
	if (0 == pcReq->m_pcJson) {
		throw JsonApiException("Unable to create the JSON document.");
	}

	json::value       cCmd      = pcReq->m_pcJson->operator[]("cmd");
	cxx::lang::String cCmdStr;
	cCmd.str(cCmdStr);
	
	json::value       cPath     = pcReq->m_pcJson->operator[]("path");
	cPath.str(pcReq->m_cPath);

	if (true == pcReq->m_pcJson->exists("data")) {
		json::value cData = pcReq->m_pcJson->operator[]("data");
		pcReq->m_pcData   = cData.to_object();
	}

	if (0 != pcReq->m_pcData) {
		if (true == pcReq->m_pcData->exists("id")) {
			json::object&     cDataObj = *pcReq->m_pcData;
			json::value       cId      = cDataObj["id"];
			cId.str(pcReq->m_cIdStr);

			pcReq->m_pcUUID = cxx::util::UUID::fromString(pcReq->m_cIdStr);
			if (0 == pcReq->m_pcUUID) {
				DEEP_LOG_ERROR(OTHER, "Invalid UUID %s\n", pcReq->m_cIdStr.c_str());
				pcReq->m_eRetCode = HANDLER_BAD_REQUEST;
			}
		}
	}

	DEEP_LOG_DEBUG(OTHER, "Client API Request Received [%s : %s].\n",
		       cCmdStr.c_str(),
		       pcReq->m_cPath.c_str());

	pcReq->m_eRequest = requestCode(cCmdStr);
	if (UNKNOWN == pcReq->m_eRequest) {
		pcReq->m_eRetCode = HANDLER_BAD_REQUEST;
	} else {
		/**
		 * Now post the request to the FabricCassiMessageQ.
		 * There bridge code will have registered to process
		 * specific path/Request code tuples.
		 */
		inttype iStatus = enqueueREQ(pcReq);
		if (0 != iStatus) {
			pcReq->m_eRetCode = HANDLER_INTERNAL_ERROR;
			pcReq->m_cErrorTitle = "Unable to Queue up REQUEST to FabCassi.";
		} else {
			boolean bHasWork = false;
			while (false == bHasWork) {
				iStatus = pollREP(1000, bHasWork);
				if (true == bHasWork) {
					if (0 != dequeueREP(pcReq)) {
						pcReq->m_eRetCode = HANDLER_INTERNAL_ERROR;
						pcReq->m_cErrorTitle = "Unable to dequeue the REPLY from FabCassi.";
					}
				}
			}
		}
	}

	if (true == positiveReturn(pcReq->m_eRetCode)) {
		json_object cRetRoot;

		if (0 != pcReq->m_pcUUID) {
			pcReq->m_cPath += "/" + pcReq->m_cIdStr;
		}

		cRetRoot.add("code", pcReq->m_eRetCode);
		cRetRoot.add("title", returnStatus(pcReq->m_eRetCode).c_str());

		subbuffer cRetPath(pcReq->m_cPath);
		cRetRoot.add("path", cRetPath);

		if (0 != pcReq->m_pcRetData) {
			cRetRoot.add("data", *(pcReq->m_pcRetData));
		}

		cxx::lang::String cResp;
		cRetRoot.to_string<cxx::lang::String>(cResp);

		strcpy(piReadBuffer, cResp.c_str());
		iWriteSize = cResp.size()+1;

	} else {
		json_object cRetRoot;

		cRetRoot.add("code", pcReq->m_eRetCode);
		cRetRoot.add("status", returnStatus(pcReq->m_eRetCode).c_str());
		cRetRoot.add("title", pcReq->m_cErrorTitle.c_str());
		cRetRoot.add("id", pcReq->m_cIdStr.c_str());
		cRetRoot.add("path", pcReq->m_cPath.c_str());

		cxx::lang::String cResp;
		cRetRoot.to_string<cxx::lang::String>(cResp);

		strcpy(piReadBuffer, cResp.c_str());
		iWriteSize = cResp.size()+1;
	}

	delete pcReq; pcReq = 0;

	return iWriteSize;
}

/**
 * enqueueREQ
 * API->fabcassi. Contains the API request from the NBI
 * @param pcReq - the JSON request info.
 * @return 0 on success
 */
inttype JsonApi::enqueueREQ(JsonApiRequest* pcReq) {
	if (true == m_cApiToCassi.isFull()) {
		return -1;
	}

	if (0 != m_cApiToCassi.enqueue(pcReq)) {
		return -2;
	}

	return 0;
}

/**
 * dequeueREP
 * fabcassi->API. Contains the response data from handlers.
 * @param pcReq [out] - returns a REQ block with return information
 * provided.
 * @return 0 on success.
 */
inttype JsonApi::dequeueREP(JsonApiRequest*& pcReq) {
	if (true == m_cCassiToApi.isEmpty()) {
		return 1;
	}

	voidptr pReq = 0;

	if (0 != m_cCassiToApi.dequeue(pReq)) {
		return -1;
	}

	pcReq = static_cast<JsonApiRequest* >(pReq);
	if (0 == pcReq) {
		return -2;
	}

	return 0;
}

/**
 * dequeueREQ
 * Called by fabcassi to process the API request.
 * @param pcReq [out] - returns a REQ block
 * @return 0 on success
 */
inttype JsonApi::dequeueREQ(JsonApiRequest*& pcReq) {
	if (true == m_cApiToCassi.isEmpty()) {
		return 1;
	}

	voidptr pReq = 0;

	if (0 != m_cApiToCassi.dequeue(pReq)) {
		return -1;
	}

	pcReq = static_cast<JsonApiRequest* >(pReq);
	if (0 == pcReq) {
		return -2;
	}

	return 0;
}

/**
 * enqueueREP
 * Called by fabcassi to send the REQ block with the
 * provided return information.
 * @param pcReq - the response block.
 * @return 0 on success.
 */
inttype JsonApi::enqueueREP(JsonApiRequest* pcReq) {
	if (true == m_cCassiToApi.isFull()) {
		return -1;
	}

	if (0 != m_cCassiToApi.enqueue(pcReq)) {
		return -2;
	}

	return 0;
}

/**
 * enablePoller
 * Enable epolling of the FabCassi->API Queue.
 * @return 0 on success
 */
inttype JsonApi::enablePoller(void) {
	inttype iFd = m_cCassiToApi.eventFd();
	errno = 0;

	m_iApiEpollFd = epoll_create(cxx::io::MAXCONN);
	if (-1 == m_iApiEpollFd) {
		DEEP_LOG_ERROR(OTHER, "Unable to create an epoll FD\n");
		return -1;
	}

	errno = 0;

	inttype iStatus = cxx::io::modifyEpollContext(m_iApiEpollFd, EPOLL_CTL_ADD, iFd, EPOLLIN, 0);
	if (0 != iStatus) {
		DEEP_LOG_ERROR(OTHER, "Unable to add API poller FD to the epoll fd.\n");
		return -2;
	}

	return 0;
}


/**
 * pollREP
 * Poll for REPLY items from the FabCassi. If enablePoller was
 * called then we efficiently wait for events to happen so we can
 * check. Otherwise we run in a hot mode.
 * @param iTimeout - milliseconds to timeout waiting for an event.
 * @param bHasWork [out] - true or false if there are items on the
 * queue.
 * @return 0 on success, errors mean bad things happened.
 */
inttype JsonApi::pollREP(ulongtype iTimeout, boolean& bHasWork) {
	if (-1 == m_iApiEpollFd) {
		/** Run hot */
		bHasWork = apiHasWork();
		return 0;
	}

	errno = 0;

	inttype iEventCount = epoll_wait(m_iApiEpollFd, &m_stApiEvents, 1, iTimeout);
	if (-1 == iEventCount) {
		if (EINTR != errno) {
			return -1;
		}
	}

	/** Whether we have any events or not, check the Queue. */
	bHasWork = apiHasWork();

	return 0;
}

/**
 * cassHasWorkPoll
 * Blocking call for the given timeout for any event on the API to
 * CASSI message queue.
 * @param iTimeout - milliseconds to block and wait.
 * @return true if there is work, else false.
 */
boolean JsonApi::cassHasWorkPoll(ulongtype iTimeout) {
	boolean bHasWork = false;

	if (0 != m_cApiToCassi.poller(iTimeout, bHasWork)) {
		throw JsonApiException("Failed polling the API to consumer FD.");
	}

	return bHasWork;
}
