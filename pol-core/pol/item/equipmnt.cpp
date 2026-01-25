/** @file
 *
 * @par History
 * - 2007/03/28 Shinigami: added increv() to Equipment::reduce_hp_from_hit
 */


#include "equipmnt.h"

#include <stddef.h>

#include "../../bscript/executor.h"
#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/clib_endian.h"
#include "../../clib/fileutil.h"
#include "../../clib/passert.h"
#include "../../clib/random.h"
#include "../../clib/streamsaver.h"
#include "../../plib/pkg.h"
#include "../../plib/systemstate.h"
#include "../equipdsc.h"
#include "../globals/object_storage.h"
#include "../globals/state.h"
#include "../globals/uvars.h"
#include "../layers.h"
#include "../mobile/charactr.h"
#include "../syshookscript.h"
#include "../tooltips.h"
#include "../ufunc.h"
#include "armor.h"
#include "item.h"
#include "itemdesc.h"
#include "weapon.h"


namespace Pol::Items
{
Equipment::Equipment( const ItemDesc& itemdesc, Core::UOBJ_CLASS uobj_class,
                      const Core::EquipDesc* permanent_descriptor )
    : Item( itemdesc, uobj_class ), tmpl( permanent_descriptor )
{
}

Equipment::~Equipment()
{
  // Every intrinsic equipment has its own local itemdesc element that should be deleted here.
  // Only exception is the wrestling weapon, which should be deferred
  // to the global desctable cleaning.

  if ( is_intrinsic() )
  {
    Core::EquipDesc* ed = const_cast<Core::EquipDesc*>( tmpl );
    if ( !ed->is_pc_intrinsic )
    {
      ed->unload_scripts();

      delete tmpl;
      tmpl = nullptr;
    }
  }
}

Item* Equipment::clone() const
{
  Equipment* eq = static_cast<Equipment*>( base::clone() );

  return eq;
}

void Equipment::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );
}

void Equipment::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );
}

void Equipment::reduce_hp_from_hit()
{
  // Intrinsic equipment does ot get damaged
  if ( tmpl->is_intrinsic )
    return;

  if ( hp_ >= 1 && Clib::random_int( 99 ) == 0 )
  {
    set_dirty();
    --hp_;
    increv();
    if ( isa( Core::UOBJ_CLASS::CLASS_ARMOR ) )
    {
      Mobile::Character* chr = GetCharacterOwner();
      if ( chr != nullptr )
        chr->refresh_ar();
    }
    send_object_cache_to_inrange( this );
  }
}

/// Tells eather an equipment is intrinsic or not
/// Intrinsic equipment is, by example, NPCs "natural" weapon and shield
/// or the PCs wrestling weapon
bool Equipment::is_intrinsic() const
{
  return tmpl->is_intrinsic;
}

size_t Equipment::estimatedSize() const
{
  size_t size = base::estimatedSize() + sizeof( const Core::EquipDesc* ); /*tmpl*/
  if ( is_intrinsic() && !tmpl->is_pc_intrinsic )
    size += tmpl->estimatedSize();
  return size;
}

bool Equipment::get_method_hook( const char* methodname, Bscript::Executor* ex,
                                 Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.equipment_method_script.get(), methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}

/// Looks up for an existing intrinsic equipment and return it or nullptr if not found
Equipment* find_intrinsic_equipment( const std::string& name, u8 layer )
{
  auto itr = Core::gamestate.intrinsic_equipments.find( Core::NameAndLayer( name, layer ) );
  if ( itr == Core::gamestate.intrinsic_equipments.end() )
    return nullptr;
  return itr->second;
}

/// Must be called when a new intrinsic equipment is created
void register_intrinsic_equipment( const std::string& name, Equipment* equip )
{
  equip->inuse( true );

  // during system startup, defer serial allocation in order to avoid clashes with
  // saved items.
  if ( !Core::stateManager.gflag_in_system_startup )
  {
    equip->serial = Core::GetNewItemSerialNumber();
    equip->serial_ext = ctBEu32( equip->serial );
    Core::objStorageManager.objecthash.Insert( equip );
  }

  insert_intrinsic_equipment( name, equip );
}

/// Adds a new intrisinc equipment to the map of known ones
void insert_intrinsic_equipment( const std::string& name, Equipment* equip )
{
  passert_always_r( equip->layer,
                    "Trying to use register equipment without a layer as intrinsic. Please report "
                    "this bug on the forums." );
  passert_always_r(
      equip->is_intrinsic(),
      "Trying to register non-intrinsic equipment. Please report this bug on the forums." );
  Core::gamestate.intrinsic_equipments.insert(
      Core::IntrinsicEquipments::value_type( Core::NameAndLayer( name, equip->layer ), equip ) );
}

/// Deferred allocator for serials during startup, see comments in register_intrinsic_equipment()
void allocate_intrinsic_equipment_serials()
{
  for ( auto& intrinsic_equipment : Core::gamestate.intrinsic_equipments )
  {
    Equipment* eqp = intrinsic_equipment.second;
    eqp->serial = Core::GetNewItemSerialNumber();
    eqp->serial_ext = ctBEu32( eqp->serial );
    Core::objStorageManager.objecthash.Insert( eqp );
  }
}

/// Recreates intrinsic equipment for defined NPCs
/// must be called at startup
void load_npc_intrinsic_equip()
{
  if ( Clib::FileExists( "config/npcdesc.cfg" ) )
  {
    Clib::ConfigFile cf( "config/npcdesc.cfg" );
    Clib::ConfigElem elem;
    while ( cf.read( elem ) )
    {
      Items::create_intrinsic_weapon_from_npctemplate( elem, nullptr );
      Items::create_intrinsic_shield_from_npctemplate( elem, nullptr );
    }
  }
  for ( const auto& pkg : Plib::systemstate.packages )
  {
    std::string filename = Plib::GetPackageCfgPath( pkg, "npcdesc.cfg" );

    if ( Clib::FileExists( filename.c_str() ) )
    {
      Clib::ConfigFile cf( filename.c_str() );
      Clib::ConfigElem elem;
      while ( cf.read( elem ) )
      {
        std::string newrest = ":" + pkg->name() + ":" + std::string( elem.rest() );
        elem.set_rest( newrest.c_str() );

        create_intrinsic_weapon_from_npctemplate( elem, pkg );
        create_intrinsic_shield_from_npctemplate( elem, pkg );
      }
    }
  }
}
}  // namespace Pol::Items
