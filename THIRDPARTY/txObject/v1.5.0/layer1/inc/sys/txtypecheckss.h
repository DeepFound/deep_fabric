///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if !defined ( __TXTYPECHECKSS_H__ )
#define __TXTYPECHECKSS_H__

class txList;
class txString;
class txHashSet;
class txHashMap;

class txTypeCheckSS
{
	private:
		static txHashMap* _subclassing_;
		static txHashMap* _superclassing_;
		static txHashSet* _class_type_set_;

	private:
		static void _connect_ (
			txHashMap* map,
			const char* src,
			const char* dest);

		static int _isConnected_ (
			txHashMap* map,
			const txString& src,
			const txString& dest);

		static int _findClassRelations_ (
			txHashMap* map,
			const txString& src,
			txList& user);

		static void _addTypeToList_ (
			txString& src,
			txList& user);

		static void _flushClassTypeSet_ (
			txHashSet* set);

		static void _flushClassDictionary_ (
			txHashMap* map);

	public:
		static void addRelation (const char* sub, const char* super)
		{
			startup(); // to ensure memory init. before main.

			_connect_(_subclassing_, sub, super);
			_connect_(_superclassing_, super, sub);
		}

		static int isSubClass (const txString& sub,
			const txString& oftype)
		{
			return ((!isShutdown() && isClass(sub, oftype)) || 
				_isConnected_(_subclassing_, sub, oftype));
		}

		static int isSuperClass (const txString& super,
			const txString& oftype)
		{
			return ((!isShutdown() && isClass(super, oftype)) || 
				_isConnected_(_superclassing_, super, oftype));
		}

		static void subClasses (const txString& src, txList& l)
		{
			if (!isShutdown())
			{
				_addTypeToList_((txString&) src, l);
				_findClassRelations_(_subclassing_, src, l);
			}
		}

		static void superClasses (const txString& src, txList& l)
		{
			if (!isShutdown())
			{
				_addTypeToList_((txString&) src, l);
				_findClassRelations_(_superclassing_, src, l);
			}
		}

		static const txList* immediateSuperClasses (const txString&);

		static const txList* immediateSubClasses (const txString&);

		static int isClass (const txString&, const txString&);

		static const txString* findRTTIType (const char*);

		static int subClassEntries (void);

		static int superClassEntries (void);

		static void startup (void);

		static void shutdown (void);

		static int isShutdown (void)
		{
			return _subclassing_ ? 0 : 1; // or _superclassing_
		}
};

//
// txHash : txObject id generator
//

unsigned long txHash (const char* data, unsigned long length);

#endif // __TXTYPECHECKSS_H__
