/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/dirlist.h"
#include "../clib/fileutil.h"
#include "../clib/maputil.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "pkg.h"
#include "polver.h"

bool Package::provides_system_home_page() const
{
	return provides_system_home_page_;
}

typedef std::map<string,Package*,ci_cmp_pred> PackagesByName;
PackagesByName packages_byname;
Package* find_package( const std::string& pkgname )
{
	PackagesByName::iterator itr = packages_byname.find( pkgname );
	if (itr != packages_byname.end())
	{
		return (*itr).second;
	}
	else
	{
		return NULL;
	}
}

void remove_package( Package* pkg )
{
	Packages::iterator last = remove_if(packages.begin(), packages.end(),
										bind2nd(equal_to<Package*>(), pkg) ) ;
	packages.erase( last );
	
	PackagesByName::iterator itr = packages_byname.begin();
	while (itr != packages_byname.end())
	{
		PackagesByName::iterator tempitr = itr;
		++itr;
		if ((*tempitr).second == pkg)
			packages_byname.erase( tempitr );
	}
}

void compare_versions( const std::string& verleft,
					   const std::string& verright, 
					   bool& isgreater,
					   bool& isequal )
{
	isgreater = isequal = false;

	const char* vneed = verright.c_str();
	const char* vhave = verleft.c_str();

	while ((vneed != NULL && vneed[0] != '\0') || 
		   (vhave != NULL && vhave[0] != '\0'))
	{
		char* new_vneed;
		char* new_vhave;
		unsigned int vneedpart, vhavepart;
		vneedpart = (vneed != NULL) ? strtoul( vneed, &new_vneed, 0 ) : 0;
		vhavepart = (vhave != NULL) ? strtoul( vhave, &new_vhave, 0 ) : 0;

		if (vhavepart > vneedpart)
		{
			isgreater= true;
			return;
		}
		else if (vhavepart < vneedpart)
		{
			return;
		}
		else	 // same, so check the next one
		{
			vneed = new_vneed;
			if (vneed && *vneed) ++vneed;
			vhave = new_vhave;
			if (vhave && *vhave) ++vhave;
		}
	}
	isequal = true;
}

bool check_version2( const std::string& version_have, const std::string& version_need )
{
	bool isgreater;
	bool isequal;
	compare_versions( version_have, version_need, isgreater, isequal );
	return (isequal || isgreater);
}

void test_check_version()
{						 // have // need  
	passert( check_version2( "0",	 "0"	  ) == true );
	passert( check_version2( "1",	 "0"	  ) == true );
	passert( check_version2( "0",	 "1"	  ) == false );
	passert( check_version2( "0.5",   "1"	  ) == false );
	passert( check_version2( "0.5",   "0"	  ) == true );
	passert( check_version2( "1.2",   "1.12"   ) == false );
	passert( check_version2( "1.12",  "1.2"	) == true );
	passert( check_version2( "1.2.3", "1"	  ) == true );
	passert( check_version2( "1.1",   "1.2.3"  ) == false );
	passert( check_version2( "1.3",   "1.2.3"  ) == true );
}
Packages packages;


PackageList::PackageList( ConfigElem& elem, const char* tag )
{
	string tmp;
	while (elem.remove_prop( tag, &tmp ))
	{
		mklower( tmp );
		ISTRINGSTREAM is( tmp );
		Elem elem;
		if (is >> elem.pkgname)
		{
			if (! (is >> elem.version))
				elem.version = "0";
			
			elems.push_back( elem );
		}
	}
}

Package::Package( const std::string& pkg_dir, ConfigElem& elem ) :
	dir_( pkg_dir ),
	name_( elem.remove_string( "Name" ) ),
	version_( elem.remove_string( "Version", "0" ) ),
	core_required_( 0 ),
	requires_( elem, "Requires" ),
	conflicts_( elem, "Conflicts" ),
	replaces_( elem, "Replaces" ),
	provides_system_home_page_( elem.remove_bool( "ProvidesSystemHomePage", false ) )
{
	mklower( name_ );
	// CoreRequired can either be a number (94,,95 etc)
	// or a version string (POL095-2003-01-28)
	string tmp = elem.read_string( "CoreRequired", "0" );
	if (isdigit( tmp[0] ))
	{
		// the first kind - a number.
		core_required_ = elem.remove_ushort( "CoreRequired", 0 );
	}
	else
	{
		core_versionstring_required_ = elem.remove_string( "CoreRequired" );
	}
}

string Package::desc() const
{
	return name() + " (" + dir() + ")";
}

