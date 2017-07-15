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
#ifndef COM_DEEPIS_COMMUNICATION_COMMON_MSGIDS_H_
#define COM_DEEPIS_COMMUNICATION_COMMON_MSGIDS_H_

#include "cxx/lang/types.h"

#define MSG_ID(_PREFIX, _ID) static_cast<uinttype>(((_PREFIX & 0xffff) << 16) | (_ID & 0xffff))
#define HAS_PREFIX(_PREFIX, _MSG_ID) static_cast<boolean>((_PREFIX == ((_MSG_ID >> 16) & 0xffff)) ? true : false)

namespace com { namespace deepis { namespace communication { namespace common {

namespace msgPrefix {

/**
 * This is the master list of message ID prefixes.
 *
 * Always append. Never delete or renumber these prefixes.
 * Mark any depricated prefixes.
 */
static const uinttype REALTIME_FABRIC_INTERNAL   = 0x0001;
static const uinttype REALTIME_FABRIC_CASSI_SHIM = 0x0002;
static const uinttype REALTIME_DISTRIBUTED_CASSI = 0x0003;
static const uinttype FILE_REPLICATION           = 0x0004;

} } } } } // namespace

#endif /** COM_DEEPIS_COMMUNICATION_COMMON_MSGIDS_H_ */
