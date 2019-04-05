/** @file
 *
 * @par History
 */


#include "scrdef.h"

#include "../bscript/escrutil.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../plib/pkg.h"
#include "scrstore.h"

namespace Pol
{
namespace Core
{
extern std::map<std::string, Bscript::Program* (*)()> map;

std::string full_scriptname( const std::string& spec, const Plib::Package* pkg,
                             const char* mainpfx )
{
  if ( spec.empty() )
    return spec;

  std::string prefix = ( pkg != nullptr ) ? pkg->dir() + spec :

                                          ( spec.find( '/' ) == std::string::npos )
                                              ? mainpfx + spec
                                              : "scripts/" + spec;
  for ( auto& kv : map )
  {
    if ( Clib::endsWith( prefix, kv.first ) )
      return prefix;

    std::string possibleName = prefix + kv.first;
    if ( Clib::FileExists( possibleName ) )
      return possibleName;
  }

  return Bscript::normalize_ecl_filename( prefix );
}

ScriptDef::ScriptDef( const std::string& iname, const Plib::Package* ipkg, const char* mainpfx )
{
  config( iname, ipkg, mainpfx, true );
}

ScriptDef::ScriptDef( const std::string& iname, const Plib::Package* ipkg )
{
  config( iname, ipkg, "", true );
}

bool ScriptDef::operator==( const ScriptDef& other ) const
{
  if ( empty() && other.empty() )
    return true;

  return pkg_ == other.pkg_ && name_ == other.name_;
}
bool ScriptDef::operator!=( const ScriptDef& other ) const
{
  return !( *this == other );
}

void ScriptDef::config( const std::string& iname, const Plib::Package* ipkg, const char* mainpfx,
                        bool warn_if_not_found )
{
  std::string path;
  const Plib::Package* npkg;
  if ( !Plib::pkgdef_split( iname, ipkg, &npkg, &path ) )
  {
    ERROR_PRINT << "Error reading script descriptor '" << iname << "'\n";
    throw std::runtime_error( "Error reading script descriptor" );
  }

  localname_ = path;
  name_ = full_scriptname( path, npkg, mainpfx );
  pkg_ = npkg;

  if ( warn_if_not_found )
  {
    if ( !empty() && !exists() )
    {
      ERROR_PRINT << "Warning! " << name_.get() << " does not exist!\n";
    }
  }
}

void ScriptDef::config( const std::string& iname, const Plib::Package* ipkg )
{
  config( iname, ipkg, "", true );
}

bool ScriptDef::config_nodie( const std::string& iname, const Plib::Package* ipkg,
                              const char* mainpfx )
{
  std::string path;
  const Plib::Package* npkg;
  if ( !Plib::pkgdef_split( iname, ipkg, &npkg, &path ) )
  {
    ERROR_PRINT << "Error reading script descriptor '" << iname << "'\n";
    return false;
  }

  localname_ = iname;
  name_ = full_scriptname( path, npkg, mainpfx );
  pkg_ = npkg;

  return true;
}

std::string ScriptDef::relativename( const Plib::Package* pkg ) const
{
  if ( name_ == "" )
    return "";
  else if ( pkg == pkg_ )
    return localname_;
  else
    return ":" + ( pkg_ ? pkg_->name() : "" ) + ":" + localname_.get();
}

void ScriptDef::quickconfig( const Plib::Package* pkg, const std::string& name_ecl )
{
  localname_ = "unknown";
  name_ = pkg->dir() + name_ecl;
  pkg_ = pkg;
}

void ScriptDef::quickconfig( const std::string& name_ecl )
{
  localname_ = "unknown";
  name_ = name_ecl;
  pkg_ = nullptr;
}

bool ScriptDef::exists() const
{
  if ( empty() )
    return false;

  else if ( Clib::FileExists( c_str() ) )
    return true;

  for ( auto& kv : map )
  {
    std::string possibleName = name_.get() + kv.first;
    if ( !Clib::endsWith( name_.get(), kv.first ) && Clib::FileExists( possibleName.c_str() ) )
      return true;
  }
  return false;

  // ref_ptr<EScriptProgram> prog = find_script2( *this, false, true );
  // return (prog.get() != nullptr);
}

void ScriptDef::clear()
{
  localname_ = "";
  name_ = "";
  pkg_ = nullptr;
}

size_t ScriptDef::estimatedSize() const
{
  return sizeof( ScriptDef );
}
}
}
