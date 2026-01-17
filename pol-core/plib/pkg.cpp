/** @file
 *
 * @par History
 */


#include "pkg.h"

#include "pol_global_config.h"
#include <algorithm>
#include <filesystem>
#include <stdlib.h>
#include <system_error>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/clib.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "systemstate.h"

namespace fs = std::filesystem;

namespace Pol::Plib
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

  return nullptr;
}

void remove_package( Package* pkg )
{
  auto last = std::remove( systemstate.packages.begin(), systemstate.packages.end(), pkg );
  systemstate.packages.erase( last, systemstate.packages.end() );

  auto itr = systemstate.packages_byname.begin();
  while ( itr != systemstate.packages_byname.end() )
  {
    if ( itr->second == pkg )
    {
      systemstate.packages_byname.erase( itr );
      break;
    }
    ++itr;
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
    if ( vhavepart < vneedpart )
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

bool version_greater_or_equal( const std::string& version_have, const std::string& version_need )
{
  bool isgreater;
  bool isequal;
  compare_versions( version_have, version_need, isgreater, isequal );
  return ( isequal || isgreater );
}

bool version_equal( const std::string& version_have, const std::string& version_need )
{
  bool isgreater;
  bool isequal;
  compare_versions( version_have, version_need, isgreater, isequal );
  return isequal;
}

void test_check_version()
{  // have // need
  passert( version_greater_or_equal( "0", "0" ) == true );
  passert( version_greater_or_equal( "1", "0" ) == true );
  passert( version_greater_or_equal( "0", "1" ) == false );
  passert( version_greater_or_equal( "0.5", "1" ) == false );
  passert( version_greater_or_equal( "0.5", "0" ) == true );
  passert( version_greater_or_equal( "1.2", "1.12" ) == false );
  passert( version_greater_or_equal( "1.12", "1.2" ) == true );
  passert( version_greater_or_equal( "1.2.3", "1" ) == true );
  passert( version_greater_or_equal( "1.1", "1.2.3" ) == false );
  passert( version_greater_or_equal( "1.3", "1.2.3" ) == true );

  passert( version_equal( "93", "93.0.0" ) == true );
  passert( version_equal( "93.0.0", "93" ) == true );
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
    : dir_( Clib::normalized_dir_form( pkg_dir ) ),
      name_( elem.remove_string( "Name" ) ),
      version_( elem.remove_string( "Version", "0" ) ),
      core_required_( "" ),
      requires_( elem, "Requires" ),
      conflicts_( elem, "Conflicts" ),
      replaces_( elem, "Replaces" ),
      provides_system_home_page_( elem.remove_bool( "ProvidesSystemHomePage", false ) )
{
  Clib::mklowerASCII( name_ );
  std::string tmp = elem.read_string( "CoreRequired", "0" );
  if ( isdigit( tmp[0] ) )
  {
    core_required_ = elem.remove_string( "CoreRequired", "" );
  }
  else
  {
    // Forms like CoreRequired POL095-2003-01-28 are no longer allowed.
    ERROR_PRINTLN(
        "Error in package {}({}):\n"
        "  Core version must be a semantic version (n.n.n), but got "
        "\"{}\".",
        name_, dir_, core_required_ );
    throw std::runtime_error( "Malformed CoreRequired package field" );
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
      INFO_PRINTLN( "Package {} replaces package {}", desc(), found->desc() );
      remove_package( found );
      delete found;
      found = nullptr;
    }
  }
  return any;
}

void Package::check_dependencies() const
{
  if ( !core_required_.empty() )
  {
    if ( !( version_greater_or_equal( POL_VERSION_STR, core_required_ ) ) )
    {
      ERROR_PRINTLN(
          "Error in package {}:\n"
          "  Core version {} is required, but version {} is running.",
          desc(), core_required_, POL_VERSION_STR );
      throw std::runtime_error( "Package requires a newer core version" );
    }
  }
  for ( const auto& elem : requires_.elems )
  {
    Package* found = find_package( elem.pkgname );
    if ( found == nullptr )
    {
      ERROR_PRINTLN(
          "Error in package '{}' ({}):\n"
          "  Package '{}' is required, but is not installed.",
          name_, dir_, elem.pkgname );
      throw std::runtime_error( "Package dependency error" );
    }

    if ( !version_greater_or_equal( found->version_, elem.version ) )
    {
      ERROR_PRINTLN(
          "Error in package '{}' ({}):\n"
          "  Package '{}' version {} is required, but version {} "
          "was found",
          name_, dir_, elem.pkgname, elem.version, found->version_ );
      throw std::runtime_error( "Package dependency error" );
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
      ERROR_PRINTLN(
          "Error in package {}:\n"
          "  Package conflicts with package {}",
          desc(), found->desc() );
      throw std::runtime_error( "Package dependency error" );
    }
  }
}

size_t Package::estimateSize() const
{
  size_t size = dir_.capacity() + name_.capacity() + version_.capacity() +
                +core_required_.capacity() + requires_.sizeEstimate() + conflicts_.sizeEstimate() +
                replaces_.sizeEstimate() + sizeof( bool ) /*provides_system_home_page_*/
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
        INFO_PRINTLN( "Replacing package {} version {} with version {} found in {}",
                      existing_pkg->desc(), existing_pkg->version(), pkg->version(), pkg->dir() );
      remove_package( existing_pkg );
      delete existing_pkg;
      existing_pkg = nullptr;
    }
    else if ( isequal )
    {
      ERROR_PRINTLN(
          "Error in package {}:\n"
          "  Package by same name already found in {}",
          pkg->desc(), existing_pkg->desc() );
      throw std::runtime_error( "Duplicate package found" );
    }
    else
    {
      // skip this package, its version is older
      if ( !quiet )
        INFO_PRINTLN( "Skipping package {} version {} because version {} was already found in ",
                      pkg->desc(), pkg->version(), existing_pkg->version(), existing_pkg->dir() );
      return;
    }
  }

  systemstate.packages.push_back( pkg.get() );
  Package* ppkg = pkg.release();
  systemstate.packages_byname.insert( PackagesByName::value_type( ppkg->name(), ppkg ) );
}


