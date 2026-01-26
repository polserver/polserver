/** @file
 *
 * @par History
 */


#include "stackcfg.h"

#include <string>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include "globals/uvars.h"


namespace Pol::Core
{
void read_stacking_cfg( Clib::ConfigFile& cf )
{
  Clib::ConfigElem elem;
  if ( cf.read( elem ) )
  {
    if ( elem.type_is( "Stacking" ) )
    {
      std::string temp = elem.remove_string( "IgnoreCprops" );
      ISTRINGSTREAM is( temp );
      std::string cprop_name;
      while ( is >> cprop_name )
        gamestate.Global_Ignore_CProps.insert( cprop_name );
    }
  }
}

void load_stacking_cfg()
{
  std::string main_cfg = "config/stacking.cfg";

  if ( Clib::FileExists( main_cfg.c_str() ) )
  {
    Clib::ConfigFile cf_main( main_cfg.c_str() );
    read_stacking_cfg( cf_main );
  }
  for ( auto pkg : Plib::systemstate.packages )
  {
    // string filename = pkg->dir() + cfgname + ".cfg";
    std::string filename = Plib::GetPackageCfgPath( pkg, "stacking.cfg" );
    if ( Clib::FileExists( filename.c_str() ) )
    {
      Clib::ConfigFile cf( filename.c_str() );
      read_stacking_cfg( cf );
    }
  }
}
}  // namespace Pol::Core
