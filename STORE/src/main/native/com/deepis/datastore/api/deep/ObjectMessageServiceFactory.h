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
#ifndef COM_DEEPIS_DATASTORE_API_DEEP_OBJECTMESSAGESERVICEFACTORY_H_
#define COM_DEEPIS_DATASTORE_API_DEEP_OBJECTMESSAGESERVICEFACTORY_H_

#include "com/deepis/db/store/relative/util/InvalidException.h"
#include "com/deepis/datastore/api/deep/DeepTypeDefs.h"
#include "com/deepis/db/store/relative/distributed/ObjectMessageService.h"

namespace com { namespace deepis { namespace db { namespace store { namespace relative { namespace distributed {

class ObjectMessageServiceFactory {
	public:
		static IMessageService* createMessageService(int keyType) {
			IMessageService* service = null;
			
			switch(keyType) {
			case CT_DATASTORE_LONGLONG:
				service = new ObjectMessageService<DeepLongLong>();
				break;
			case CT_DATASTORE_ULONGLONG:
				service = new ObjectMessageService<DeepULongLong>();
				break;
			case CT_DATASTORE_LONG_INT:
				service = new ObjectMessageService<DeepLongInt>();
				break;
			case CT_DATASTORE_ULONG_INT:
				service = new ObjectMessageService<DeepULongInt>();
				break;
			#ifndef DEEP_REDUCE_TEMPLATES
			case CT_DATASTORE_SHORT_INT:
				service = new ObjectMessageService<DeepShortInt>();
				break;
			case CT_DATASTORE_USHORT_INT:
				service = new ObjectMessageService<DeepUShortInt>();
				break;
			case CT_DATASTORE_INT8:
				service = new ObjectMessageService<DeepTinyInt>();
				break;
			case CT_DATASTORE_UINT8:
				service = new ObjectMessageService<DeepUTinyInt>();
				break;
			case CT_DATASTORE_FLOAT:
				service = new ObjectMessageService<DeepFloat>();
				break;
			case CT_DATASTORE_DOUBLE:
				service = new ObjectMessageService<DeepDouble>();
				break;
			#endif
			case CT_DATASTORE_INT24:
			case CT_DATASTORE_UINT24:
			case CT_DATASTORE_NUMBER_NULL:
			case CT_DATASTORE_BINARY:
			case CT_DATASTORE_VARBINARY1:
			case CT_DATASTORE_VARBINARY2:
			case CT_DATASTORE_FIXED_VARBINARY1:
			case CT_DATASTORE_TEXT:
			case CT_DATASTORE_TEXT_MULTIBYTE:
			case CT_DATASTORE_VARTEXT1:
			case CT_DATASTORE_VARTEXT2:
			case CT_DATASTORE_FIXED_VARTEXT1:
				service = new ObjectMessageService<DeepNByte*>();
				break;
			case CT_DATASTORE_COMPOSITE:
				service = new ObjectMessageService<DeepComposite*>();
				break;
			default:
				throw UnsupportedOperationException("Invalid key type");
		}

		return service;
	}
};

} } } } } } // namespace

#endif /* COM_DEEPIS_DATASTORE_API_DEEP_OBJECTMESSAGESERVICEFACTORY_H_ */
