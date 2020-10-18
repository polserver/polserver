/** @file
 *
 * @par History
 */


#ifndef PKG_H
#define PKG_H

#include <string>
#include <vector>

namespace Pol
{
namespace Clib
{
class ConfigElem;
}

/**
 * Plib namespace is for pol-related functions and classes, like reading a map file or keeping track
 * of realms
 */
namespace Plib
{
class PackageList
{
private:
  friend class Package;
  PackageList( Clib::ConfigElem& elem, const char* tag );
  size_t sizeEstimate() const;
  struct Elem
  {
    std::string pkgname;
    std::string version;
  };

  std::vector<Elem> elems;
};

class Package
{
public:
  Package( const std::string& pkg_dir, Clib::ConfigElem& elem );

  const std::string& dir() const;
  const std::string& name() const;
  const std::string& version() const;
  std::string desc() const;
  bool provides_system_home_page() const;

  bool check_replacements() const;
  void check_dependencies() const;
  void check_conflicts() const;

  size_t estimateSize() const;

private:
  std::string dir_;
  std::string name_;
  std::string version_;

  std::string core_required_;

  PackageList requires_;
  PackageList conflicts_;
  PackageList replaces_;

  bool provides_system_home_page_;

private:  // not implemented:
  Package( const Package& );
  Package& operator=( const Package& );
};


inline const std::string& Package::dir() const
{
  return dir_;
}
inline const std::string& Package::name() const
{
  return name_;
}
inline const std::string& Package::version() const
{
  return version_;
}
Package* find_package( const std::string& pkgname );

bool pkgdef_split( const std::string& spec, const Package* inpkg, const Package** outpkg,
                   std::string* path );

void load_packaged_cfgs( const char* cfgname, const char* taglist,
                         void ( *loadentry )( const Package*, Clib::ConfigElem& ) );
void load_all_cfgs( const char* cfgname, const char* taglist,
                    void ( *loadentry )( const Package*, Clib::ConfigElem& ) );

void load_packages( bool quiet = false );
void load_packages( const std::string& basedir, bool quiet = false );
void replace_packages();
void check_package_deps();
std::string GetPackageCfgPath( const Package* pkg, const std::string& filename );
}
}
#endif
