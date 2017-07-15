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
#ifndef COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_FABRICAPIHANDLERS_H
#define COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_FABRICAPIHANDLERS_H

namespace com { namespace deepis { namespace communication { namespace fabricconnector {

/**
 * Fabric Handler for doing the AddPeer POST request.
 * The POST JSON includes an attributes block that contains the
 * address for the peer to connect to. The UUID to associate with this
 * peer is provided to us in the REQuest block. This callback need to
 * map the /fabric/peer/UUDI and the UUID together so upon delPeer we can
 * quickly find the UUID. See addMapping in the delete handler.
 * The full path for this Peer is automatically registered.
 */
class FabricApiHandler : public ApiCallback {
public:
	FabricApiHandler(FabricCassiMessageQueue* pcParent) : m_pcParent(pcParent) {
	}

	virtual ~FabricApiHandler(){
		m_pcParent = 0;
	}

	virtual inttype callback(JsonApiRequest* pcReq) {
		if (JsonApi::POST != pcReq->m_eRequest) {
			pcReq->m_eRetCode = JsonApi::HANDLER_BAD_REQUEST;
			pcReq->m_cErrorTitle = "Only POST requests are handled for this path.";

			return IMMEDIATE_REPLY_RETVAL;
		}

		if (false == pcReq->m_pcData->exists("attributes")) {
			pcReq->m_eRetCode = JsonApi::HANDLER_BAD_REQUEST;
			pcReq->m_cErrorTitle = "The attributes are missing in the request.";

			return IMMEDIATE_REPLY_RETVAL;
		}

		json::object* pcAttr = pcReq->m_pcData->operator[]("attributes").to_object();

		if (false == pcAttr->exists("address")) {
			pcReq->m_eRetCode = JsonApi::HANDLER_BAD_REQUEST;
			pcReq->m_cErrorTitle = "The address is missing in the attributes.";

			return IMMEDIATE_REPLY_RETVAL;
		}

		json::value cAddress = pcAttr->operator[]("address");
		cxx::lang::String cAddr;
		if (false == cAddress.str(cAddr)) {
			pcReq->m_eRetCode = JsonApi::HANDLER_BAD_REQUEST;
			pcReq->m_cErrorTitle = "The address is unreadable.";

			return IMMEDIATE_REPLY_RETVAL;
		}

		m_pcParent->addPeerCmd(pcReq, pcReq->m_cPath, cAddr, *(pcReq->m_pcUUID));

		return DEFER_REPLY_RETVAL;
	}

private:
	FabricCassiMessageQueue* m_pcParent;

}; /** FabricApiHandler */

/**
 * Fabric Handler for doing the DelPeer POST request.
 */
class DelPeerApiHandler : public ApiCallback {
public:
	DelPeerApiHandler(FabricCassiMessageQueue* pcParent) :
		m_pcParent(pcParent),
		m_cMap(16, true, true, false) {
	}

	virtual ~DelPeerApiHandler(){
		m_pcParent = 0;
	}

	inttype addMapping(cxx::lang::String& cPath, cxx::util::UUID& cUUID, cxx::lang::String& cRetVal) {
		if (0 != m_cMap.get(&cPath)) {
			return -1;
		}

		std::stringstream ss;
		ss << cPath << '/' << cUUID.toString();

		cxx::lang::String* pcKey = new cxx::lang::String(ss.str());
		cxx::util::UUID*   pcVal = new cxx::util::UUID(cUUID.getMostSignificantBits(),
							       cUUID.getLeastSignificantBits());

		if (0 != m_cMap.put(pcKey, pcVal)) {
			return -2;
		}

		cRetVal = *pcKey;

		return 0;
	}

	virtual inttype callback(JsonApiRequest* pcReq) {
		if (JsonApi::DELETE != pcReq->m_eRequest) {
			pcReq->m_eRetCode = JsonApi::HANDLER_BAD_REQUEST;
			pcReq->m_cErrorTitle = "Only DELETE requests are handled for this path.";

			return IMMEDIATE_REPLY_RETVAL;
		}

		cxx::util::UUID* pcVal = m_cMap.get(&(pcReq->m_cPath));
		if (0 == pcVal) {
			pcReq->m_eRetCode = JsonApi::HANDLER_NOT_FOUND;
			pcReq->m_cErrorTitle = "No path found to delete this peer.";

			return IMMEDIATE_REPLY_RETVAL;
		}

		if (0 == m_pcParent->delPeerCmd(pcReq, *pcVal)) {
			m_cMap.remove(&(pcReq->m_cPath));
		}

		return DEFER_REPLY_RETVAL;
	}

private:
	FabricCassiMessageQueue*                                 m_pcParent;
	cxx::util::HashMap<cxx::lang::String*, cxx::util::UUID*> m_cMap;

}; /** DelPeerApiHandler */

class FabricTopoApiHandler : public ApiCallback {
public:
	FabricTopoApiHandler(FabricCassiMessageQueue* pcParent) : m_pcParent(pcParent) {
	}

	virtual ~FabricTopoApiHandler(){
		m_pcParent = 0;
	}

	virtual inttype callback(JsonApiRequest* pcReq) {
		if (JsonApi::GET != pcReq->m_eRequest) {
			pcReq->m_eRetCode = JsonApi::HANDLER_BAD_REQUEST;
			pcReq->m_cErrorTitle = "Only GET requests are handled for this path.";

			return IMMEDIATE_REPLY_RETVAL;
		}

		DEEP_LOG_DEBUG(OTHER, "/fabric/topology GET\n");

		if (0 != m_pcParent->getTopo(pcReq)) {
			pcReq->m_eRetCode = JsonApi::HANDLER_INTERNAL_ERROR;
			pcReq->m_cErrorTitle = "Unable to process the GET Topology request.\n";

			return IMMEDIATE_REPLY_RETVAL;
		}

		return DEFER_REPLY_RETVAL;
	}

private:
	FabricCassiMessageQueue*                                 m_pcParent;
};

class CassiCommandHandler : public ApiCallback {
public:

	CassiCommandHandler(FabricCassiMessageQueue* pcParent) : m_pcParent(pcParent) {

	}

	virtual ~CassiCommandHandler(){
		m_pcParent = 0;
	}

	virtual inttype callback(JsonApiRequest* pcReq) {
		DEEP_LOG_DEBUG(OTHER, "/cassi/commands POST\n");

		m_pcParent->processCassiCommand(pcReq);
		return IMMEDIATE_REPLY_RETVAL;
	}

private:
	FabricCassiMessageQueue* m_pcParent;

}; /** CassiCommandHandler */

} } } } // namespace

#endif /** COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_FABRICAPIHANDLERS_H */
