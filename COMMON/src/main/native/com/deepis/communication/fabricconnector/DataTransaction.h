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
#ifndef COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_DATATRANSACTION_H_
#define COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_DATATRANSACTION_H_

#include "com/deepis/db/store/relative/distributed/Facilitator.h"
#include "com/deepis/db/store/relative/distributed/LogEntry.h"
#include "com/deepis/db/store/relative/distributed/QuickList.h"
#include "com/deepis/db/store/relative/distributed/Serializer.h"
#include "com/deepis/communication/common/FabricEnvelope.h"
#include "cxx/lang/Runnable.h"

using namespace com::deepis::communication::common;
using namespace com::deepis::db::store::relative::distributed;
using namespace cxx::lang;

namespace com { namespace deepis { namespace communication { namespace fabricconnector {

template<typename K>
class DataTransaction : public Runnable {
	private:

		Facilitator<K>* m_facilitator;
		QuickList<DataLogEntry<K>*>* m_data;

	public:
		DataTransaction(Facilitator<K>* facilitator) :
			m_facilitator(facilitator),
			m_data(null) {
		}

		void addDataLogEntry(encodeProtocol::reader* message) {
			if (m_data == null) {
				m_data = new QuickList<DataLogEntry<K>*>();
			}
			DataLogEntry<K>* dle = new DataLogEntry<K>();

			Serializer<K>::deserialize(message, dle);
			m_data->addEntry(dle);
		}

		void setData(QuickList<DataLogEntry<K>*>* data) {
			m_data = data;
		}

		virtual void run() {
			m_facilitator->processDataTransaction(m_data);
		}

		virtual ~DataTransaction() {
			delete m_data;
		}
};

} } } } // namespace

#endif /* COM_DEEPIS_COMMUNICATION_FACBRICCONNECTOR_DATATRANSACTION_H_ */
