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
#ifndef COM_DEEPIS_DATASTORE_API_DEEP_TYPEDEFS_H_
#define COM_DEEPIS_DATASTORE_API_DEEP_TYPEDEFS_H_

typedef inttype    DeepLongInt;
typedef uinttype   DeepULongInt;
typedef shorttype  DeepShortInt;
typedef ushorttype DeepUShortInt;
typedef longtype   DeepLongLong;
typedef ulongtype  DeepULongLong;
typedef chartype   DeepTinyInt;
typedef uchartype  DeepUTinyInt;
typedef doubletype DeepDouble;
typedef floattype  DeepFloat;

class DeepNByte : public nbyte {
	public:
		DeepNByte(const inttype size):
			nbyte(size) {
		}

		DeepNByte(const nbyte* bytes):
			nbyte(bytes) {
		}

		DeepNByte(const bytearray data, const inttype size):
			nbyte(data, size) {
		}

		DeepNByte(const voidarray data, const inttype size):
			nbyte(data, size) {
		}
};

class DeepComposite : public nbyte {
	private:
		uinttype m_keyPartMask;

		/* weight of missing key parts:
		 *     < 0 - missing key parts are lower
		 *     = 0 - missing key parts are equal
		 *     > 0 - missing key parts are higher
		 */
		bytetype m_keyPartWeight;

		bytetype m_keyPartMatch;

		boolean m_ignorePrimary:1;

	public:
		DeepComposite(const inttype size, uinttype keyPartMask = ~0, bytetype keyPartWeight = -1):
			nbyte(size),
			m_keyPartMask(keyPartMask),
			m_keyPartWeight(keyPartWeight),
			m_keyPartMatch(-1),
			m_ignorePrimary(false) {
		}

		DeepComposite(const nbyte* bytes, uinttype keyPartMask = ~0, bytetype keyPartWeight = -1):
			nbyte(bytes),
			m_keyPartMask(keyPartMask),
			m_keyPartWeight(keyPartWeight),
			m_keyPartMatch(-1),
			m_ignorePrimary(false) {
		}

		DeepComposite(const bytearray data, const inttype size, uinttype keyPartMask = ~0, bytetype keyPartWeight = -1):
			nbyte(data, size),
			m_keyPartMask(keyPartMask),
			m_keyPartWeight(keyPartWeight),
			m_keyPartMatch(-1),
			m_ignorePrimary(false) {
		}

		DeepComposite(const voidarray data, const inttype size, uinttype keyPartMask = ~0, bytetype keyPartWeight = -1):
			nbyte(data, size),
			m_keyPartMask(keyPartMask),
			m_keyPartWeight(keyPartWeight),
			m_keyPartMatch(-1),
			m_ignorePrimary(false) {
		}

		DeepComposite* reassignComposite(const bytearray data, const inttype size, uinttype keyPartMask = ~0, bytetype keyPartWeight = -1, bytetype keyPartMatch = -1) {
			m_keyPartMask = keyPartMask;
			m_keyPartWeight = keyPartWeight;
			m_keyPartMatch = keyPartMatch;

			reassign(data, size);

			return this;
		}

		FORCE_INLINE void setKeyPartMask(uinttype keyPartMask) {
			m_keyPartMask = keyPartMask;
		}

		FORCE_INLINE uinttype getKeyPartMask() const {
			return m_keyPartMask;
		}

		FORCE_INLINE void setKeyPartWeight(bytetype keyPartWeight) {
			m_keyPartWeight = keyPartWeight;
		}

		FORCE_INLINE bytetype getKeyPartWeight() const {
			return m_keyPartWeight;
		}

		FORCE_INLINE void calculateKeyPartMatch(uinttype keyPartMask) {
			bytetype i=0;

			for (; (keyPartMask >>= 1) != 0; i++) ;

			m_keyPartMatch = i;
		}

		FORCE_INLINE void setKeyPartMatch(bytetype keyPartMatch) {
			m_keyPartMatch = keyPartMatch;
		}

		FORCE_INLINE bytetype getKeyPartMatch() const {
			return m_keyPartMatch;
		}

		FORCE_INLINE void setIgnorePrimary(boolean flag) {
			m_ignorePrimary = flag;
		}

		FORCE_INLINE boolean getIgnorePrimary() const {
			return m_ignorePrimary;
		}
};

#endif /* COM_DEEPIS_DATASTORE_API_DEEP_TYPEDEFS_H_ */
