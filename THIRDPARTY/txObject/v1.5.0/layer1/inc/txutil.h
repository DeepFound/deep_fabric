///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#if !defined ( __TXUTIL_H__ )
#define __TXUTIL_H__

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_STATIC_ALLOC_NOTIFY(f)                                             \
        class txStaticNotify##f                                               \
        {                                                                     \
                public:                                                       \
                       txStaticNotify##f (void) { f(); }                      \
        };                                                                    \
                                                                              \
        static txStaticNotify##f txStaticNotifyValue##f;

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_STATIC_DEALLOC_NOTIFY(f)                                           \
        class txStaticNotify##f                                               \
        {                                                                     \
                public:                                                       \
                       ~txStaticNotify##f (void) { f(); }                     \
        };                                                                    \
                                                                              \
        static txStaticNotify##f txStaticNotifyValue##f;

////////////////////////////////// USER MACRO /////////////////////////////////
#if defined OSTORE_SUPPORT
        #include <ostore/ostore.hh>

        #define TX_PERSIST                                                    \
                public:                                                       \
                static os_typespec* get_os_typespec();

        #define TX_GET_OBJECT_CLUSTER(ptr) os_segment::of(ptr)
        #define TX_IS_OBJECT_PERSISTENT(ptr) objectstore::is_persistent(ptr)
#else
        #define TX_PERSIST
#endif

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_CRASH (*(int*) 0x0 = 1);

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_DECLARE_STREAM_TYPE(c)                                             \
        TX_DECLARE_STREAM_TYPE_WITH_ALIAS(c,c)

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_DECLARE_STREAM_TYPE_WITH_ALIAS(c,alias)                            \
        public:                                                               \
        TX_DEFINE_TYPE(alias)                                                 \
        friend txStream* txCreate##c##Func (void);

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_DEFINE_STREAM_TYPE(c,p)                                            \
        TX_DEFINE_STREAM_TYPE_WITH_ALIAS(c,p,c)

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_DEFINE_STREAM_TYPE_WITH_ALIAS(c,p,alias)                           \
        TX_DEFINE_PARENT_TYPE_WITH_ALIAS(c,p,alias)                           \
                                                                              \
        txStream* txCreate##c##Func (void)                                    \
        {                                                                     \
                return new c();                                               \
        }                                                                     \
                                                                              \
        static txFactoryBinder txFactoryBinder##c(c::Type,txCreate##c##Func);

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_DECLARE_STREAM_TYPE_OPER(c)                                        \
        inline txIn& operator>> (txIn& in, c& self)                           \
        {                                                                     \
                in.destream(self);                                            \
                return in;                                                    \
        }                                                                     \
                                                                              \
        inline txIn& operator>> (txIn& in, c*& self)                          \
        {                                                                     \
                self = (c*) in.destream();                                    \
                return in;                                                    \
        }

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_DEFINE_TYPE(c)                                                     \
        TX_PERSIST                                                            \
                                                                              \
        public:                                                               \
                                                                              \
        static const txString Type;                                           \
                                                                              \
        virtual const char* className (void) const                            \
        {                                                                     \
               return #c;                                                     \
        }                                                                     \
                                                                              \
        virtual const txString& classType (void) const                        \
        {                                                                     \
               return Type;                                                   \
        }

////////////////////////////////// USER MACRO /////////////////////////////////
#define TX_DEFINE_PARENT_TYPE(c,p)                                            \
        TX_DEFINE_PARENT_TYPE_WITH_ALIAS(c,p,c)

#define TX_DEFINE_PARENT_TYPE_WITH_ALIAS(c,p,alias)                           \
        const txString c::Type(#alias);                                       \
                                                                              \
        class txTypeBinder##c##p                                              \
        {                                                                     \
                public:                                                       \
                        txTypeBinder##c##p (void)                             \
                        {                                                     \
                                txTypeCheckSS::addRelation(#alias, #p);       \
                        }                                                     \
        };                                                                    \
                                                                              \
        static txTypeBinder##c##p typeBinder##c##p;

#endif // __TXUTIL_H__
