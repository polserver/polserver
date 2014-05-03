/*
History
=======


Notes
=======

*/

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif
#include <sstream>
#include "../clib/stl_inc.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"

#include "../plib/pkg.h"
#include "stackcfg.h"
namespace Pol {
  namespace Core {
	PropSet Global_Ignore_CProps;

	void read_stacking_cfg( Clib::ConfigFile& cf )
	{
      Clib::ConfigElem elem;
	  if ( cf.read( elem ) )
	  {
		if ( elem.type_is( "Stacking" ) )
		{
		  string temp = elem.remove_string( "IgnoreCprops" );
		  ISTRINGSTREAM is( temp );
		  string cprop_name;
		  while ( is >> cprop_name )
			Global_Ignore_CProps.insert( cprop_name );
		}
	  }
	}

	void load_stacking_cfg()
	{
	  string main_cfg = "config/stacking.cfg";

      if ( Clib::FileExists( main_cfg.c_str( ) ) )
	  {
        Clib::ConfigFile cf_main( main_cfg.c_str( ) );
		read_stacking_cfg( cf_main );
	  }
      for ( Plib::Packages::iterator itr = Plib::packages.begin( ); itr != Plib::packages.end( ); ++itr )
	  {
        Plib::Package* pkg = ( *itr );
		//string filename = pkg->dir() + cfgname + ".cfg";
        string filename = Plib::GetPackageCfgPath( pkg, "stacking.cfg" );
        if ( Clib::FileExists( filename.c_str( ) ) )
		{
          Clib::ConfigFile cf( filename.c_str( ) );
		  read_stacking_cfg( cf );
		}
	  }
	}
  }
}
