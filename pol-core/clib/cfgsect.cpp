/** @file
 *
 * @par History
 */


#include "cfgsect.h"

#include "cfgelem.h"
#include "cfgfile.h"

#include <stdexcept>

namespace Pol
{
namespace Clib
{
ConfigSection::ConfigSection( ConfigFile& cf, const std::string& sectname, unsigned flags )
    : _cf( cf ), _sectname( sectname ), _found( false ), _flags( flags )
{
  _cf.allowed_types_.insert( sectname.c_str() );
}

ConfigSection::~ConfigSection() noexcept( false )
{
  if ( !_found && ( _flags & CST_MANDATORY ) )
  {
    _cf.display_error( "Mandatory section type '" + _sectname + "' not found", false );
    throw std::runtime_error( "Mandatory section type '" + _sectname +
                              "' not found in configuration file \"" + _cf.filename() + "\"" );
  }
}

bool ConfigSection::matches( const ConfigElem& elem )
{
  if ( elem.type_is( _sectname.c_str() ) )
  {
    if ( _found && ( _flags & CST_UNIQUE ) )
    {
      elem.throw_error( "Section type " + _sectname + " found more than once" );
    }
    _found = true;
    return true;
  }
  else
  {
    return false;
  }
}
}  // namespace Clib
}  // namespace Pol
