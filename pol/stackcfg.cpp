/*
History
=======


Notes
=======

*/

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include "../clib/stl_inc.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"

#include "../plib/pkg.h"
#include "stackcfg.h"

PropSet Global_Ignore_CProps;

void read_stacking_cfg(ConfigFile& cf)
{
	ConfigElem elem;
	if( cf.read( elem ) )
	{
		if (elem.type_is( "Stacking" ))
		{
			string temp = elem.remove_string( "IgnoreCprops" );
			ISTRINGSTREAM is( temp );
			string cprop_name;
			while (is >> cprop_name)
				Global_Ignore_CProps.insert(cprop_name);
		}
	}
}

void load_stacking_cfg()
{
	string main_cfg = "config/stacking.cfg";
	
	if (FileExists( main_cfg.c_str() ) )
	{
		ConfigFile cf_main( main_cfg.c_str() );
		read_stacking_cfg(cf_main);
	}
	for( Packages::iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		//string filename = pkg->dir() + cfgname + ".cfg";
		string filename = GetPackageCfgPath(pkg, "stacking.cfg");
		if (FileExists( filename.c_str() ) )
		{
			ConfigFile cf( filename.c_str() );
			read_stacking_cfg(cf);
		}
	}
}

