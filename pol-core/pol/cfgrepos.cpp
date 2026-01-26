/** @file
 *
 * @par History
 * - 2005/07/01 Shinigami: added StoredConfigFile::modified_ (stat.st_mtime) to detect cfg file
 * modification
 *                         added oldcfgfiles - holds all references to "removed" cfg files
 *                         UnloadConfigFile will reload file if modified only
 * - 2005/07/04 Shinigami: added StoredConfigFile::reload to check for file modification
 *                         moved file modification check from UnloadConfigFile to FindConfigFile
 *                         added ConfigFiles_log_stuff for memory logging
 * - 2007/04/28 Shinigami: polcore().internal information will be logged in excel-friendly format
 * too (leak.log)
 *
 * @note Configuration File Repository
 */


#include "cfgrepos.h"

#include <ctype.h>
#include <exception>
#include <iosfwd>
#include <stdlib.h>
#include <sys/stat.h>

#include "../bscript/bobject.h"
#include "../bscript/escrutil.h"
#include "../bscript/impstr.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include "globals/ucfg.h"


namespace Pol::Core
{
StoredConfigElem::StoredConfigElem( Clib::ConfigElem& elem )
{
  std::string propname, propval;

  while ( elem.remove_first_prop( &propname, &propval ) )
  {
    Bscript::BObjectImp* newimp = Bscript::bobject_from_string( propval );

    addprop( propname, newimp );
  }
}

// ToDo: we have to think over... it's a problem with script-inside references
StoredConfigElem::~StoredConfigElem()
{
  //  while (!propimps_.empty())
  //  {
  //    delete ((*propimps_.begin()).second);
  //    propimps_.erase( propimps_.begin() );
  //  }
}

void StoredConfigElem::addprop( const std::string& propname, Bscript::BObjectImp* imp )
{
  propimps_.insert( PropImpList::value_type( boost_utils::cfg_key_flystring( propname ),
                                             ref_ptr<class Bscript::BObjectImp>( imp ) ) );
}

Bscript::BObjectImp* StoredConfigElem::getimp( const std::string& propname ) const
{
  PropImpList::const_iterator itr = propimps_.find( propname );
  if ( itr == propimps_.end() )
    return nullptr;
  return ( *itr ).second.get();
}

Bscript::BObjectImp* StoredConfigElem::listprops() const
{
  Bscript::ObjArray* objarr = new Bscript::ObjArray;
  PropImpList::const_iterator itr;
  for ( itr = propimps_.begin(); itr != propimps_.end(); ++itr )
  {
    Bscript::String propname( ( *itr ).first );
    if ( !objarr->contains( propname ) )
      objarr->addElement( propname.copy() );
  }
  return objarr;
}

std::pair<StoredConfigElem::const_iterator, StoredConfigElem::const_iterator>
StoredConfigElem::equal_range( const std::string& propname ) const
{
  return propimps_.equal_range( propname );
}

size_t StoredConfigElem::estimateSize() const
{
  size_t size = Clib::memsize( propimps_ );
  for ( const auto& pair : propimps_ )
  {
    if ( pair.second.get() != nullptr )
      size += pair.second->sizeEstimate();
  }
  return size;
}

StoredConfigFile::StoredConfigFile() : reload( false ), modified_( 0 ) {}

// ToDo: we have to think over... it's a problem with script-inside references
// StoredConfigFile::~StoredConfigFile( )
//{
//  while (!elements_bynum_.empty())
//  {
//    delete ((*elements_bynum_.begin()).second);
//    elements_bynum_.erase( elements_bynum_.begin() );
//  }
//
//  while (!elements_byname_.empty())
//  {
//    delete ((*elements_byname_.begin()).second);
//    elements_byname_.erase( elements_byname_.begin() );
//  }
//}

void StoredConfigFile::load( Clib::ConfigFile& cf )
{
  reload = false;
  modified_ = cf.modified();

  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    ElemRef elemref( new StoredConfigElem( elem ) );

    if ( isdigit( elem.rest()[0] ) )
    {
      unsigned int key = strtoul( elem.rest(), nullptr, 0 );
      elements_bynum_.insert( ElementsByNum::value_type( key, elemref ) );
    }

    std::string key( elem.rest() );
    elements_byname_.insert( ElementsByName::value_type( key, elemref ) );
  }
}

StoredConfigFile::ElemRef StoredConfigFile::findelem( int key )
{
  ElementsByNum::const_iterator itr = elements_bynum_.find( key );
  if ( itr == elements_bynum_.end() )
    return ElemRef( nullptr );
  return ( *itr ).second;
}

StoredConfigFile::ElemRef StoredConfigFile::findelem( const std::string& key )
{
  ElementsByName::const_iterator itr = elements_byname_.find( key );
  if ( itr == elements_byname_.end() )
    return ElemRef( nullptr );
  return ( *itr ).second;
}

int StoredConfigFile::maxintkey() const
{
  if ( elements_bynum_.empty() )
  {
    return 0;
  }

  ElementsByNum::const_iterator itr = elements_bynum_.end();
  --itr;
  return ( *itr ).first;
}

time_t StoredConfigFile::modified() const
{
  return modified_;
}

//  From "[some stuff]" return "some stuff"
std::string extractkey( const std::string& istr )
{
  std::string::size_type vstart = istr.find_first_not_of( " [" );
  std::string::size_type vend = istr.find_last_not_of( "] " );
  return istr.substr( vstart, vend );
}

