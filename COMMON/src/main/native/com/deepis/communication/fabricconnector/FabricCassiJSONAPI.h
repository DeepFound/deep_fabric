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
#ifndef COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_JSONAPI_H_
#define COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_JSONAPI_H_

#include <sstream>
#include <json.h>
#include <json_parser.h>

#include "cxx/util/HashMap.h"
#include "com/deepis/communication/common/FabricEnvelope.h"
#include "com/deepis/communication/common/MsgIds.h"
#include "com/deepis/communication/fabricconnector/FabricCassiMessageQueue.h"
#include "com/deepis/communication/fabricconnector/RealtimeFabricAPI.h"

using namespace com::deepis::communication::common;

namespace com { namespace deepis { namespace communication { namespace fabricconnector {

class JsonApiException : public Exception {
public:
	JsonApiException(void) {
	}

	JsonApiException(const char* message)
		: Exception(message) {
	}
}; /**  */

class JsonApiRequest;

class JsonApi : public cxx::lang::Runnable, public RealtimeFabricAPIServer {
public:
	JsonApi(uinttype iServerId);
	JsonApi(uinttype iServerId, ushorttype iPort);

	virtual ~JsonApi();

	virtual void run(void);

	virtual inttype handler(bytetype* piReadBuffer,
				inttype   iReadSize);

	/** API methods */
	inttype enqueueREQ(JsonApiRequest* pcReq);
	inttype dequeueREP(JsonApiRequest*& pcReq);

	/** FabricCassiMessageQ methods */
	inttype dequeueREQ(JsonApiRequest*& pcReq);
	inttype enqueueREP(JsonApiRequest* pcReq);

	boolean cassiHasWork() {
		return ((false == m_cApiToCassi.isEmpty()) ? true : false);
	}

	boolean cassHasWorkPoll(ulongtype iTimeout);

	boolean apiHasWork() {
		return ((false == m_cCassiToApi.isEmpty()) ? true : false);
	}

	inttype enablePoller(void);
	inttype pollREP(ulongtype iTimeout, boolean& bHasWork);

	enum requestCodes {
		UNKNOWN                = 0,
		POST                   = 1,
		DELETE                 = 2,
		PATCH                  = 3,
		GET                    = 4,
	};

	static requestCodes requestCode(cxx::lang::String& cRequest) {
		if (0 == cRequest.compare("GET")) {
			return GET;
		} else if (0 == cRequest.compare("POST")) {
			return POST;
		} else if (0 == cRequest.compare("DELETE")) {
			return DELETE;
		} else if (0 == cRequest.compare("PATCH")) {
			return PATCH;
		}

		return UNKNOWN;
	}

	enum handlerCodes {
		/* +ve responses */
		HANDLER_OK             = 200,
		HANDLER_CREATED        = 201,
		HANDLER_ACCEPTED       = 202,
		HANDLER_OK_NO_CONTENT  = 204,

		/* -ve responses */
		HANDLER_BAD_REQUEST    = 400,
		HANDLER_UNAUTHORIZED   = 401,
		HANDLER_FORBIDDEN      = 403,
		HANDLER_NOT_FOUND      = 404,

		HANDLER_INTERNAL_ERROR = 500,
	};

	static cxx::lang::String returnStatus(handlerCodes eCode) {
		switch (eCode) {
		case HANDLER_OK:
			return "OK";
		case HANDLER_CREATED:
			return "Created";
		case HANDLER_ACCEPTED:
			return "Accepted";
		case HANDLER_OK_NO_CONTENT:
			return "No Content";

		case HANDLER_BAD_REQUEST:
			return "Bad Request";
		case HANDLER_UNAUTHORIZED:
			return "Unauthorized";
		case HANDLER_FORBIDDEN:
			return "Forbidden";
		case HANDLER_NOT_FOUND:
			return "Not Found";

		case HANDLER_INTERNAL_ERROR:
			return "Internal Server Error";

		default:
			break;
		};

		return "Unknown code";
	}

	static boolean positiveReturn(handlerCodes eCode) {
		return ((200 <= eCode) && (299 >= eCode) ? true : false);
	}

	void stopService(void) { m_bStop = true; }

private:
	uinttype           m_iServerId;
	cxx::lang::String  m_cSocketName;
	boolean            m_bStop;

	cxx::io::MsgQueue  m_cCassiToApi;
	cxx::io::MsgQueue  m_cApiToCassi;
	inttype            m_iApiEpollFd;
	struct epoll_event m_stApiEvents;

}; /** JsonApi */

class JsonApiRequest : public cxx::lang::Object {
public:
	JsonApiRequest() : m_eRequest(JsonApi::UNKNOWN),
			   m_pcJson(0),
			   m_pcUUID(0),
			   m_pcData(0),
			   m_pcRetData(0),
			   m_eRetCode(JsonApi::HANDLER_OK) {
	}

	virtual ~JsonApiRequest() {
		if (0 != m_pcUUID) {
			delete m_pcUUID; m_pcUUID = 0;
		}

		if (0 != m_pcData) {
			// Is a reference into m_pcJson
			m_pcData = 0;
		}

		if (0 != m_pcRetData) {
			delete m_pcRetData; m_pcRetData = 0;
		}

		if (0 != m_pcJson) {
			delete m_pcJson; m_pcJson = 0;
		}
	}

	JsonApi::requestCodes m_eRequest;
	json::root*           m_pcJson;
	cxx::util::UUID*      m_pcUUID;
	cxx::lang::String     m_cIdStr;
	cxx::lang::String     m_cPath;
	json::object*         m_pcData;

	json_object*          m_pcRetData;
	JsonApi::handlerCodes m_eRetCode;
	cxx::lang::String     m_cErrorTitle;

}; /** JsonApiRequest */

class ApiCallback : public cxx::lang::Object {
public:
	virtual ~ApiCallback() {
	}

	static const inttype DEFER_REPLY_RETVAL     = 1;
	static const inttype IMMEDIATE_REPLY_RETVAL = 0;
	/** -VE values are errors. */

	virtual inttype callback(JsonApiRequest* pcReq) {
		return IMMEDIATE_REPLY_RETVAL;
	}

}; /** ApiCallback */


} // fabricconnector
} // communication
} // deepis
} // com

#endif /** COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_JSONAPI_H_ */
