#include "polobject.h"
#include "uoexec.h"
namespace Pol
{
namespace Core
{
Bscript::BObjectImp* PolObjectImp::call_method( const char* methodname, Bscript::Executor& ex )
{
  auto& uoex = static_cast<Core::UOExecutor&>( ex );
  return this->call_polmethod( methodname, uoex );
}

Bscript::BObjectImp* PolObjectImp::call_method_id( const int id, Bscript::Executor& ex,
                                                   bool forcebuiltin = false )
{
  auto& uoex = static_cast<Core::UOExecutor&>( ex );
  return this->call_polmethod_id( id, uoex, forcebuiltin );
}

}  // namespace Core
}  // namespace Pol
