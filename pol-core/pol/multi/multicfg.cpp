/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include <string>

#include "../../clib/cfgfile.h"
#include "../../clib/fileutil.h"
#include "../../clib/refptr.h"
#include "../../plib/pkg.h"
#include "../../plib/systemstate.h"
#include "../cfgrepos.h"


namespace Pol::Multi
{
void load_special_storedconfig( const std::string& cfgname )
{
  std::string main_cfg = "config/" + cfgname + ".cfg";

  Core::CreateEmptyStoredConfigFile( main_cfg );
  ref_ptr<Core::StoredConfigFile> scfg = Core::FindConfigFile( main_cfg, "" );
  if ( Clib::FileExists( main_cfg.c_str() ) )
  {
    Clib::ConfigFile cf_main( main_cfg.c_str() );
    scfg->load( cf_main );
  }

  for ( Plib::Packages::iterator itr = Plib::systemstate.packages.begin();
        itr != Plib::systemstate.packages.end(); ++itr )
  {
    Plib::Package* pkg = ( *itr );
    // string filename = pkg->dir() + cfgname + ".cfg";
    std::string filename = Plib::GetPackageCfgPath( pkg, cfgname + ".cfg" );
    if ( Clib::FileExists( filename.c_str() ) )
    {
      Clib::ConfigFile cf( filename.c_str() );
      scfg->load( cf );
    }
  }
}
}  // namespace Pol::Multi
