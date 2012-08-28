/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef WIN32
#	pragma warning(disable:4786)
#endif

#include "../clib/xmain.h"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <memory>
#include "../clib/clib.h"
#include "../clib/dirlist.h"
#include "../clib/fileutil.h"
#include "../clib/mdump.h"
#include "../clib/progver.h"
#include "../clib/wallclock.h"

#include "../plib/pkg.h"

#include "../bscript/compilercfg.h"
#include "../bscript/filefmt.h"
#include "../bscript/userfunc.h"
#include "../bscript/compiler.h"
#include "../bscript/escriptv.h"
#include "../bscript/executor.h"
#include "../bscript/userfunc.h"

ExecInstrFunc Executor::GetInstrFunc( const Token& token )
{
    return NULL;
}
#if REFPTR_DEBUG
unsigned int ref_counted::_ctor_calls;
#endif

int debug = 0;
bool quiet = false;
bool opt_generate_wordlist = false;
bool keep_building = false;
bool verbose = false;
bool force_update = false;
bool show_timing_details = false;
bool timing_quiet_override = false;
bool expect_compile_failure = false;
bool dont_optimize_object_members = false;
std::string EmPathEnv; // "ECOMPILE_PATH_EM=xxx"
std::string IncPathEnv; // ECOMPILE_PATH_INC=yyy"
std::string CfgPathEnv; // ECOMPILE_CFG_PATH=zzz"

struct Summary
{
    unsigned UpToDateScripts;
    unsigned CompiledScripts;
    unsigned ScriptsWithCompileErrors;
} summary;

void usage(void)
{
    cerr << " Usage:" << endl;
    cerr << "   " << progverstr << " [options] filespec [filespec ...]" << endl;
    cerr << "   Output is : filespec.ecl" << endl;
    cerr << "   Options: " << endl;
    cerr << "       -a           compile *.asp pages also" << endl;
    cerr << "       -A           automatically compile scripts in main and enabled packages" << endl;
    cerr << "       -Au          (as '-A' but only compile updated files)" << endl;
    cerr << "       -b           keep building other scripts after errors" << endl;
    cerr << "       -C cfgpath   path to configuration (replaces ecompile.cfg)" << endl;
    cerr << "       -d           display confusing compiler parse information" << endl;
    cerr << "       -D           write dependency information" << endl;
    cerr << "       -e           report error on successful compilation (used for testing)" << endl;
#ifdef WIN32
    cerr << "       -Ecfgpath    set or change the ECOMPILE_CFG_PATH Evironment Variable" << endl;
#endif
    cerr << "       -i           include intrusive debug info in .ecl file" << endl;
    cerr << "       -l           generate listfile" << endl;
    cerr << "       -m           don't optimize object members" << endl;
#ifdef WIN32
    cerr << "       -Pdir        set or change the EM and INC files Environment Variables" << endl;
#endif
	cerr << "       -q           quiet mode (suppress normal output)" << endl;
    cerr << "       -r [dir]     recurse folder [from 'dir'] (defaults to current folder)" << endl;
	cerr << "       -ri [dir]      (as '-r' but only compile .inc files)" << endl;
	cerr << "         -t[v]      show timing/profiling information [override quiet mode]" << endl;
    cerr << "         -u         compile only updated scripts (.src newer than .ecl)" << endl;
    cerr << "           -f       force compile even if up-to-date" << endl;
    cerr << "       -s           display summary if -q is not set" << endl;
    cerr << "       -vN          verbosity level" << endl;
    cerr << "       -w           display warnings" << endl;
    cerr << "       -W           generate wordfile" << endl;
    cerr << "       -y           treat warnings as errors" << endl;
    cerr << "       -x           write external .dbg file" << endl;
    cerr << "       -xt          write external .dbg.txt info file" << endl;
	cerr << endl;
	cerr << " NOTE:" << endl;
	cerr << "   If <filespec> are required after an empty -r[i] option, you MUST specify" << endl;
	cerr << "   a literal [dir] of '.' (no quotes) or options will not parse correctly." << endl;
    exit(1);
}

void generate_wordlist()
{
    cout << "Writing word list to wordlist.txt" << endl;
    ofstream ofs( "wordlist.txt", ios::out|ios::trunc );
    Parser::write_words( ofs );
}

