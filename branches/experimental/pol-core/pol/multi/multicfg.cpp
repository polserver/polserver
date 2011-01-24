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

void load_special_storedconfig( string cfgname )
{
	string main_cfg = "config/" + cfgname + ".cfg";
	
	CreateEmptyStoredConfigFile(main_cfg.c_str());
	ref_ptr<StoredConfigFile> scfg = FindConfigFile(main_cfg, "");
	if ( FileExists(main_cfg.c_str()) )
	{
		ConfigFile cf_main(main_cfg.c_str());
		scfg->load(cf_main);
	}

	for( Packages::iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		//string filename = pkg->dir() + cfgname + ".cfg";
		string filename = GetPackageCfgPath(pkg, cfgname+".cfg");
		if ( FileExists(filename.c_str()) )
		{
			ConfigFile cf(filename.c_str());
			scfg->load(cf);
		}
	}
}
