/** @file
 *
 * @par History
 * - 2006/05/10 MuadDib:   Removed unload_npc_templates for non use.
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: elem->type() check will use strlen() now
 */


#include <exception>
#include <iterator>
#include <stdexcept>
#include <string.h>
#include <string>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/clib.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include "globals/uvars.h"
#include "npctmpl.h"
#include <format/format.h>


namespace Pol
{
namespace Core
{
NpcTemplateConfigSource::NpcTemplateConfigSource( const Clib::ConfigFile& cf )
    : _filename( cf.filename() ), _fileline( cf.element_line_start() ){};
NpcTemplateConfigSource::NpcTemplateConfigSource() : _filename( "" ), _fileline( 0 ) {}
size_t NpcTemplateConfigSource::estimateSize() const
{
  return _filename.capacity() + sizeof( unsigned int );
}
void NpcTemplateConfigSource::display_error( const std::string& msg, bool /*show_curline*/,
                                             const Clib::ConfigElemBase* elem, bool error ) const
{
  bool showed_elem_line = false;

  std::string tmp = fmt::format(
      "{} reading configuratiom file {}:\n"
      "\t{}",
      error ? "Error" : "Warning", _filename, msg );

  if ( elem != nullptr )
  {
    if ( strlen( elem->type() ) > 0 )
    {
      fmt::format_to( std::back_inserter( tmp ), "\n\tElement: {} {}", elem->type(), elem->rest() );
      if ( _fileline )
        fmt::format_to( std::back_inserter( tmp ), ", found on line {}", _fileline );
      showed_elem_line = true;
    }
  }

  if ( _fileline && !showed_elem_line )
    fmt::format_to( std::back_inserter( tmp ), "\n\tElement started on line: {}", _fileline );
  ERROR_PRINTLN( tmp );
}


NpcTemplateElem::NpcTemplateElem() {}
NpcTemplateElem::NpcTemplateElem( const Clib::ConfigFile& cf, const Clib::ConfigElem& elem )
    : _source( cf ), _elem( elem )
{
}

void NpcTemplateElem::copyto( Clib::ConfigElem& elem ) const
{
  elem = _elem;
  elem.set_source( &_source );
}

size_t NpcTemplateElem::estimateSize() const
{
  size_t size = sizeof( NpcTemplateElem ) + _source.estimateSize() + _elem.estimateSize();
  return size;
}


bool FindNpcTemplate( const char* template_name, Clib::ConfigElem& elem )
{
  auto itr = gamestate.npc_template_elems.find( template_name );
  if ( itr != gamestate.npc_template_elems.end() )
  {
    const NpcTemplateElem& tm = ( *itr ).second;
    tm.copyto( elem );
    return true;
  }
  else
  {
    return false;
  }
}

// FIXME inefficient.  Templates should be read in once, and reused.
bool FindNpcTemplate( const char* template_name, Clib::ConfigFile& cf, Clib::ConfigElem& elem )
{
  try
  {
    const Plib::Package* pkg;
    std::string npctemplate;
    if ( !Plib::pkgdef_split( template_name, nullptr, &pkg, &npctemplate ) )
      return false;

    std::string filename =
        Plib::GetPackageCfgPath( const_cast<Plib::Package*>( pkg ), "npcdesc.cfg" );

    cf.open( filename.c_str() );
    while ( cf.read( elem ) )
    {
      if ( !elem.type_is( "NpcTemplate" ) )
        continue;

      std::string orig_rest = elem.rest();
      if ( pkg != nullptr )
      {
        std::string newrest = ":" + pkg->name() + ":" + npctemplate;
        elem.set_rest( newrest.c_str() );
      }
      const char* rest = elem.rest();
      if ( rest != nullptr && *rest != '\0' )
      {
        if ( stricmp( orig_rest.c_str(), npctemplate.c_str() ) == 0 )
          return true;
      }
      else
      {
        std::string tname = elem.remove_string( "TemplateName" );
        if ( stricmp( tname.c_str(), npctemplate.c_str() ) == 0 )
          return true;
      }
    }
    return false;
  }
  catch ( const char* msg )
  {
    ERROR_PRINTLN( "NPC Creation ({}) Failed: {}", template_name, msg );
  }
  catch ( std::string& str )
  {
    ERROR_PRINTLN( "NPC Creation ({}) Failed: {}", template_name, str );
  }                                 // egcs has some trouble realizing 'exception' should catch
  catch ( std::runtime_error& re )  // runtime_errors, so...
  {
    ERROR_PRINTLN( "NPC Creation ({}) Failed: {}", template_name, re.what() );
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINTLN( "NPC Creation ({}) Failed: {}", template_name, ex.what() );
  }
#ifndef WIN32
  catch ( ... )
  {
  }
#endif
  return false;
}

void read_npc_templates( Plib::Package* pkg )
{
  std::string filename = GetPackageCfgPath( pkg, "npcdesc.cfg" );
  if ( !Clib::FileExists( filename ) )
    return;

  Clib::ConfigFile cf( filename.c_str() );
  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    if ( elem.type_is( "NpcTemplate" ) )
    {
      // first determine the NPC template name.
      std::string namebase;
      const char* rest = elem.rest();
      if ( rest != nullptr && *rest != '\0' )
      {
        namebase = rest;
      }
      else
      {
        namebase = elem.remove_string( "TemplateName" );
      }
      std::string descname;
      if ( pkg != nullptr )
      {
        descname = ":" + pkg->name() + ":" + namebase;
        elem.set_rest( descname.c_str() );
      }
      else
        descname = namebase;

      gamestate.npc_template_elems[descname] = NpcTemplateElem( cf, elem );
    }
  }
}

void read_npc_templates()
{
  gamestate.npc_template_elems.clear();
  read_npc_templates( nullptr );
  for ( auto& pkg : Plib::systemstate.packages )
  {
    read_npc_templates( pkg );
  }
}
}  // namespace Core
}  // namespace Pol
