/** @file
 *
 * @par History
 * - 2003/07/10 Dave: armordesc expects string zone names
 */


#include "armor.h"

#include <stddef.h>
#include <stdexcept>

#include "../../bscript/bstruct.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
#include "../../clib/cfgelem.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/streamsaver.h"
#include "../../clib/strutil.h"
#include "../../plib/pkg.h"
#include "../../plib/systemstate.h"
#include "../../plib/tiles.h"
#include "../equipdsc.h"
#include "../extobj.h"
#include "../globals/settings.h"
#include "../globals/uvars.h"
#include "../layers.h"
#include "../syshookscript.h"
#include "../uobject.h"
#include "armrtmpl.h"
#include "itemdesc.h"


namespace Pol
{
namespace Mobile
{
const char* zone_to_zone_name( unsigned short zone );
unsigned short layer_to_zone( unsigned short layer );
unsigned short zone_name_to_zone( const char* zname );
}  // namespace Mobile
namespace Items
{
/// Since the constructor is doing some wrong guessing to tell when an armor is a shield,
/// forceShield will force to consider it a shield
ArmorDesc::ArmorDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg,
                      bool forceShield )
    : EquipDesc( objtype, elem, ARMORDESC, pkg ),
      ar( elem.remove_ushort( "AR", 0 ) ),
      zones(),
      on_hit_script( elem.remove_string( "ONHITSCRIPT", "" ), pkg, "scripts/items/" )
{
  std::string coverage;
  while ( elem.remove_prop( "COVERAGE", &coverage ) )
  {
    try
    {
      zones.insert( Mobile::zone_name_to_zone( coverage.c_str() ) );
    }
    catch ( std::runtime_error& )
    {
      std::string tmp = fmt::format( "Error in Objtype {:#x} {}itemdesc.cfg", objtype,
                                     pkg == nullptr ? "config/" :

                                                    pkg->dir() );

      ERROR_PRINTLN( tmp );
      throw;
    }
  }

  if ( zones.empty() )
  {
    // No 'COVERAGE' entries existed.
    // default coverage based on object type/layer
    unsigned short layer = Plib::systemstate.tile[graphic].layer;
    // special case for shields - they effectively have no coverage.
    if ( !forceShield && layer != Core::LAYER_HAND1 && layer != Core::LAYER_HAND2 )
    {
      try
      {
        zones.insert( Mobile::layer_to_zone( layer ) );
      }
      catch ( std::runtime_error& )
      {
        std::string tmp = fmt::format( "Error in Objtype {:#x} {}itemdesc.cfg", objtype,
                                       pkg == nullptr ? "config/" :

                                                      pkg->dir() );
        ERROR_PRINTLN( tmp );
        throw;
      }
    }
  }
}

void ArmorDesc::PopulateStruct( Bscript::BStruct* descriptor ) const
{
  base::PopulateStruct( descriptor );
  descriptor->addMember( "OnHitScript", new Bscript::String( on_hit_script.relativename( pkg ) ) );
  descriptor->addMember( "AR", new Bscript::BLong( ar ) );

  std::unique_ptr<Bscript::ObjArray> arr_zones( new Bscript::ObjArray() );
  std::set<unsigned short>::const_iterator itr;
  for ( itr = zones.begin(); itr != zones.end(); ++itr )
    arr_zones->addElement( new Bscript::String( Mobile::zone_to_zone_name( *itr ) ) );


  if ( arr_zones->ref_arr.size() > 0 )
    descriptor->addMember( "Coverage", arr_zones.release() );
}

size_t ArmorDesc::estimatedSize() const
{
  size_t size = base::estimatedSize() + sizeof( unsigned short ) /*ar*/
                + on_hit_script.estimatedSize() + Clib::memsize( zones );
  return size;
}

UArmor::UArmor( const ArmorDesc& descriptor, const ArmorDesc* permanent_descriptor )
    : Equipment( descriptor, Core::UOBJ_CLASS::CLASS_ARMOR, permanent_descriptor ),
      onhitscript_( descriptor.on_hit_script )
{
}

unsigned short UArmor::ar() const
{
  short ar_mod = this->ar_mod();
  int ar = ar_base() * hp_ / maxhp();
  if ( ar_mod != 0 )
  {
    ar += ar_mod;
  }

  if ( ar < 0 )
    return 0;
  else if ( ar <= USHRT_MAX )
    return static_cast<unsigned short>( ar );
  else
    return USHRT_MAX;
}

unsigned short UArmor::ar_base() const
{
  return ARMOR_TMPL->ar;
}

bool UArmor::covers( unsigned short zlayer ) const
{
  passert( tmpl != nullptr );
  return ARMOR_TMPL->zones.find( zlayer ) != ARMOR_TMPL->zones.end();
}

Item* UArmor::clone() const
{
  UArmor* armor = static_cast<UArmor*>( base::clone() );
  armor->ar_mod( this->ar_mod() );
  armor->onhitscript_ = onhitscript_;
  armor->tmpl = tmpl;
  return armor;
}

void UArmor::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );
  if ( has_ar_mod() )
    sw.add( "AR_mod", ar_mod() );
  if ( tmpl != nullptr && onhitscript_ != ARMOR_TMPL->on_hit_script )
    sw.add( "OnHitScript", onhitscript_.relativename( tmpl->pkg ) );
}

