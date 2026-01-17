#include "polobject.h"
#include "uoexec.h"

namespace Pol::Core
{
using namespace Bscript;
BObjectImp* PolObjectImp::call_method( const char* methodname, Executor& ex )
{
  passert( ex.type() == ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( ex );
  return this->call_polmethod( methodname, uoex );
}

BObjectImp* PolObjectImp::call_method_id( const int id, Executor& ex, bool forcebuiltin )
{
  passert( ex.type() == ExecutorType::POL );
  auto& uoex = static_cast<Core::UOExecutor&>( ex );
  return this->call_polmethod_id( id, uoex, forcebuiltin );
}

BObjectImp* PolObjectImp::call_polmethod( const char* methodname, Core::UOExecutor& uoex )
{
  return BObjectImp::call_method( methodname, uoex );
}

BObjectImp* PolObjectImp::call_polmethod_id( const int id, Core::UOExecutor& uoex,
                                             bool forcebuiltin )
{
  return BObjectImp::call_method_id( id, uoex, forcebuiltin );
}
}  // namespace Pol::Core