void compile_inc( const char* path )
{
    if (!quiet)
        cout << "Compiling: " << path << endl;

    Compiler C(cout);

    C.setQuiet(!debug);
    C.setIncludeCompileMode();
    int res = C.compileFile(path);
    
    if (res) 
        throw runtime_error( "Error compiling file" );
}

bool compile_file(const char *path)
{
        string fname(path);
        string filename_src = fname, ext("");

	    string::size_type pos = fname.rfind(".");
	    if ( pos != string::npos )
		    ext = fname.substr(pos);

        if (!ext.compare(".inc"))
        {
            compile_inc( path );
            return true;
        }

        if (ext.compare(".src") != 0 &&
		    ext.compare(".hsr") != 0 &&
		    ext.compare(".asp") != 0)
        {
            cout << "Didn't find '.src', '.hsr', or '.asp' extension on source filename '"
                << path << "'!" << endl;
            throw runtime_error( "Error in source filename" );
        }
        string filename_ecl = fname.replace(pos, 4, ".ecl");
        string filename_lst = fname.replace(pos, 4, ".lst");
        string filename_dep = fname.replace(pos, 4, ".dep");
        string filename_dbg = fname.replace(pos, 4, ".dbg");

        if (compilercfg.OnlyCompileUpdatedScripts && !force_update)
        {
            bool all_old = true;
            unsigned int ecl_timestamp = GetFileTimestamp( filename_ecl.c_str() );
            if (GetFileTimestamp( filename_src.c_str() ) >= ecl_timestamp)
            {
                if (verbose)
                    cout << filename_src << " is newer than " << filename_ecl << endl;
                all_old = false;
            }

            if (all_old)
            {
                ifstream ifs( filename_dep.c_str() );
                // if the file doesn't exist, gotta build.
                if (ifs.is_open())
                {
                    string depname;
                    while (getline( ifs, depname ))
                    {
                        if (GetFileTimestamp( depname.c_str() ) >= ecl_timestamp )
                        {
                            if (verbose)
                                cout << depname << " is newer than " << filename_ecl << endl;
                            all_old = false;
                            break;
                        }
                    }
                }
                else
                {
                    if (verbose)
                        cout << filename_dep << " does not exist." << endl;
                    all_old = false;
                }
            }
            if (all_old)
            {
			    if (!quiet && compilercfg.DisplayUpToDateScripts)
				    cout << filename_ecl << " is up-to-date." << endl;
                return false;
            }

        }

        {
            if (!quiet)
                cout << "Compiling: " << path << endl;

            Compiler C(cout);

            C.setQuiet(!debug);
            int res = C.compileFile(path);
        
            if (expect_compile_failure)
            {
                if (res)        // good, it failed
                {
                    if (!quiet)
                        cout << "Compilation failed as expected." << endl;
                    return true;
                }
                else
                {
                    throw runtime_error( "Compilation succeeded (-e indicates failure was expected)" );
                }
            }

            if (res) 
                throw runtime_error( "Error compiling file" );
        


            if (!quiet)
                cout << "Writing:   " << filename_ecl << endl;

            if (C.write(filename_ecl.c_str()))
            {
                throw runtime_error( "Error writing output file" );
            }

            if (compilercfg.GenerateListing)
            {
                if (!quiet)
                    cout << "Writing:   " << filename_lst << endl;
                ofstream ofs( filename_lst.c_str() );
                C.dump( ofs );
            }
            else if (FileExists( filename_lst.c_str() ) )
            {
                if (!quiet) cout << "Deleting:  " << filename_lst << endl;
                RemoveFile( filename_lst );
            }

            if (compilercfg.GenerateDebugInfo)
            {
                if (!quiet)
                {
                    cout << "Writing:   " << filename_dbg << endl;
                    if (compilercfg.GenerateDebugTextInfo)
                        cout << "Writing:   " << filename_dbg << ".txt" << endl;
                }
                C.write_dbg( filename_dbg.c_str(), compilercfg.GenerateDebugTextInfo );
            }
            else if (FileExists( filename_dbg.c_str() ) )
            {
                if (!quiet) cout << "Deleting:  " << filename_dbg << endl;
                RemoveFile( filename_dbg );
            }

            if (compilercfg.GenerateDependencyInfo)
            {
                if (!quiet)
                    cout << "Writing:   " << filename_dep << endl;
                C.writeIncludedFilenames( filename_dep.c_str() );
            }
            else if (FileExists( filename_dep.c_str() ) )
            {
                if (!quiet) cout << "Deleting:  " << filename_dep << endl;
                RemoveFile( filename_dep );
            }

        }
    return true;
}

