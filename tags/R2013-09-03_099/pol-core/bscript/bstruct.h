/*
History
=======
2009/09/05 Turley:    Added struct .? and .- as shortcut for .exists() and .erase()
2009/12/21 Turley:    ._method() call fix

Notes
=======

*/

#ifndef BSCRIPT_BSTRUCT_H
#define BSCRIPT_BSTRUCT_H

#include "bobject.h"
#include "../clib/maputil.h"

class BStruct : public BObjectImp
{
public:
    BStruct();
    BStruct( std::istream& is, unsigned size, BObjectType type );
    static BObjectImp* unpack( std::istream& is );

    void addMember( const char* name, BObjectRef val );
    void addMember( const char* name, BObjectImp* imp );

    const BObjectImp* FindMember( const char* name );

    size_t mapcount() const;

    typedef map<std::string,BObjectRef, ci_cmp_pred> Contents;
	const Contents& contents() const;

protected:
    explicit BStruct( const BStruct& other, BObjectType type );
    explicit BStruct( BObjectType type );

    virtual BObjectImp* copy() const;

    virtual char packtype() const;
    virtual const char* typetag() const;
    virtual void FormatForStringRep( std::ostream& os, const std::string& key, const BObjectRef& bvalref ) const;

    virtual std::string getStringRep() const;
    virtual size_t sizeEstimate() const; 
    virtual void packonto( std::ostream& os ) const;
    virtual const char* typeOf() const;
	virtual int typeOfInt() const;

    virtual ContIterator* createIterator( BObject* pIterVal );

    virtual BObjectRef OperSubscript( const BObject& obj );
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
    virtual BObjectRef set_member( const char* membername, BObjectImp* value, bool copy );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef operDotPlus( const char* name );
	virtual BObjectRef operDotMinus( const char* name );
	virtual BObjectRef operDotQMark( const char* name );
    virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy );

    friend class BStructIterator;

private:
    Contents contents_;
    BStruct& operator=( const BStruct& ); // not implemented
};

#endif
