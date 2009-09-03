/*
History
=======


Notes
=======

*/

#ifndef ACSCROBJ_H
#define ACSCROBJ_H

#include "reftypes.h"

class AccountPtrHolder
{
public:
    explicit AccountPtrHolder( AccountRef i_acct ) : acct(i_acct) {}
    Account* operator->() { return acct.get(); }
    const Account* operator->() const { return acct.get(); }

    Account* Ptr() { return acct.get(); }
private:
    AccountRef acct;
};

extern BApplicObjType accountobjimp_type;
class AccountObjImp : public BApplicObj< AccountPtrHolder >
{
    typedef BApplicObj< AccountPtrHolder > base;
public:
    explicit AccountObjImp( const AccountPtrHolder& other ) :
      BApplicObj< AccountPtrHolder >(&accountobjimp_type, other)
      {
      }
    virtual const char* typeOf() const;
    virtual BObjectImp* copy() const;
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id ); //id test
};


#endif
