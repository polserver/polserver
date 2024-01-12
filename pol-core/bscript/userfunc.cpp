/** @file
 *
 * @par History
 */


#include "userfunc.h"

#include <stddef.h>

#include "../clib/logfacility.h"
#include <format/format.h>

namespace Pol
{
namespace Bscript
{
unsigned int UserFunction::_instances;
std::set<UserFunction*> UserFunction::_instancelist;
void UserFunction::show_instances()
{
  std::string tmp;
  for ( const auto& uf : _instancelist )
  {
    tmp += uf->name + "\n";
  }
  ERROR_PRINT( tmp );
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
UserFunction::UserFunction()
    : name( "" ),
      parameters(),
      position( 0 ),
      forward_callers(),
      ctx(),
      function_body( nullptr ),
      exported( false ),
      emitted( false )
{
}

}  // namespace Bscript
}  // namespace Pol