void compile_file_wrapper( const char* path )
{
    try
    {
        if (compile_file( path ))
            ++summary.CompiledScripts;
        else
            ++summary.UpToDateScripts;
    }
    catch( std::exception&  )
    {
        ++summary.CompiledScripts;
        ++summary.ScriptsWithCompileErrors;
        if (!keep_building)
            throw;
    }
}

bool setting_value( const char* arg )
{
    // format of arg is -C or -C-
    if (arg[2] == '\0')
        return true;
    else if (arg[2] == '-')
        return false;
    else if (arg[2] == '+')
        return true;
    else
        return true;
}


int readargs(int argc, char **argv)
{
	bool unknown_opt = false;

    for(int i=1;i<argc;i++)
    {
        const char* arg = argv[i];
#ifdef __linux__
        if ( arg[0] == '-' )
#else
        if ( arg[0] == '/' || arg[0] == '-' )
#endif
        {
            switch(arg[1]) 
            {
                case 'A': // skip it at this point.
                    break;

                case 'C': // skip it at this point.
                    ++i; // and skip its parameter.
                    break;

				case 'd':
                    debug = 1;
                    break;

				case 'D':
					compilercfg.GenerateDependencyInfo = true;
                    break;

                case 'e':
                    expect_compile_failure = true;
                    break;

#ifdef WIN32
	            case 'E':
						{
							std::string path = &argv[i][2];
							CfgPathEnv = "ECOMPILE_CFG_PATH=" + path;
							_putenv( CfgPathEnv.c_str() );
						}
						break;
#endif

				case 'q':
                    quiet = true;
                    break;

                case 'w':
                    compilercfg.DisplayWarnings = setting_value( arg );
                    break;
                case 'y':
                    compilercfg.ErrorOnWarning = setting_value( arg );
                    break;

                case 'l':
                    compilercfg.GenerateListing = setting_value( arg );
                    break;

                case 'i':
                    include_debug = 1;
                    break;

                case 'r': // -r[i] [dir] is okay
					// Only suboption allowed is '-ri' (.inc recurse)
					if ( argv[i][2] && argv[i][2] != 'i' )
					{
						// BZZZZT! *error*
						unknown_opt = true;
						break;
					}

					// Only skip next parameter if it's not an option!!
#ifdef __linux__
					if ( i+1<argc && argv[i+1][0] != '-' )
#else
					if ( i+1<argc && argv[i+1][0] != '/' && argv[i+1][0] != '-' )
#endif					
						++i;
                    break;

				case 't':
					show_timing_details = true;
					if ( argv[i][2] == 'v' )
						timing_quiet_override = true;
					break;

                case 's':
                    // show_source = true;
                    compilercfg.DisplaySummary = true;
					break;

                case 'W':
                    opt_generate_wordlist = true;
                    break;

                case 'a':
                    compilercfg.CompileAspPages = setting_value( arg );
                    break;

                case 'm':
                    compilercfg.OptimizeObjectMembers = false;
                    break;

                case 'b':
                    keep_building = true;
                    break;

                case 'u':
                    compilercfg.OnlyCompileUpdatedScripts = setting_value( arg );
                    if (compilercfg.OnlyCompileUpdatedScripts)
                        compilercfg.GenerateDependencyInfo = true;
                    break;

                case 'f':
                    force_update = true;
                    break;

                case 'v':
                    verbose = true;
                    int vlev;
                    vlev = atoi( &argv[i][2] );
                    if (!vlev)
                        vlev = 1;
                    Compiler::setVerbosityLevel( vlev );
                    break;

                case 'x':
                    compilercfg.GenerateDebugInfo = true;
                    compilercfg.GenerateDebugTextInfo = (argv[i][2] == 't');
                    break;
                        
#ifdef WIN32
                case 'P':
                    {
                        std::string path = &argv[i][2];
                        EmPathEnv = "ECOMPILE_PATH_EM=" + path;
                        IncPathEnv = "ECOMPILE_PATH_INC=" + path;
                        _putenv( EmPathEnv.c_str() );
                        _putenv( IncPathEnv.c_str() );
                    }
                    break;
#endif

                default:
					unknown_opt = true;
					break;
			}
		}

		if ( unknown_opt )
		{
			cerr << "Unknown option: " << argv[i] << endl;
			usage();
			return 1;
		}
    }
    return 0;
}