void load_packages( const std::string& basedir, bool quiet )
{
  std::error_code ec;
  for ( auto dir_itr = fs::recursive_directory_iterator( basedir, ec );
        dir_itr != fs::recursive_directory_iterator(); ++dir_itr )
  {
    if ( !dir_itr->is_directory() )
      continue;
    if ( auto fn = dir_itr->path().filename().string();
         !fn.empty() && ( *fn.begin() == '.' || fn == "template" ) )
    {
      dir_itr.disable_recursion_pending();
      continue;
    }
    const auto pkg_dir = dir_itr->path();
    const auto pkg_cfg = pkg_dir / "pkg.cfg";
    if ( !fs::exists( pkg_cfg ) )
      continue;
    Clib::ConfigFile cf( pkg_cfg.string().c_str() );
    Clib::ConfigElem elem;

    cf.readraw( elem );

    if ( ( elem.remove_bool( "Enabled" ) == true || fs::exists( pkg_dir / "enabled.pkg" ) ) &&
         !fs::exists( pkg_dir / "disabled.pkg" ) )
    {
      if ( !quiet )
        INFO_PRINTLN( "Loading package in {}", pkg_dir.string() );
      load_package( pkg_dir.string() + "/", elem, quiet );
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
          INFO_PRINTLN( "Searching for packages under {}", dir );
        load_packages( dir.c_str(), quiet );
      }
    }
  }

  replace_packages();

  check_package_deps();
  // sort pkg vector by name, so e.g. startup order is in a defined and maybe also expected order.
  std::sort( systemstate.packages.begin(), systemstate.packages.end(),
             []( const Package* pkg1, const Package* pkg2 )
             { return pkg1->name() < pkg2->name(); } );
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
          ERROR_PRINTLN( "Unable to find package '{}'", pkgname );
          return false;
        }
      }
      else
      {
        ERROR_PRINTLN( "Poorly formed packagefile descriptor: '{}'", spec );
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
}  // namespace Pol::Plib