void StoredConfigFile::load_tus_scp( const std::string& filename )
{
  std::ifstream ifs( filename.c_str() );

  int count = 0;
  ElemRef elemref( new StoredConfigElem() );
  elements_bynum_.insert( ElementsByNum::value_type( count++, elemref ) );

  std::string strbuf;
  bool first_line = true;
  while ( getline( ifs, strbuf ) )
  {
    if ( first_line )
    {
      Clib::remove_bom( &strbuf );
      first_line = false;
    }
    Clib::sanitizeUnicodeWithIso( &strbuf );
    if ( strbuf[0] == '[' )
    {
      elemref.set( new StoredConfigElem() );
      elements_bynum_.insert( ElementsByNum::value_type( count++, elemref ) );
      strbuf = extractkey( strbuf );
      elemref->addprop( "_key", Bscript::bobject_from_string( strbuf, 16 ) );
    }
    // FIXME: skip empty lines and comment lines (duh)
    std::string propname, propvalue;
    Clib::splitnamevalue( strbuf, propname, propvalue );

    if ( propname.empty() || propname.substr( 0, 2 ) == "//" )
      continue;

    Bscript::BObjectImp* newimp = Bscript::bobject_from_string( propvalue, 16 );

    elemref->addprop( propname, newimp );
  }
}

size_t StoredConfigFile::estimateSize() const
{
  size_t size = sizeof( bool )      /* bool reload*/
                + sizeof( time_t ); /* time_t modified_*/

  size += Clib::memsize( elements_byname_,
                         []( const auto& v )
                         {
                           if ( v.get() )
                             return sizeof( ElemRef ) + v->estimateSize();
                           return sizeof( ElemRef );
                         } );
  // both maps share the same ref
  size += Clib::memsize( elements_bynum_ );
  return size;
}

ConfigFileRef FindConfigFile( const std::string& filename, const std::string& allpkgbase )
{
  CfgFiles::iterator itr = Core::configurationbuffer.cfgfiles.find( filename );
  if ( itr != Core::configurationbuffer.cfgfiles.end() )
  {
    if ( ( *itr ).second->reload )  // check cfg file modification?
    {
      struct stat newcfgstat;
      stat( filename.c_str(), &newcfgstat );
      if ( ( *itr ).second->modified() != newcfgstat.st_mtime )
      {
        Core::configurationbuffer.oldcfgfiles.push_back( ( *itr ).first );
        Core::configurationbuffer.cfgfiles.erase( itr );
      }
      else
        return ( *itr ).second;
    }
    else
      return ( *itr ).second;
  }

  try
  {
    if ( !allpkgbase.empty() )
    {
      bool any = false;
      ref_ptr<StoredConfigFile> scfg( new StoredConfigFile() );
      std::string main_cfg = "config/" + allpkgbase + ".cfg";
      if ( Clib::FileExists( main_cfg.c_str() ) )
      {
        Clib::ConfigFile cf_main( main_cfg.c_str() );
        scfg->load( cf_main );
        any = true;
      }
      for ( auto pkg : Plib::systemstate.packages )
      {
        // string pkgfilename = pkg->dir() + allpkgbase + ".cfg";
        std::string pkgfilename = GetPackageCfgPath( pkg, allpkgbase + ".cfg" );
        if ( Clib::FileExists( pkgfilename.c_str() ) )
        {
          Clib::ConfigFile cf( pkgfilename.c_str() );
          scfg->load( cf );
          any = true;
        }
      }
      if ( !any )
        return ConfigFileRef( nullptr );
      Core::configurationbuffer.cfgfiles.insert( CfgFiles::value_type( filename, scfg ) );
      return scfg;
    }

    if ( !Clib::FileExists( filename.c_str() ) )
    {
      if ( Plib::systemstate.config.report_missing_configs )
      {
        DEBUGLOGLN( "Config File {} does not exist.", filename );
      }
      return ConfigFileRef( nullptr );
    }

    Clib::ConfigFile cf( filename.c_str() );

    ref_ptr<StoredConfigFile> scfg( new StoredConfigFile() );
    scfg->load( cf );
    Core::configurationbuffer.cfgfiles.insert( CfgFiles::value_type( filename, scfg ) );
    return scfg;
  }
  catch ( std::exception& ex )
  {
    // There was some weird problem reading the config file.
    DEBUGLOGLN( "An exception was encountered while reading {}: {}", filename, ex.what() );
    return ConfigFileRef( nullptr );
  }
}

ConfigFileRef LoadTusScpFile( const std::string& filename )
{
  if ( !Clib::FileExists( filename.c_str() ) )
  {
    return ConfigFileRef( nullptr );
  }

  ref_ptr<StoredConfigFile> scfg( new StoredConfigFile() );
  scfg->load_tus_scp( filename );
  return scfg;
}

void CreateEmptyStoredConfigFile( const std::string& filename )
{
  ref_ptr<StoredConfigFile> scfg( new StoredConfigFile() );
  Core::configurationbuffer.cfgfiles.insert( CfgFiles::value_type( filename, scfg ) );
}

int UnloadConfigFile( const std::string& filename )
{
  CfgFiles::iterator itr = Core::configurationbuffer.cfgfiles.find( filename );
  if ( itr != Core::configurationbuffer.cfgfiles.end() )
  {
    ( *itr ).second->reload = true;  // check cfg file modification on FindConfigFile

    return ( *itr ).second->count() - 1;
  }

  return -1;
}

#ifdef MEMORYLEAK
void ConfigFiles_log_stuff()
{
  DEBUGLOGLN( "ConfigFiles: {} files loaded and {} files 'removed'",
              Core::configurationbuffer.cfgfiles.size(),
              Core::configurationbuffer.oldcfgfiles.size() );

  LEAKLOG( "{};{};", Core::configurationbuffer.cfgfiles.size(),
           Core::configurationbuffer.oldcfgfiles.size() );
}
#endif
}  // namespace Pol::Core
