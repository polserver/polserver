/*
History
=======
2009/12/21 Turley:    ._method() call fix


Notes
=======

*/

#ifndef EXSCROBJ_H
#define EXSCROBJ_H

#include "../bscript/bobject.h"
#include "../clib/weakptr.h"

class UOExecutor;

extern BApplicObjType scriptexobjimp_type;
typedef weak_ptr<UOExecutor> ScriptExPtr;
class ScriptExObjImp : public BApplicObj< ScriptExPtr >
{
    typedef BApplicObj< ScriptExPtr > base;
public:
    explicit ScriptExObjImp( UOExecutor* uoexec );
    virtual const char* typeOf() const;
    virtual BObjectImp* copy() const;
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin=false );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id );
};

#endif
