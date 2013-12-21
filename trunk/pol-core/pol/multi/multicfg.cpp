/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#include "../../clib/stl_inc.h"
#include "../../clib/cfgfile.h"
#include "../../clib/fileutil.h"

#include "../../plib/pkg.h"

#include "../cfgrepos.h"
namespace Pol {
  namespace Multi {
	void load_special_storedconfig( string cfgname )
	{
	  string main_cfg = "config/" + cfgname + ".cfg";

	  Core::CreateEmptyStoredConfigFile( main_cfg );
	  ref_ptr<Core::StoredConfigFile> scfg = Core::FindConfigFile( main_cfg, "" );
      if ( Clib::FileExists( main_cfg.c_str( ) ) )
	  {
        Clib::ConfigFile cf_main( main_cfg.c_str( ) );
		scfg->load( cf_main );
	  }

      for ( Plib::Packages::iterator itr = Plib::packages.begin( ); itr != Plib::packages.end( ); ++itr )
	  {
        Plib::Package* pkg = ( *itr );
		//string filename = pkg->dir() + cfgname + ".cfg";
        string filename = Plib::GetPackageCfgPath( pkg, cfgname + ".cfg" );
        if ( Clib::FileExists( filename.c_str( ) ) )
		{
          Clib::ConfigFile cf( filename.c_str( ) );
		  scfg->load( cf );
		}
	  }
	}
  }
}