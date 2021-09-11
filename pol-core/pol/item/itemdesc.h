/** @file
 *
 * @par History
 */


#ifndef ITEMDESC_H
#define ITEMDESC_H

#include <map>
#include <string>
#include <vector>

#include "../../clib/boostutils.h"
#include "../../clib/rawtypes.h"
#include "../dice.h"
#include "../proplist.h"
#include "../scrdef.h"
#include "../uobject.h"
#include "base/vector.h"

namespace Pol
{
namespace Clib
{
class ConfigElem;
}  // namespace Clib
namespace Plib
{
class Package;
}  // namespace Plib
}  // namespace Pol

namespace Pol
{
namespace Bscript
{
class BStruct;
}
namespace Core
{
class ExportScript;
class ResourceDef;
}  // namespace Core
namespace Items
{
struct ResourceComponent
{
  Core::ResourceDef* rd;
  unsigned amount;

  ResourceComponent( const std::string& rname, unsigned amount );
};

class ItemDesc
{
public:
  enum Type
  {
    ITEMDESC,
    CONTAINERDESC,
    DOORDESC,
    WEAPONDESC,
    ARMORDESC,
    BOATDESC,
    HOUSEDESC,
    SPELLBOOKDESC,
    SPELLSCROLLDESC,
    MAPDESC
  };

  static ItemDesc* create( Clib::ConfigElem& elem, const Plib::Package* pkg );

  ItemDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg );
  explicit ItemDesc( Type type );
  virtual ~ItemDesc();
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const;
  virtual size_t estimatedSize() const;
  std::string objtype_description() const;
  bool default_movable() const;

  Type type;

  const Plib::Package* pkg;
  std::string objtypename;

  u32 objtype;
  u16 graphic;
  u16 color;
  // u16 weight;
  u8 facing;
  int weightmult;
  int weightdiv;
  boost_utils::object_name_flystring desc;
  std::string tooltip;
  Core::ScriptDef walk_on_script;
  Core::ScriptDef on_use_script;
  boost_utils::script_name_flystring equip_script;
  boost_utils::script_name_flystring unequip_script;
  Core::ScriptDef control_script;
  Core::ScriptDef create_script;
  Core::ScriptDef destroy_script;
  bool requires_attention;
  bool lockable;
  unsigned int vendor_sells_for;
  unsigned int vendor_buys_for;
  unsigned decay_time;
  enum Movable : u8
  {
    UNMOVABLE,
    MOVABLE,
    DEFAULT
  } movable;
  unsigned short doubleclick_range;
  bool use_requires_los;  // DAVE 11/24
  bool ghosts_can_use;    // DAVE 11/24
  bool can_use_while_paralyzed;
  bool can_use_while_frozen;
  bool newbie;
  bool insured;
  bool invisible;
  bool decays_on_multis;
  bool blocks_casting_if_in_hand;
  bool no_drop;
  unsigned short base_str_req;
  unsigned short stack_limit;
  double quality;
  unsigned short multiid;
  unsigned short maxhp;
  unsigned short lower_reag_cost;
  unsigned short spell_damage_increase;
  unsigned short faster_casting;
  unsigned short faster_cast_recovery;
  unsigned short defence_increase;
  unsigned short defence_increase_cap;
  unsigned short lower_mana_cost;
  unsigned short hit_chance;
  unsigned short resist_fire_cap;
  unsigned short resist_cold_cap;
  unsigned short resist_energy_cap;
  unsigned short resist_physical_cap;
  unsigned short resist_poison_cap;
  unsigned short defence_increase_mod;
  unsigned short defence_increase_cap_mod;
  unsigned short lower_mana_cost_mod;
  unsigned short hit_chance_mod;
  unsigned short resist_fire_cap_mod;
  unsigned short resist_cold_cap_mod;
  unsigned short resist_energy_cap_mod;
  unsigned short resist_physical_cap_mod;
  unsigned short resist_poison_cap_mod;
  unsigned short lower_reagent_cost_mod;
  unsigned short spell_damage_increase_mod;
  unsigned short faster_casting_mod;
  unsigned short faster_cast_recovery_mod;
  unsigned short luck;
  unsigned short luck_mod;
  unsigned short swing_speed_increase;
  unsigned short swing_speed_increase_mod;


