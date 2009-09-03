/*
History
=======

Notes
=======

*/

#ifndef BSCRIPT_BSTRUCT_H
#define BSCRIPT_BSTRUCT_H

#include "bobject.h"
#include "clib/maputil.h"

class BStruct : public BObjectImp
{
public:
    BStruct();
    BStruct( istream& is, unsigned size, BObjectType type );
    static BObjectImp* unpack( istream& is );

    void addMember( const char* name, BObjectRef val );
    void addMember( const char* name, BObjectImp* imp );

    const BObjectImp* FindMember( const char* name );

    unsigned mapcount() const;

    typedef map<std::string,BObjectRef, ci_cmp_pred> Contents;
	const Contents& contents() const;

protected:
    explicit BStruct( const BStruct& other, BObjectType type );
    explicit BStruct( BObjectType type );

    virtual BObjectImp* copy() const;

    virtual char packtype() const;
    virtual const char* typetag() const;
    virtual void FormatForStringRep( ostream& os, const string& key, const BObjectRef& bvalref ) const;

    virtual string getStringRep() const;
    virtual unsigned long sizeEstimate() const; 
    virtual void packonto( ostream& os ) const;
    virtual const char* typeOf() const;

    virtual ContIterator* createIterator( BObject* pIterVal );

    virtual BObjectRef OperSubscript( const BObject& obj );
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex );
    virtual BObjectRef set_member( const char* membername, BObjectImp* value );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef operDotPlus( const char* name );
    virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target );

    friend class BStructIterator;

private:
    Contents contents_;
    BStruct& operator=( const BStruct& ); // not implemented
};

#endif