bool Package::check_replacements() const
{
	bool any = false;
	for( unsigned i = 0; i < replaces_.elems.size(); ++i )
	{
		const PackageList::Elem& elem = replaces_.elems[i];
		Package* found = find_package( elem.pkgname );
		if (found != NULL)
		{
			any = true;
			cout << "Package " << desc() << " replaces package " << found->desc() << endl;
			remove_package( found );
			delete found;
			found = NULL;
		}
	}
	return any;
}

void Package::check_dependencies() const
{
	if (core_required_)
	{
		if (core_required_ > polver)
		{
			cerr << "Error in package " << desc() << ":" << endl;
			cerr << "  Core version " << core_required_ 
				<< " is required, but version " << polver << " is running." << endl;
			throw runtime_error("Package requires a newer core version");
		}
	}
	else if (!core_versionstring_required_.empty())
	{
		int cmp = stricmp( polverstr, core_versionstring_required_.c_str() );
		if (cmp < 0)
		{
			cerr << "Error in package " << desc() << ":" << endl;
			cerr << "  Core version " << core_versionstring_required_ 
				<< " is required, but version " << polverstr << " is running." << endl;
			throw runtime_error("Package requires a newer core version");
		}
	}
	for( unsigned i = 0; i < requires_.elems.size(); ++i )
	{
		const PackageList::Elem& elem = requires_.elems[i];
		Package* found = find_package( elem.pkgname );
		if (found == NULL)
		{
			cerr << "Error in package '" << name_ << "' (" << dir_ << "):" << endl;
			cerr << "	Package '" << elem.pkgname << "' is required, but is not installed." << endl;
			throw runtime_error("Package dependency error");
		}
		else
		{
			if ( ! check_version2( found->version_, elem.version  ) )
			{
				cerr << "Error in package '" << name_ << "' (" << dir_ << "):" << endl;
				cerr << "	Package '" << elem.pkgname << "' version " << elem.version 
							<< " is required, but version " << found->version_ << " was found"
							<< endl;
				throw runtime_error("Package dependency error");
			}
		}
	}
}

void Package::check_conflicts() const
{
	for( unsigned i = 0; i < conflicts_.elems.size(); ++i )
	{
		const PackageList::Elem& elem = conflicts_.elems[i];
		Package* found = find_package( elem.pkgname );
		if (found != NULL)
		{
			cerr << "Error in package " << desc() << ":" << endl;
			cerr << "	Package conflicts with package " << found->desc() << endl;
			throw runtime_error("Package dependency error");
		}
	}
}

void load_package( const std::string& pkg_dir, ConfigElem& elem, bool quiet )
{
	auto_ptr<Package> pkg(new Package( pkg_dir, elem )); 
	Package* existing_pkg = find_package( pkg->name() );

	if (existing_pkg != NULL)
	{
		bool isgreater, isequal;
		compare_versions( pkg->version(), existing_pkg->version(), isgreater, isequal );
		if (isgreater)
		{
			// replace existing package with newer version
			if (!quiet)
				cout << "Replacing package " << existing_pkg->desc() 
				 << " version " << existing_pkg->version() 
				 << " with version " << pkg->version() << " found in " << pkg->dir()
				 << endl;
			remove_package( existing_pkg );
			delete existing_pkg;
			existing_pkg = NULL;
		}
		else if (isequal)
		{
			cerr << "Error in package " << pkg->desc() << ":" << endl;
			cerr << "	Package by same name already found in " << existing_pkg->desc() << endl;
			throw runtime_error("Duplicate package found");
		}
		else
		{
			// skip this package, its version is older
			if (!quiet)
			   cout << "Skipping package " << pkg->desc() 
				<< " version " << pkg->version()
				<< " because version " << existing_pkg->version() << " was already found in " 
				<< existing_pkg->dir()
				<< endl;
			return;
		}
	}

	packages.push_back( pkg.get() ); 
	Package* ppkg = pkg.release();
	packages_byname.insert( PackagesByName::value_type( ppkg->name(), ppkg ) );
}


