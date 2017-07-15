///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXOUT_H__ )
#define __TXOUT_H__

#include "txsmrtbuf.h" 
#include "txtypecodes.h" 
 
class txObject;
class txHashMap;
 
class txOut
{
	private: 
		short _ref_objs_;
		txHashMap* _map_context_;
		unsigned long _userflags_;
		txSmartBuffer _datastream_;

	private:
		inline unsigned long _put_ (txObject*);
		inline void _writeChar_ (signed char);
		inline void _writeUChar_ (unsigned char);
		inline void _prependUInt_ (unsigned long);
		inline void _writeHeader_ (unsigned long);
		inline void _writeStr_ (const char*, unsigned long);
		inline void _writeHeader_ (unsigned long, unsigned long);
		inline void _validateRefContainers_ (txOut* out_stream = 0);
		inline unsigned long _calHeader_ (unsigned long, unsigned long);

	public:
		void writeUInt (unsigned long);
		void writeRawHeader (unsigned long);
		void writeHeader (unsigned long, unsigned long, unsigned long);

	public: 
		txOut (void);  // object referencing is off by default
		~txOut (void);

		void putNull (void);

		txOut& operator<< (float o);			// 32 bits
		txOut& operator<< (double o);			// 64 bits

		txOut& operator<< (const char*);		// ASCII

		txOut& operator<< (signed char o);		// 8 bits
		txOut& operator<< (unsigned char o);		// 8 bits

		txOut& operator<< (signed short int o);		// 16 bits
		txOut& operator<< (signed long int o);		// 32 bits 

		txOut& operator<< (unsigned short int o);	// 16 bits
		txOut& operator<< (unsigned long int o);	// 32 bits

		txOut& operator<< (const txObject*);
		txOut& operator<< (const txObject&);

		void put (const txObject*);
		void put (const txObject&);
		void put (const char*, unsigned long, unsigned long);

		void append (const txOut& out);
		void append (const char* data, int length);

		unsigned int length (void) const;

		const char* data (void) const;
	
		void flush (void);

		// Referencing Utilities //

		int objectReferencing (void) const;

		void objectReferencingOff (void);

		void objectReferencingOn (void);

		// User Flag Utilities //

		unsigned long userFlags (void) const;

		void userFlags (unsigned long);
};

#endif // __TXOUT_H__