  Core::Dice resist_dice;

  Core::Resistances element_resist;
  Core::ElementDamages element_damage;

  std::vector<ResourceComponent> resources;

  Core::PropertyList props;
  std::set<std::string> ignore_cprops;  // dave added 1/26/3

  Core::ExportScript* method_script;

  bool save_on_exit;

  void unload_scripts();
};

class ContainerDesc : public ItemDesc
{
  typedef ItemDesc base;

public:
  ContainerDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg );
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  virtual ~ContainerDesc(){};
  virtual size_t estimatedSize() const override;

  // string name;
  // u16 objtype;
  u16 gump;
  u16 minx, maxx;
  u16 miny, maxy;

  u16 max_weight;
  u16 max_items;

  u8 max_slots;

  bool no_drop_exception;

  Core::ScriptDef can_insert_script;
  Core::ScriptDef on_insert_script;
  Core::ScriptDef can_remove_script;
  Core::ScriptDef on_remove_script;
};

class DoorDesc final : public ItemDesc
{
  typedef ItemDesc base;

public:
  DoorDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg );
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  virtual ~DoorDesc(){};
  virtual size_t estimatedSize() const override;
  Core::Vec2d mod;
  u16 open_graphic;
};

class SpellbookDesc final : public ContainerDesc
{
  typedef ContainerDesc base;

public:
  SpellbookDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg );
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  virtual ~SpellbookDesc(){};
  virtual size_t estimatedSize() const override;

  std::string spelltype;
};

class SpellScrollDesc final : public ItemDesc
{
  typedef ItemDesc base;

public:
  SpellScrollDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg );
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  virtual ~SpellScrollDesc(){};
  virtual size_t estimatedSize() const override;

  std::string spelltype;
};

class MultiDesc : public ItemDesc
{
  typedef ItemDesc base;

public:
  MultiDesc( u32 objtype, Clib::ConfigElem& elem, Type type, const Plib::Package* pkg );
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  virtual ~MultiDesc(){};
  virtual size_t estimatedSize() const override;
};

class BoatDesc final : public MultiDesc
{
  typedef MultiDesc base;

public:
  BoatDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg );
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  virtual ~BoatDesc(){};
  virtual size_t estimatedSize() const override;
};

class HouseDesc final : public MultiDesc
{
  typedef MultiDesc base;

public:
  HouseDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg );
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  virtual ~HouseDesc(){};
  virtual size_t estimatedSize() const override;
};

class MapDesc final : public ItemDesc
{
  typedef ItemDesc base;

public:
  MapDesc( u32 objtype, Clib::ConfigElem& elem, const Plib::Package* pkg );
  virtual void PopulateStruct( Bscript::BStruct* descriptor ) const override;
  virtual ~MapDesc(){};
  virtual size_t estimatedSize() const override;
  bool editable;
};


const ItemDesc& find_itemdesc( unsigned int objtype );
const ContainerDesc& find_container_desc( u32 objtype );
const DoorDesc& fast_find_doordesc( u32 objtype );
const MultiDesc& find_multidesc( u32 objtype );

bool has_itemdesc( u32 objtype );

unsigned short getgraphic( unsigned int objtype );
unsigned short getcolor( unsigned int objtype );

unsigned int get_objtype_byname( const char* name );
unsigned int get_objtype_from_string( const std::string& str );
bool objtype_is_lockable( u32 objtype );

void load_itemdesc( Clib::ConfigElem& elem );

const ItemDesc* CreateItemDescriptor( Bscript::BStruct* itemdesc_struct );
}  // namespace Items
}  // namespace Pol
#endif
