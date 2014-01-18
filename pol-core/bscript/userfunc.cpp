/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/logfacility.h"

#include "userfunc.h"

namespace Pol {
  namespace Bscript {
	unsigned int UserFunction::_instances;
	set<UserFunction*> UserFunction::_instancelist;
	void UserFunction::show_instances()
	{
      fmt::Writer tmp;
	  for ( auto uf : _instancelist )
	  {
        tmp << uf->name << "\n";
	  }
      ERROR_PRINT << tmp.c_str();
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
	unsigned int UserFunction::instances()
	{
	  return _instances;
	}
	UserFunction::UserFunction() :
	  name( "" ),
	  parameters(),
	  position( 0 ),
	  forward_callers(),
	  ctx(),
	  function_body( NULL ),
	  exported( false ),
	  emitted( false )
	{}

	UserFunction::~UserFunction()
	{}
  }
}