void recurse_compile( string basedir )
{
	int s_compiled, s_uptodate, s_errors;
	clock_t start, finish;

	if ( !IsDirectory( basedir.c_str() ) )
		return;

	s_compiled = s_uptodate = s_errors = 0;
	start = clock();
    for( DirList dl( basedir.c_str() ); !dl.at_end(); dl.next() )
    {
		string name = dl.name(), ext;
        if (name[0] == '.') continue;

		string::size_type pos = name.rfind(".");
		if (pos != string::npos)
			ext = name.substr(pos);

        try 
        {
			if ( pos != string::npos &&
				(!ext.compare(".src") ||
				 !ext.compare(".hsr") ||
				 (compilercfg.CompileAspPages && !ext.compare(".asp"))) )
			{
				s_compiled++;
				if (compile_file( (basedir + name).c_str() ))
                {
                    ++summary.CompiledScripts;
                }
                else
                {
                    ++s_uptodate;
                    ++summary.UpToDateScripts;
                }
			}
            else
            {
                recurse_compile( basedir + name + "/" );
            }
        }
        catch( std::exception& )
        {
            ++summary.CompiledScripts;
            ++summary.ScriptsWithCompileErrors;
            if (!keep_building)
                throw;
			s_errors++;
        }
    }
	finish = clock();

	if ( (!quiet || timing_quiet_override) && show_timing_details && s_compiled > 0 ) {
		cout << "Compiled " << s_compiled << " script" << (s_compiled==1?"":"s")
			 << " in " << basedir
			 << " in " << (int)((finish-start)/CLOCKS_PER_SEC) << " second(s)" << endl;
		if ( s_uptodate > 0 )
			cout << "    " << s_uptodate << " script" << (s_uptodate==1?" was":"s were")
				 << " already up-to-date." << endl;
		if ( s_errors > 0 )
			cout << "    " << s_errors << " script" << (s_errors==1?"":"s")
				 << " had errors." << endl;
	}
}
void recurse_compile_inc( string basedir )
{
    for( DirList dl( basedir.c_str() ); !dl.at_end(); dl.next() )
    {
        string name = dl.name(), ext;
        if (name[0] == '.') continue;

		string::size_type pos = name.rfind(".");
		if (pos != string::npos)
			ext = name.substr(pos);

		if (pos != string::npos && !ext.compare(".inc"))
        {
            compile_file( (basedir + name).c_str() );
        }
        else
        {
            recurse_compile( basedir + name + "/" );
        }
    }
}

void AutoCompile()
{
    bool save = compilercfg.OnlyCompileUpdatedScripts;
    compilercfg.OnlyCompileUpdatedScripts = compilercfg.UpdateOnlyOnAutoCompile;

    recurse_compile( normalized_dir_form( compilercfg.PolScriptRoot ) );
    for( size_t i = 0; i < packages.size(); ++i )
    {
        const Package* pkg = packages[i];
        recurse_compile( normalized_dir_form( pkg->dir() ) );
    }
    compilercfg.OnlyCompileUpdatedScripts = save;
}

