///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXTYPECODES_H__ )
#define __TXTYPECODES_H__

//
// txObject Base Types (will add #defines for different platforms if needed)
//

class txObjectType
{
	public:
		typedef long Long;
		typedef char Char;
		typedef short Short;
		typedef float Float;
		typedef char* charPtr;
		typedef double Double;
		typedef unsigned long ULong;
		typedef unsigned char Octet;
		typedef unsigned short UShort;
		typedef unsigned char Boolean;
		// typedef long long LongLong;
		// typedef long double LongDouble;
		// typedef unsigned long long ULongLong;
};

//
// txObject Constructor Values
//

enum TX_AUTODEL_FLAG
{
	TX_AUTODEL_ON = 1,
	TX_AUTODEL_OFF = 0
};

enum TX_COMPARE_FLAG
{
	TX_COMPARE_V2K = 0,
	TX_COMPARE_K2V = 1
};

TX_AUTODEL_FLAG TX_GET_AUTODEL_FLAG (unsigned long value);
TX_COMPARE_FLAG TX_GET_COMPARE_FLAG (unsigned long value);

void TX_SET_AUTODEL_FLAG (unsigned long& value, TX_AUTODEL_FLAG);
void TX_SET_COMPARE_FLAG (unsigned long& value, TX_COMPARE_FLAG);

//
// txObject Serialization Header Types
//

const unsigned long TX_NULL		= 0x00000000;	// v1.0

const unsigned long TX_BOOLEAN_TRUE	= 0x10000001;	// v1.0
const unsigned long TX_BOOLEAN_FALSE	= 0x10000000;	// v1.0

const unsigned long TX_SIGNED_CHAR8	= 0x20000004;	// v1.0
const unsigned long TX_UNSIGNED_CHAR8	= 0x20000005;	// v1.0
const unsigned long TX_SIGNED_CHAR16	= 0x20000006;	// v1.0
const unsigned long TX_UNSIGNED_CHAR16	= 0x20000007;	// v1.0
	
const unsigned long TX_SIGNED_INT8	= 0x30000004;	// v1.0
const unsigned long TX_SIGNED_INT16	= 0x30000005;	// v1.0
const unsigned long TX_SIGNED_INT32	= 0x30000006;	// v1.0
const unsigned long TX_SIGNED_INT64	= 0x3000000B;	// v1.0

const unsigned long TX_UNSIGNED_INT8	= 0x40000004;	// v1.0
const unsigned long TX_UNSIGNED_INT16	= 0x40000005;	// v1.0
const unsigned long TX_UNSIGNED_INT32	= 0x40000006;	// v1.0
const unsigned long TX_UNSIGNED_INT64	= 0x4000000B;	// v1.0

const unsigned long TX_FLOAT		= 0x50000004;	// v1.0
const unsigned long TX_DOUBLE		= 0x50000009;	// v1.0

const unsigned long TX_ASCII_STRING	= 0x60000000;	// v1.0
const unsigned long TX_UTF8_STRING	= 0x60000001;	// v1.0
const unsigned long TX_UTF16_STRING	= 0x60000002;	// v1.0
const unsigned long TX_EMPTY_STRING	= 0x60000003;	// v1.0

const unsigned long TX_BYTE_ARRAY	= 0x70000000;	// v1.0
const unsigned long TX_BYTE_ARRAYR0	= 0x70000000;	// v1.0
const unsigned long TX_BYTE_ARRAYR1	= 0x70000001;	// v1.0
const unsigned long TX_BYTE_ARRAYR2	= 0x70000002;	// v1.0
const unsigned long TX_BYTE_ARRAYR3	= 0x70000003;	// v1.0

const unsigned long TX_LIST		= 0x80000000;	// v1.0
const unsigned long TX_SET		= 0x80000001;	// v1.0
const unsigned long TX_MAP		= 0x80000002;	// v1.0
const unsigned long TX_TREE		= 0x80000003;	// v1.0

const unsigned long TX_GTYPE_INT32	= 0x90000000;	// v1.0
const unsigned long TX_GTYPE_INT64	= 0x90000001;	// v1.0
const unsigned long TX_GTYPE_STRING	= 0x90000002;	// v1.0
const unsigned long TX_GTYPE_OBJECT	= 0x90000003;	// v1.0

const unsigned long TX_OBJREF_INT32	= 0xA0000000;	// v1.0
const unsigned long TX_OBJREF_INT64	= 0xA0000001;	// v1.0
const unsigned long TX_OBJREF_STRING	= 0xA0000002;	// v1.0
const unsigned long TX_OBJREF_OBJECT	= 0xA0000003;	// v1.0

const unsigned long TX_REFOBJ_INT32	= 0xB0000000;	// v1.0
const unsigned long TX_REFOBJ_INT64	= 0xB0000001;	// v1.0
const unsigned long TX_REFOBJ_STRING	= 0xB0000002;	// v1.0
const unsigned long TX_REFOBJ_OBJECT	= 0xB0000003;	// v1.0

//
// txObject Serialization Header Masks
//

const unsigned long TX_BASETYPE		= 0xF0000000;	// v1.0
const unsigned long TX_SUBTYPE		= 0x00000003;	// v1.0
const unsigned long TX_FULLTYPE		= 0xF0000003;	// v1.0
const unsigned long TX_LENGTH		= 0x0FFFFFFC;	// v1.0

#endif // __TXTYPECODES_H__
