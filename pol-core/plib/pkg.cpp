/** @file
 *
 * @par History
 */


#include "pkg.h"

#include "pol_global_config.h"
#include <stdlib.h>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/clib.h"
#include "../clib/dirlist.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "systemstate.h"

namespace Pol
{
namespace Plib
{
bool Package::provides_system_home_page() const
{
  return provides_system_home_page_;
}


Package* find_package( const std::string& pkgname )
{
  auto itr = systemstate.packages_byname.find( pkgname );
  if ( itr != systemstate.packages_byname.end() )
  {
    return ( *itr ).second;
  }
  else
  {
    return nullptr;
  }
}

void remove_package( Package* pkg )
{
  auto last = std::remove_if( systemstate.packages.begin(), systemstate.packages.end(),
                              std::bind2nd( std::equal_to<Package*>(), pkg ) );
  systemstate.packages.erase( last, systemstate.packages.end() );

  // TODO: Check this loop. It looks odd.
  //       Should the loop stop after removing the package?
  //       Is it possible to have more than one name for the same package?

  auto itr = systemstate.packages_byname.begin();
  while ( itr != systemstate.packages_byname.end() )
  {
    auto tempitr = itr;
    ++itr;
    if ( ( *tempitr ).second == pkg )
      systemstate.packages_byname.erase( tempitr );
  }
}

void compare_versions( const std::string& verleft, const std::string& verright, bool& isgreater,
                       bool& isequal )
{
  isgreater = isequal = false;

  const char* vneed = verright.c_str();
  const char* vhave = verleft.c_str();

  while ( ( vneed != nullptr && vneed[0] != '\0' ) || ( vhave != nullptr && vhave[0] != '\0' ) )
  {
    char* new_vneed = nullptr;
    char* new_vhave = nullptr;
    unsigned int vneedpart, vhavepart;
    vneedpart = ( vneed != nullptr ) ? strtoul( vneed, &new_vneed, 0 ) : 0;
    vhavepart = ( vhave != nullptr ) ? strtoul( vhave, &new_vhave, 0 ) : 0;

    if ( vhavepart > vneedpart )
    {
      isgreater = true;
      return;
    }
    else if ( vhavepart < vneedpart )
    {
      return;
    }
    else  // same, so check the next one
    {
      vneed = new_vneed;
      if ( vneed && *vneed )
        ++vneed;
      vhave = new_vhave;
      if ( vhave && *vhave )
        ++vhave;
    }
  }
  isequal = true;
}

bool check_version2( const std::string& version_have, const std::string& version_need )
{
  bool isgreater;
  bool isequal;
  compare_versions( version_have, version_need, isgreater, isequal );
  return ( isequal || isgreater );
}

void test_check_version()
{  // have // need
  passert( check_version2( "0", "0" ) == true );
  passert( check_version2( "1", "0" ) == true );
  passert( check_version2( "0", "1" ) == false );
  passert( check_version2( "0.5", "1" ) == false );
  passert( check_version2( "0.5", "0" ) == true );
  passert( check_version2( "1.2", "1.12" ) == false );
  passert( check_version2( "1.12", "1.2" ) == true );
  passert( check_version2( "1.2.3", "1" ) == true );
  passert( check_version2( "1.1", "1.2.3" ) == false );
  passert( check_version2( "1.3", "1.2.3" ) == true );
}

PackageList::PackageList( Clib::ConfigElem& elem, const char* tag )
{
  std::string tmp;
  while ( elem.remove_prop( tag, &tmp ) )
  {
    Clib::mklowerASCII( tmp );
    ISTRINGSTREAM is( tmp );
    Elem _elem;
    if ( is >> _elem.pkgname )
    {
      if ( !( is >> _elem.version ) )
        _elem.version = "0";

      elems.push_back( _elem );
    }
  }
}
size_t PackageList::sizeEstimate() const
{
  size_t size = sizeof( PackageList );
  for ( const auto& elem : elems )
    size += elem.pkgname.capacity() + elem.version.capacity();
  return size;
}

Package::Package( const std::string& pkg_dir, Clib::ConfigElem& elem )
    : dir_( pkg_dir ),
      name_( elem.remove_string( "Name" ) ),
      version_( elem.remove_string( "Version", "0" ) ),
      core_required_( 0 ),
      requires_( elem, "Requires" ),
      conflicts_( elem, "Conflicts" ),
      replaces_( elem, "Replaces" ),
      provides_system_home_page_( elem.remove_bool( "ProvidesSystemHomePage", false ) )
{
  Clib::mklowerASCII( name_ );
  // CoreRequired can either be a number (94,,95 etc)
  // or a version string (POL095-2003-01-28)
  std::string tmp = elem.read_string( "CoreRequired", "0" );
  if ( isdigit( tmp[0] ) )
  {
    // the first kind - a number.
    core_required_ = elem.remove_ushort( "CoreRequired", 0 );
  }
  else
  {
    core_versionstring_required_ = elem.remove_string( "CoreRequired" );
  }
}

std::string Package::desc() const
{
  return name() + " (" + dir() + ")";
}

bool Package::check_replacements() const
{
  bool any = false;
  for ( const auto& elem : replaces_.elems )
  {
    Package* found = find_package( elem.pkgname );
    if ( found != nullptr )
    {
      any = true;
      INFO_PRINT << "Package " << desc() << " replaces package " << found->desc() << "\n";
      remove_package( found );
      delete found;
      found = nullptr;
    }
  }
  return any;
}

void Package::check_dependencies() const
{
  if ( core_required_ )
  {
    if ( core_required_ > POL_VERSION )  // TODO: use a more fine grained check here
    {
      ERROR_PRINT << "Error in package " << desc() << ":\n"
                  << "  Core version " << core_required_ << " is required, but version "
                  << POL_VERSION << " is running.\n";
      throw std::runtime_error( "Package requires a newer core version" );
    }
  }
  else if ( !core_versionstring_required_.empty() )
  {
    int cmp = stricmp( POL_VERSION_ID, core_versionstring_required_.c_str() );
    if ( cmp < 0 )
    {
      ERROR_PRINT << "Error in package " << desc() << ":\n"
                  << "  Core version " << core_versionstring_required_
                  << " is required, but version " << POL_VERSION_ID << " is running.\n";
      throw std::runtime_error( "Package requires a newer core version" );
    }
  }
  for ( const auto& elem : requires_.elems )
  {
    Package* found = find_package( elem.pkgname );
    if ( found == nullptr )
    {
      ERROR_PRINT << "Error in package '" << name_ << "' (" << dir_ << "):\n"
                  << "  Package '" << elem.pkgname << "' is required, but is not installed.\n";
      throw std::runtime_error( "Package dependency error" );
    }
    else
    {
      if ( !check_version2( found->version_, elem.version ) )
      {
        ERROR_PRINT << "Error in package '" << name_ << "' (" << dir_ << "):\n"
                    << "  Package '" << elem.pkgname << "' version " << elem.version
                    << " is required, but version " << found->version_ << " was found\n";
        throw std::runtime_error( "Package dependency error" );
      }
    }
  }
}

void Package::check_conflicts() const
{
  for ( const auto& elem : conflicts_.elems )
  {
    Package* found = find_package( elem.pkgname );
    if ( found != nullptr )
    {
      ERROR_PRINT << "Error in package " << desc() << ":\n"
                  << "  Package conflicts with package " << found->desc() << "\n";
      throw std::runtime_error( "Package dependency error" );
    }
  }
}

size_t Package::estimateSize() const
{
  size_t size = dir_.capacity() + name_.capacity() + version_.capacity() +
                sizeof( unsigned short ) /*core_required*/
                + core_versionstring_required_.capacity() + requires_.sizeEstimate() +
                conflicts_.sizeEstimate() + replaces_.sizeEstimate() +
                sizeof( bool ) /*provides_system_home_page_*/
      ;
  return size;
}

void load_package( const std::string& pkg_dir, Clib::ConfigElem& elem, bool quiet )
{
  std::unique_ptr<Package> pkg( new Package( pkg_dir, elem ) );
  Package* existing_pkg = find_package( pkg->name() );

  if ( existing_pkg != nullptr )
  {
    bool isgreater, isequal;
    compare_versions( pkg->version(), existing_pkg->version(), isgreater, isequal );
    if ( isgreater )
    {
      // replace existing package with newer version
      if ( !quiet )
        INFO_PRINT << "Replacing package " << existing_pkg->desc() << " version "
                   << existing_pkg->version() << " with version " << pkg->version() << " found in "
                   << pkg->dir() << "\n";
      remove_package( existing_pkg );
      delete existing_pkg;
      existing_pkg = nullptr;
    }
    else if ( isequal )
    {
      ERROR_PRINT << "Error in package " << pkg->desc() << ":\n"
                  << "  Package by same name already found in " << existing_pkg->desc() << "\n";
      throw std::runtime_error( "Duplicate package found" );
    }
    else
    {
      // skip this package, its version is older
      if ( !quiet )
        INFO_PRINT << "Skipping package " << pkg->desc() << " version " << pkg->version()
                   << " because version " << existing_pkg->version() << " was already found in "
                   << existing_pkg->dir() << "\n";
      return;
    }
  }

  systemstate.packages.push_back( pkg.get() );
  Package* ppkg = pkg.release();
  systemstate.packages_byname.insert( PackagesByName::value_type( ppkg->name(), ppkg ) );
}


void load_packages( const std::string& basedir, bool quiet )
{
  for ( Clib::DirList dl( basedir.c_str() ); !dl.at_end(); dl.next() )
  {
    std::string dirname = dl.name();
    if ( dirname[0] == '.' )
      continue;
    if ( dirname == "template" )
      continue;

    std::string pkg_dir = basedir + dirname + "/";
    std::string pkg_cfg = pkg_dir + "pkg.cfg";

    if ( Clib::FileExists( pkg_cfg.c_str() ) )
    {
      Clib::ConfigFile cf( pkg_cfg.c_str() );
      Clib::ConfigElem elem;

      cf.readraw( elem );
      std::string enabled_pkg = pkg_dir + "enabled.pkg";
      std::string disabled_pkg = pkg_dir + "disabled.pkg";

      if ( ( elem.remove_bool( "Enabled" ) == true || Clib::FileExists( enabled_pkg.c_str() ) ) &&
           !Clib::FileExists( disabled_pkg.c_str() ) )
      {
        if ( !quiet )
          INFO_PRINT << "Loading package in " << pkg_dir << "\n";
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

void check_deps_for_package( const Package* pkg )
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

    for ( const auto& pkg : systemstate.packages )
    {
      bool change = pkg->check_replacements();
      if ( change )
      {
        done = false;
        break;
      }
    }

  } while ( !done );
}

void check_package_deps()
{
  for ( const auto& pkg : systemstate.packages )
    check_deps_for_package( pkg );
}

void load_packages( bool quiet )
{
  test_check_version();

  load_packages( "pkg/", quiet );

  if ( Clib::FileExists( "config/pkgroots.cfg" ) )
  {
    Clib::ConfigFile cf( "config/pkgroots.cfg", "PackageRoot" );
    Clib::ConfigElem elem;
    while ( cf.read( elem ) )
    {
      std::string dir;
      while ( elem.remove_prop( "dir", &dir ) )
      {
        dir = Clib::normalized_dir_form( dir );
        if ( !quiet )
          INFO_PRINT << "Searching for packages under " << dir << "\n";
        load_packages( dir.c_str(), quiet );
      }
    }
  }

  replace_packages();

  check_package_deps();
}

bool pkgdef_split( const std::string& spec, const Package* inpkg, const Package** outpkg,
                   std::string* path )
{
  if ( spec[0] == ':' )
  {
    if ( spec[1] == ':' )  // '::corefile'  -- a core file
    {
      *outpkg = nullptr;
      *path = spec.substr( 2, std::string::npos );
    }
    else  // ':pkgname:pkgfile'  -- a packaged file
    {
      std::string::size_type second_colon = spec.find( ':', 2 );
      if ( second_colon != std::string::npos )
      {
        std::string pkgname = spec.substr( 1, second_colon - 1 );
        std::string pkgfile = spec.substr( second_colon + 1, std::string::npos );
        Package* dstpkg = find_package( pkgname );
        if ( dstpkg != nullptr )
        {
          *outpkg = dstpkg;
          *path = pkgfile;
        }
        else
        {
          ERROR_PRINT << "Unable to find package '" << pkgname << "'\n";
          return false;
        }
      }
      else
      {
        ERROR_PRINT << "Poorly formed packagefile descriptor: '" << spec << "'\n";
        return false;
      }
    }
  }
  else
  {
    *outpkg = inpkg;
    *path = spec;
  }
  return true;
}

void load_packaged_cfgs( const char* cfgname, const char* taglist,
                         void ( *loadentry )( const Package*, Clib::ConfigElem& ) )
{
  for ( const auto& pkg : systemstate.packages )
  {
    std::string filename = GetPackageCfgPath( pkg, cfgname );
    if ( Clib::FileExists( filename.c_str() ) )
    {
      Clib::ConfigFile cf( filename.c_str(), taglist );
      Clib::ConfigElem elem;

      while ( cf.read( elem ) )
      {
        loadentry( pkg, elem );
      }
    }
  }
}

void load_all_cfgs( const char* cfgname, const char* taglist,
                    void ( *loadentry )( const Package*, Clib::ConfigElem& ) )
{
  std::string filename = std::string( "config/" ) + cfgname;
  if ( Clib::FileExists( filename ) )
  {
    Clib::ConfigFile cf( filename.c_str(), taglist );
    Clib::ConfigElem elem;

    while ( cf.read( elem ) )
    {
      loadentry( nullptr, elem );
    }
  }
  load_packaged_cfgs( cfgname, taglist, loadentry );
}


std::string GetPackageCfgPath( const Package* pkg, const std::string& filename )
{
  std::string filepath;
  if ( pkg == nullptr )
  {  // If no package is sent, assume pol/config/file.xxx
    filepath = "config/" + filename;
  }
  else
  {  // ** Going to save this feature for 097 **
    // With packages, first try for /pkg/config/file.xxx
    filepath = pkg->dir() + "config/" + filename;
    if ( !Clib::FileExists( filepath ) )
    {
      // Lastly, assume /pkg/file.xxx
      filepath = pkg->dir() + filename;
    }
  }

  return filepath;
}
}  // namespace Plib
}  // namespace Pol