bool run(int argc, char **argv)
{
    for( size_t pi = 0; pi < compilercfg.PackageRoot.size(); ++pi )
    {
        load_packages( compilercfg.PackageRoot[pi], true /* quiet */ );
    }
    replace_packages();
    check_package_deps();

	wallclock_t start = wallclock();
    bool any = false;

	for(int i=1;i<argc;i++)
	{
#ifdef __linux__	
		if (argv[i][0] == '-')
#else
		if (argv[i][0] == '/' || argv[i][0] == '-')
#endif
		{
			// -r[i] [<dir>]
            if (argv[i][1] == 'A')
            {
                compilercfg.UpdateOnlyOnAutoCompile = (argv[i][2] == 'u');
				any = true;

                AutoCompile();
            }
			else if (argv[i][1] == 'r')
			{
                any = true;
				string dir(".");
				bool compile_inc = (argv[i][2] == 'i'); // compile .inc files

				++i;
				if (i<argc && argv[i] && argv[i][0] != '-')
					dir.assign(argv[i]);

				if (compile_inc)
					recurse_compile_inc( normalized_dir_form( dir ) );
				else
					recurse_compile( normalized_dir_form( dir ) );
			}
            else if (argv[i][1] == 'C')
            {
                ++i; // skip the config file pathname
            }
            // and skip any other option.
		}
		else
		{
            any = true;
#ifdef _WIN32
            forspec(argv[i], compile_file_wrapper);
#else
            compile_file_wrapper( argv[i] );
#endif
        }
    }

    if (!any && compilercfg.AutoCompileByDefault)
    {
        any = true;
        AutoCompile();
    }

    wallclock_t finish = wallclock();

    unload_packages();

    if (any && compilercfg.DisplaySummary && !quiet)
    {
        cout << "Compilation Summary:" << endl;
		if (summary.CompiledScripts)
            cout << "    Compiled " << summary.CompiledScripts << " script" << (summary.CompiledScripts==1?"":"s")
			     << " in " << wallclock_diff_ms( start, finish ) << " ms." << endl;
        
        if (summary.ScriptsWithCompileErrors)
            cout << "    " << summary.ScriptsWithCompileErrors << " of those script" << (summary.ScriptsWithCompileErrors==1?"":"s")
                 << " had errors." << endl;

        if (summary.UpToDateScripts)
            cout << "    " << summary.UpToDateScripts << " script" << (summary.UpToDateScripts==1?" was":"s were")
                 << " already up-to-date." << endl;

    }

    return any;
}

void read_config_file( int argc, char* argv[])
{
	for(int i=1;i<argc;i++)
	{
		if (argv[i][0] == '/' || argv[i][0] == '-')
		{
			// -C cfgpath
			if (argv[i][1] == 'C')
			{
                ++i;
                if (i == argc)
                    throw runtime_error( "-C specified without pathname" );

                compilercfg.Read( string(argv[i]) );
                return;
			}
		}
    }

    // check ECOMPILE_CFG_PATH environment variable
    const char* env_ecompile_cfg_path = getenv( "ECOMPILE_CFG_PATH" );
    if (env_ecompile_cfg_path != NULL)
    {
        compilercfg.Read( string(env_ecompile_cfg_path) );
        return;
    }

    // no -C arg, so use binary path (hope it's right..sigh.)
    string cfgpath = xmain_exedir + "ecompile.cfg";
    if (FileExists( "ecompile.cfg" ))
    {
        compilercfg.Read( "ecompile.cfg" );
    }
    else if (FileExists( cfgpath ))
    {
        compilercfg.Read( cfgpath );
    }
    else
    {
		cerr << "Could not find " << cfgpath << "; using defaults." << endl;
        compilercfg.SetDefaults();
    }
}

int xmain(int argc, char *argv[])
{
    strcpy( progverstr, "ECOMPILE" );
    progver = 1;

#ifdef _WIN32
    MiniDumper::Initialize();
#endif

    read_config_file( argc,argv );

    int res = readargs(argc, argv);
    if (res)
        return res;

    if (!quiet)
    {
		// vX.YY
		double vernum = (double)progver + (double)(ESCRIPT_FILE_VER_CURRENT / 100.0f);
		cerr << "EScript Compiler v" << vernum << endl;
        cerr << "Copyright (C) 1993-2012 Eric N. Swanson" << endl;
        cerr << endl;
    }

    if (opt_generate_wordlist)
    {
        generate_wordlist();
        return 0;
    }
    
    bool didanything = run( argc, argv );
    
    if (!didanything)
    {
        usage();
        return 1;
    }
    else
    {
        return 0;
    }
}
