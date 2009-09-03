/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"
#include "userfunc.h"

unsigned long UserFunction::_instances;
set<UserFunction*> UserFunction::_instancelist;
void UserFunction::show_instances()
{
    for( std::set<UserFunction*>::const_iterator citr = _instancelist.begin(); citr != _instancelist.end(); ++citr )
    {
        UserFunction* uf = (*citr);
        cerr << uf->name << endl;
    }
}
void UserFunction::register_instance()
{
    ++_instances;
    _instancelist.insert( this );
}
void UserFunction::unregister_instance()
{
    _instancelist.erase( this );
    --_instances;
}
unsigned long UserFunction::instances()
{
    return _instances;
}
UserFunction::UserFunction() :
    name(""),
    parameters(),
    position(0),
    forward_callers(),
    ctx(),
    function_body(NULL),
    exported(false),
    emitted(false)
{
}

UserFunction::~UserFunction()
{
}
