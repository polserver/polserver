/** @file
 *
 * @par History
 * - 2010/02/03 Turley:    MethodScript support for mobiles
 */


#include "npctmpl.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include "globals/uvars.h"
#include "item/armor.h"
#include "item/equipmnt.h"
#include "item/weapon.h"
#include "syshookscript.h"
#include <format/format.h>
#include <memory>

namespace Pol
{
namespace Core
{
struct TRANSLATION
{
  const char* name;
  int value;
};

int translate( const std::string& name, TRANSLATION* table )
{
  for ( int i = 0; table[i].name; ++i )
  {
    if ( table[i].name == name )
      return table[i].value;
  }
  fmt::Writer tmp;
  tmp << "Unable to translate value '" << name << "'\n";
  tmp << "  Expected one of the following values:\n";
  for ( int i = 0; table[i].name; ++i )
  {
    tmp << "  " << table[i].name << "\n";
  }
  ERROR_PRINT << tmp.str();
  throw std::runtime_error( "Unable to translate value" );
  return 0;
}

TRANSLATION xlate_align[] = { { "good", NpcTemplate::GOOD },
                              { "neutral", NpcTemplate::NEUTRAL },
                              { "evil", NpcTemplate::EVIL },
                              { nullptr, 0 } };


NpcTemplate::NpcTemplate( const Clib::ConfigElem& elem, const Plib::Package* pkg )
    : intrinsic_weapon( static_cast<Items::UWeapon*>(
          Items::find_intrinsic_equipment( elem.rest(), LAYER_HAND1 ) ) ),
      intrinsic_shield( static_cast<Items::UArmor*>(
          Items::find_intrinsic_equipment( elem.rest(), LAYER_HAND2 ) ) ),
      pkg( pkg ),
      // script( elem.read_string( "SCRIPT" ) ),
      alignment( static_cast<ALIGNMENT>(
          translate( elem.read_string( "ALIGNMENT", "neutral" ), xlate_align ) ) ),
      method_script( nullptr )
{
  if ( pkg == nullptr )
  {
    name = elem.rest();
  }
  else
  {
    if ( elem.rest()[0] == ':' )
    {
      name = elem.rest();
    }
    else
    {
      name = ":" + pkg->name() + ":" + elem.rest();
    }
  }

  if ( elem.has_prop( "MethodScript" ) )
  {
    std::string temp = elem.read_string( "MethodScript" );
    if ( !temp.empty() )
    {
      ExportScript* shs = new ExportScript( pkg, temp );
      if ( shs->Initialize() )
        method_script = shs;
      else
        delete shs;
    }
  }
}

NpcTemplate::~NpcTemplate()
{
  if ( method_script != nullptr )
  {
    delete method_script;
    method_script = nullptr;
  }
}

size_t NpcTemplate::estimateSize() const
{
  size_t size = sizeof( NpcTemplate );
  size += name.capacity();
  if ( method_script != nullptr )
    size += method_script->estimateSize();
  return size;
}

std::shared_ptr<NpcTemplate> create_npc_template( const Clib::ConfigElem& elem,
                                                  const Plib::Package* pkg )
{
  auto tmpl = std::make_shared<NpcTemplate>( elem, pkg );
  gamestate.npc_templates[tmpl->name] = tmpl;
  return tmpl;
}

void load_npc_templates()
{
  if ( Clib::FileExists( "config/npcdesc.cfg" ) )
  {
    Clib::ConfigFile cf( "config/npcdesc.cfg" );
    Clib::ConfigElem elem;
    while ( cf.read( elem ) )
    {
      create_npc_template( elem, nullptr );
    }
  }

  for ( Plib::Packages::iterator itr = Plib::systemstate.packages.begin();
        itr != Plib::systemstate.packages.end(); ++itr )
  {
    Plib::Package* pkg = ( *itr );
    std::string filename = Plib::GetPackageCfgPath( pkg, "npcdesc.cfg" );

    if ( Clib::FileExists( filename.c_str() ) )
    {
      Clib::ConfigFile cf( filename.c_str() );
      Clib::ConfigElem elem;
      while ( cf.read( elem ) )
      {
        create_npc_template( elem, pkg );
      }
    }
  }
}

std::shared_ptr<NpcTemplate> find_npc_template( const Clib::ConfigElem& elem )
{
  auto itr = gamestate.npc_templates.find( elem.rest() );
  if ( itr != gamestate.npc_templates.end() )
  {
    return itr->second;
  }
  else
  {
    const Plib::Package* pkg;
    std::string path;
    if ( Plib::pkgdef_split( elem.rest(), nullptr, &pkg, &path ) )
    {
      return create_npc_template( elem, pkg );
    }
    else
    {
      throw std::runtime_error( std::string( "Error reading NPC template name " ) + elem.rest() );
    }
  }
}
}  // namespace Core
}  // namespace Pol
