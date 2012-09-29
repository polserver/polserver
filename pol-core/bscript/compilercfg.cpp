/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/cfgfile.h"
#include "../clib/cfgelem.h"
#include "../clib/dirlist.h"
#include "../clib/xmain.h"

#include "compilercfg.h"

void CompilerConfig::Read( const string& path )
{
    ConfigFile cf( path.c_str() );
    ConfigElem elem;
    cf.readraw( elem );

    PackageRoot.clear();
    IncludeDirectory.clear();

    string tmp;
    while (elem.remove_prop( "PackageRoot", &tmp ))
    {
        PackageRoot.push_back( normalized_dir_form(tmp) );
    }
    if (elem.remove_prop( "IncludeDirectory", &tmp ))
    {
        IncludeDirectory = normalized_dir_form(tmp);
    }
    ModuleDirectory = normalized_dir_form(elem.remove_string( "ModuleDirectory" ));
    PolScriptRoot = normalized_dir_form(elem.remove_string( "PolScriptRoot" ));
	GenerateListing = elem.remove_bool( "GenerateListing", false );
	GenerateDebugInfo = elem.remove_bool( "GenerateDebugInfo", false );
	GenerateDebugTextInfo = elem.remove_bool( "GenerateDebugTextInfo", false );
    DisplayWarnings = elem.remove_bool( "DisplayWarnings", false );
    CompileAspPages = elem.remove_bool( "CompileAspPages", false );
    AutoCompileByDefault = elem.remove_bool( "AutoCompileByDefault", false );
    UpdateOnlyOnAutoCompile = elem.remove_bool( "UpdateOnlyOnAutoCompile", false );
    OnlyCompileUpdatedScripts = elem.remove_bool( "OnlyCompileUpdatedScripts", false );
    DisplaySummary = elem.remove_bool( "DisplaySummary", false );
    OptimizeObjectMembers = elem.remove_bool( "OptimizeObjectMembers", true);
    ErrorOnWarning = elem.remove_bool( "ErrorOnWarning", false);
    GenerateDependencyInfo = elem.remove_bool( "GenerateDependencyInfo", OnlyCompileUpdatedScripts );

    DisplayUpToDateScripts = elem.remove_bool( "DisplayUpToDateScripts", true );
	ThreadedCompilation = elem.remove_bool( "ThreadedCompilation", false );

	// This is where we TRY to validate full paths from what was provided in the
	// ecompile.cfg. Maybe Turley or Shini can find the best way to do this in *nix.
#ifdef WIN32
	string MyPath = path.c_str();
	// If it's just "ecompile.cfg", let's change it to the exe's path which it SHOULD be
	// with. 
	if (stricmp(MyPath.c_str(), "ecompile.cfg") == 0)
	{
		// Let's find the NEXT-TO-LAST / in the path, and remove from there on. Oh yay!
		// To bad we can't just force everyone to use ABSOLUTE PATHS NANDO. :o
		MyPath = xmain_exedir.substr(0, xmain_exedir.length()-1);
		MyPath = MyPath.substr(0, MyPath.find_last_of( '/')+1);
	}
	if (IncludeDirectory.find(':') == string::npos)
	{
		if (IncludeDirectory.substr(0, 1) != ".") // Let's make sure they didn't try using this method
		{
			IncludeDirectory = MyPath + IncludeDirectory;
		}
	}
	if (ModuleDirectory.find(':') == string::npos)
	{
		if (ModuleDirectory.substr(0, 1) != ".") // Let's make sure they didn't try using this method
		{
			ModuleDirectory = MyPath + ModuleDirectory;
		}
	}
	if (PolScriptRoot.find(':') == string::npos)
	{
		if (PolScriptRoot.substr(0, 1) != ".") // Let's make sure they didn't try using this method
		{
			PolScriptRoot = MyPath + PolScriptRoot;
		}
	}
	for( unsigned pr = 0; pr < PackageRoot.size(); ++pr )
    {
		if (PackageRoot[pr].find(':') == string::npos)
		{
			if (PackageRoot[pr].substr(0, 1) != ".") // Let's make sure they didn't try using this method
			{
				PackageRoot[pr] = MyPath + PackageRoot[pr];
			}
		}
    }

#endif
}

void CompilerConfig::SetDefaults()
{
    const char* tmp;
    
    tmp = getenv( "ECOMPILE_PATH_EM" );
    ModuleDirectory = tmp ? normalized_dir_form(tmp) : xmain_exedir;
    
    tmp = getenv( "ECOMPILE_PATH_INC" );
    IncludeDirectory = tmp ? normalized_dir_form(tmp) : xmain_exedir;

    PolScriptRoot = IncludeDirectory;

    DisplayUpToDateScripts = true;
}

CompilerConfig compilercfg;