void UArmor::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );
  this->ar_mod( static_cast<s16>( elem.remove_int( "AR_MOD", 0 ) ) );
  set_onhitscript( elem.remove_string( "ONHITSCRIPT", "" ) );
}

void UArmor::set_onhitscript( const std::string& scriptname )
{
  if ( scriptname.empty() )
  {
    onhitscript_.clear();
  }
  else
  {
    onhitscript_.config( scriptname, itemdesc().pkg, "scripts/items/", true );
  }
}
std::set<unsigned short> UArmor::tmplzones()
{
  passert( tmpl != nullptr );
  return ARMOR_TMPL->zones;
}

/// Must be called at startup, validates the intrinsic shield element
void validate_intrinsic_shield_template()
{
  const ItemDesc& id = find_itemdesc( Core::settingsManager.extobj.shield );
  if ( id.save_on_exit )
    throw std::runtime_error( "Intrinsic Shield " +
                              Clib::hexint( Core::settingsManager.extobj.shield ) +
                              " must specify SaveOnExit 0" );

  if ( id.type != ItemDesc::ARMORDESC )
    throw std::runtime_error(
        "An Armor template for Intrinsic Shield is required in itemdesc.cfg" );
}

/// Creates a new intrinsic shield and returns it
/// @param name: the unique shield's name
/// @param elem: the config element to create from
/// @param pkg: the package
UArmor* create_intrinsic_shield( const char* name, Clib::ConfigElem& elem,
                                 const Plib::Package* pkg )
{
  auto tmpl = new ArmorDesc( Core::settingsManager.extobj.shield, elem, pkg, true );
  tmpl->is_intrinsic = true;
  auto armr = new UArmor( *tmpl, tmpl );
  armr->layer = Core::LAYER_HAND2;
  armr->tmpl = tmpl;
  armr->copyprops( tmpl->props );

  Items::register_intrinsic_equipment( name, armr );

  return armr;
}

/// Creates a new intrinic shield for an NPC template and returns it
/// @param elem: The conig element defining the NPC
/// @param pkg: The package
/// @returns The created shield or nullptr if none is defined in the template
UArmor* create_intrinsic_shield_from_npctemplate( Clib::ConfigElem& elem, const Plib::Package* pkg )
{
  std::string tmp;
  if ( elem.remove_prop( "Shield", &tmp ) )
  {
    // Construct an ArmorTemplate for this NPC template.
    Clib::ConfigElem shieldelem;
    shieldelem.set_rest( elem.rest() );
    shieldelem.set_source( elem );
    shieldelem.add_prop( "Objtype", "0xFFFF" );
    shieldelem.add_prop( "Graphic", "1" );
    shieldelem.add_prop( "SaveOnExit", "0" );
    shieldelem.add_prop( "AR", tmp );

    if ( elem.remove_prop( "ShieldMaxHp", &tmp ) )
      shieldelem.add_prop( "MaxHP", tmp );
    else
      shieldelem.add_prop( "MaxHP", "1" );

    if ( elem.remove_prop( "ShieldOnHitScript", &tmp ) )
      shieldelem.add_prop( "OnHitScript", tmp );

    while ( elem.remove_prop( "ShieldCProp", &tmp ) )
      shieldelem.add_prop( "CProp", tmp );

    return create_intrinsic_shield( elem.rest(), shieldelem, pkg );
  }
  else
  {
    return nullptr;
  }
}

size_t UArmor::estimatedSize() const
{
  size_t size = base::estimatedSize() + onhitscript_.estimatedSize();
  return size;
}

bool UArmor::get_method_hook( const char* methodname, Bscript::Executor* ex,
                              Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.armor_method_script.get(), methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}
}  // namespace Items
}  // namespace Pol
