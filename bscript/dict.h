/*
History
=======

Notes
=======

*/

#ifndef BSCRIPT_DICT_H
#define BSCRIPT_DICT_H

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

class BDictionary : public BObjectImp
{
public:
    BDictionary();

    static BObjectImp* unpack( istream& is );

    void addMember( const char* name, BObjectRef val );
    void addMember( const char* name, BObjectImp* imp );
    void addMember( BObjectImp* key, BObjectImp* val );
    unsigned mapcount() const;

    typedef map<BObject,BObjectRef> Contents;
	const Contents& contents() const;

protected:
    BDictionary( istream& is, unsigned size, BObjectType type = OTDictionary );
    BDictionary( const BDictionary&, BObjectType type = OTDictionary );

    virtual BObjectImp* copy() const;
    virtual string getStringRep() const;
    virtual unsigned long sizeEstimate() const; 
    virtual void packonto( ostream& os ) const;
    virtual const char* typeOf() const;

    virtual ContIterator* createIterator( BObject* pIterVal );

    virtual char packtype() const;
    virtual const char* typetag() const;
    virtual void FormatForStringRep( ostream& os, const BObject& bkeyobj, const BObjectRef& bvalref ) const;

    virtual BObjectRef OperSubscript( const BObject& obj );
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex );
    virtual BObjectRef set_member( const char* membername, BObjectImp* value );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef operDotPlus( const char* name );
    virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target );

    friend class BDictionaryIterator;

protected:
    explicit BDictionary( BObjectType type );

private:
    Contents contents_;

    // not implemented:
    BDictionary& operator=( const BDictionary& );
};

#endif
