/*
History
=======

Notes
=======

*/

#ifndef PKG_H
#define PKG_H

#include <string>
#include <vector>

class ConfigElem;

class PackageList
{
private:
    friend class Package;
    PackageList( ConfigElem& elem, const char* tag );

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
    Package( const std::string& pkg_dir, ConfigElem& elem );

    const std::string& dir() const;
    const std::string& name() const;
    const std::string& version() const;
    std::string desc() const;
    bool provides_system_home_page() const;

    bool check_replacements() const;
    void check_dependencies() const;
    void check_conflicts() const;

private:
    std::string dir_;
    std::string name_;
    std::string version_;

    // only one of these two will be set.
    unsigned short core_required_;
    std::string core_versionstring_required_;

    PackageList requires_;
    PackageList conflicts_;
    PackageList replaces_;

    bool provides_system_home_page_;

private: // not implemented:
    Package( const Package& );
    Package& operator=( const Package& );
};

typedef std::vector<Package*> Packages;
extern Packages packages;

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

bool pkgdef_split( const std::string& spec, const Package* inpkg,
                  const Package** outpkg, std::string* path );

void load_packaged_cfgs( const char* cfgname,
                         const char* taglist,
                         void(*loadentry)(const Package*,ConfigElem&) );
void load_all_cfgs( const char* cfgname,
                         const char* taglist,
                         void(*loadentry)(const Package*,ConfigElem&) );

void load_packages( std::string basedir, bool quiet = false );
void replace_packages();
void check_package_deps();
void unload_packages();
std::string GetPackageCfgPath(Package* pkg, std::string filename);

#endif