void load_packages( string basedir, bool quiet )
{

	for( DirList dl( basedir.c_str() ); !dl.at_end(); dl.next() )
	{
		string dirname = dl.name();
		if (dirname[0] == '.') continue;
		if (dirname == "template") continue;

		string pkg_dir = basedir + dirname + "/";
		string pkg_cfg = pkg_dir + "pkg.cfg";

		if (FileExists( pkg_cfg.c_str() ))
		{
			ConfigFile cf( pkg_cfg.c_str() );
			ConfigElem elem;

			cf.readraw( elem );
			string enabled_pkg = pkg_dir + "enabled.pkg";
			string disabled_pkg = pkg_dir + "disabled.pkg";

			if ((elem.remove_bool( "Enabled" ) == true ||
				 FileExists( enabled_pkg.c_str() ) ) &&
				!FileExists( disabled_pkg.c_str() ))
			{
				if (!quiet) 
					cout << "Loading package in " << pkg_dir << endl;
				load_package( pkg_dir, elem, quiet );
				
				load_packages( pkg_dir, quiet );
			}
		}
		else
		{
			load_packages( pkg_dir, quiet );
		}
	}
}

void check_replaced_packages( Package* pkg )
{
	pkg->check_replacements();
}

void check_deps_for_package( Package* pkg )
{
	pkg->check_dependencies();
	pkg->check_conflicts();
}

void replace_packages()
{
	bool done;
	do
	{
		done = true;

		for( unsigned i = 0; i < packages.size(); ++i )
		{
			Package* pkg = packages[i];
			bool change = pkg->check_replacements();
			if (change)
			{
				done = false;
				break;
			}
		}

	} while (!done);
}

void check_package_deps()
{
	ForEach( packages, check_deps_for_package );
}

void load_packages( )
{
	test_check_version();

	load_packages( "pkg/" );
	
	if (FileExists( "config/pkgroots.cfg" ))
	{
		ConfigFile cf( "config/pkgroots.cfg", "PackageRoot" );
		ConfigElem elem;
		while (cf.read( elem ))
		{   
			std::string dir;
			while (elem.remove_prop( "dir", &dir ))
			{
				dir = normalized_dir_form( dir );
				cout << "Searching for packages under " << dir << endl;
				load_packages( dir.c_str() );
			}
		}
	}
	
	replace_packages();

	check_package_deps();
}

void unload_packages()
{
	delete_all( packages );
	packages_byname.clear();
}

bool pkgdef_split( const string& spec, const Package* inpkg,
				   const Package** outpkg, string* path )
{
	if (spec[0] == ':')
	{
		if (spec[1] == ':') // '::corefile'  -- a core file
		{
			*outpkg = NULL;
			*path = spec.substr( 2, string::npos );
		}
		else				// ':pkgname:pkgfile'  -- a packaged file
		{
			string::size_type second_colon = spec.find( ':', 2 );
			if (second_colon != string::npos)
			{
				string pkgname = spec.substr( 1, second_colon-1  );
				string pkgfile = spec.substr( second_colon+1, string::npos );
				Package* dstpkg = find_package( pkgname );
				if (dstpkg != NULL)
				{
					*outpkg = dstpkg;
					*path = pkgfile;
				}
				else
				{
					cerr << "Unable to find package '" << pkgname << "'" << endl;
					return false;
				}
			}
			else
			{
				cerr << "Poorly formed packagefile descriptor: '" << spec << "'" << endl;
				return false;
			}

		}
	}
	else
	{
		*outpkg = inpkg;
		*path =  spec;
	}
	return true;
}

void load_packaged_cfgs( const char* cfgname,
						 const char* taglist,
						 void(*loadentry)(const Package*,ConfigElem&) )
{
	for( Packages::const_iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		string filename = GetPackageCfgPath(pkg, cfgname);
		if (FileExists( filename.c_str() ))
		{
			ConfigFile cf( filename.c_str(), taglist );
			ConfigElem elem;

			while (cf.read( elem ))
			{
				loadentry( pkg, elem );
			}
		}
	}
}

void load_all_cfgs( const char* cfgname,
						 const char* taglist,
						 void(*loadentry)(const Package*,ConfigElem&) )
{
	string filename = string("config/") + cfgname;
	if (FileExists(filename))
	{
		ConfigFile cf( filename.c_str(), taglist );
		ConfigElem elem;

		while (cf.read( elem ))
		{
			loadentry( NULL, elem );
		}
	}
	load_packaged_cfgs( cfgname, taglist, loadentry );
}


string GetPackageCfgPath(Package* pkg, string filename)
{
	string filepath;
	if ( pkg == NULL )
	{	// If no package is sent, assume pol/config/file.xxx
		filepath = "config/"+filename;
	}
	else
	{	// ** Going to save this feature for 097 **
		// With packages, first try for /pkg/config/file.xxx
		filepath = pkg->dir() + "config/"+filename;
		if ( !FileExists(filepath) )
		{
			// Lastly, assume /pkg/file.xxx
			filepath = pkg->dir() + filename;
		}
	}

	return filepath;
}
