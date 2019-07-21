/** @file
 *
 * @par History
 * - 2006/03/01 MuadDib:   Altered the decayat_gameclock_ check (to check for != 0).
 * - 2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
 * - 2009/08/06 MuadDib:   Added gotten_by code for items.
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: new_ar not used
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */

#include "item.h"

#include <exception>

#include "../../bscript/berror.h"
#include "../../bscript/eprog.h"
#include "../../bscript/executor.h"
#include "../../clib/cfgelem.h"
#include "../../clib/passert.h"
#include "../../clib/refptr.h"
#include "../../clib/streamsaver.h"
#include "../../plib/clidata.h"
#include "../../plib/mapcell.h"
#include "../../plib/systemstate.h"
#include "../containr.h"
#include "../gameclck.h"
#include "../globals/uvars.h"
#include "../mobile/charactr.h"
#include "../module/uomod.h"
#include "../network/client.h"
#include "../objtype.h"
#include "../polcfg.h"
#include "../proplist.h"
#include "../resource.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../syshookscript.h"
#include "../tooltips.h"
#include "../ufunc.h"
#include "../uoscrobj.h"
#include "itemdesc.h"


namespace Pol
{
namespace Items
{
const u32 Item::SELLPRICE_DEFAULT = UINT_MAX;
const u32 Item::BUYPRICE_DEFAULT = UINT_MAX;

/**
 * This is a pretty good clone. Somewhat inefficient, but does
 * work for derived classes that do not have data.
 * (since it calls Item::create, virtual tables are copied)
 */
Item* Item::clone() const
{
  Item* item = Item::create( objtype_ );
  item->color = color;
  item->graphic = graphic;
  item->x = x;
  item->y = y;
  item->z = z;
  item->realm = realm;
  item->facing = facing;
  item->setamount( amount_ );
  item->layer = layer;
  item->tile_layer = tile_layer;
  item->container = nullptr;  // was container
  item->sellprice_( sellprice_() );
  item->buyprice_( buyprice_() );
  item->newbie( newbie() );
  item->insured( insured() );

  item->invisible( invisible() );  // dave 12-20
  item->movable( movable() );      // dave 12-20
  item->hp_ = hp_;
  item->setQuality( getQuality() );

  item->on_use_script_ = on_use_script_;    // dave 12-20
  item->equip_script_ = equip_script_;      // dave 12-20
  item->unequip_script_ = unequip_script_;  // dave 12-20

  item->name_ = name_;
  item->increv();

  item->copyprops( *this );

  item->saveonexit( saveonexit() );

  item->fire_resist( fire_resist() );
  item->cold_resist( cold_resist() );
  item->energy_resist( energy_resist() );
  item->poison_resist( poison_resist() );
  item->physical_resist( physical_resist() );

  item->fire_damage( fire_damage() );
  item->cold_damage( cold_damage() );
  item->energy_damage( energy_damage() );
  item->poison_damage( poison_damage() );
  item->physical_damage( physical_damage() );
  item->lower_reagent_cost( lower_reagent_cost() );
  item->spell_damage_increase( spell_damage_increase() );
  item->faster_casting( faster_casting() );
  item->faster_cast_recovery( faster_cast_recovery() );
  item->defence_increase( defence_increase() );
  item->defence_increase_cap( defence_increase_cap() );
  item->lower_mana_cost( lower_mana_cost() );
  item->hit_chance( hit_chance() );
  item->fire_resist_cap( fire_resist_cap() );
  item->cold_resist_cap( cold_resist_cap() );
  item->energy_resist_cap( energy_resist_cap() );
  item->poison_resist_cap( poison_resist_cap() );
  item->physical_resist_cap( physical_resist_cap() );
  item->luck( luck() );


  item->maxhp_mod( maxhp_mod() );
  item->name_suffix( name_suffix() );

  item->no_drop( no_drop() );
  return item;
}

std::string Item::name() const
{
  if ( !name_.get().empty() )
  {
    return name_;
  }
  else
  {
    const ItemDesc& id = this->itemdesc();

    if ( id.desc.get().empty() )
      return Plib::tile_desc( graphic );
    else
      return id.desc;
  }
}

const ItemDesc& Item::itemdesc() const
{
  if ( _itemdesc == nullptr )
    _itemdesc = &find_itemdesc( objtype_ );
  return *_itemdesc;
}

/**
 * there are four forms of 'name' in objinfo:
 * - name                   (normal)
 * - name%s                 (percent followed by plural-part, then null-term)
 * - name%s%                (percent followed by plural-part, then percent, then more)
 * - wheat shea%ves/f%      ( '%', plural part, '/', single part, '%', rest )
 *
 * Some examples:
 * - pil%es/e% of hides
 * - banana%s%
 * - feather%s
 *
 * Known bugs:
 * - 1 gold coin displays as "gold coin".  There must be a bit somewhere
 *   that I just don't understand yet.
 */
std::string Item::description() const
{
  std::string suffix = name_suffix();
  if ( specific_name() )
  {
    return Core::format_description( 0, name(), amount_, suffix );  // dave monkeyed with this 2/4/3
  }
  else
  {
    const ItemDesc& id = this->itemdesc();
    if ( id.desc.get().empty() )
    {
      return Core::format_description( Plib::tile_flags( graphic ), Plib::tile_desc( graphic ),
                                       amount_, suffix );
    }
    else
    {
      return Core::format_description( Plib::tile_flags( graphic ), id.desc, amount_, suffix );
    }
  }
}

std::string Item::get_use_script_name() const
{
  return on_use_script_;
}

std::string Item::merchant_description() const
{
  std::string suffix = name_suffix();
  if ( specific_name() )
  {
    return Core::format_description( 0, name(), 1, suffix );
  }
  else
  {
    const ItemDesc& id = this->itemdesc();
    if ( id.desc.get().empty() )
    {
      return Core::format_description( 0, Plib::tile_desc( graphic ), 1, suffix );
    }
    else
    {
      return Core::format_description( 0, id.desc, 1, suffix );
    }
  }
}

u32 Item::sellprice() const
{
  u32 price = sellprice_();
  if ( price == SELLPRICE_DEFAULT )
    return itemdesc().vendor_sells_for;
  return price;
}
void Item::sellprice( u32 value )
{
  sellprice_( value );
}

// Dave add buyprice() 11/28. Dont know wtf getbuyprice() is trying to do.
//  Dave, getbuyprice() was trying to return false if the vendor wasn't interested in buying.
//        it would return true if the vendor was interested in buying.
//   -Eric
u32 Item::buyprice() const
{
  u32 price = buyprice_();
  if ( price == BUYPRICE_DEFAULT )
    return itemdesc().vendor_buys_for;
  return price;
}

void Item::buyprice( u32 value )
{
  buyprice_( value );
}

bool Item::getbuyprice( u32& bp ) const
{
  bp = buyprice();
  if ( bp > 0 )
    return true;
  else
    return false;
}

Core::UObject* Item::owner()
{
  if ( container != nullptr )
    return container->self_as_owner();
  else
    return nullptr;
}

const Core::UObject* Item::owner() const
{
  if ( container != nullptr )
    return container->self_as_owner();
  else
    return nullptr;
}

Core::UObject* Item::toplevel_owner()
{
  Item* item = this;
  while ( item->container != nullptr )
    item = item->container;

  return item;
}

const Core::UObject* Item::toplevel_owner() const
{
  const Item* item = this;
  while ( item->container != nullptr )
    item = item->container;

  return item;
}

const char* Item::classname() const
{
  return "Item";
}

bool Item::default_movable() const
{
  if ( itemdesc().movable == ItemDesc::DEFAULT )
    return ( ( Plib::tile_flags( graphic ) & Plib::FLAG::MOVABLE ) != 0 );
  else
    return itemdesc().movable ? true : false;
}

bool Item::default_invisible() const
{
  return itemdesc().invisible;
}

bool Item::default_newbie() const
{
  return itemdesc().newbie;
}

bool Item::default_insured() const
{
  return itemdesc().insured;
}

/// Returns current insurance value and resets it to false
bool Item::use_insurance()
{
  if ( insured() )
  {
    set_dirty();
    insured( false );
    return true;
  }
  return false;
}

bool Item::no_drop() const
{
  return flags_.get( Core::OBJ_FLAGS::NO_DROP );
}

void Item::no_drop( bool newvalue )
{
  flags_.change( Core::OBJ_FLAGS::NO_DROP, newvalue );
}

bool Item::default_no_drop() const
{
  return itemdesc().no_drop;
}

unsigned short Item::maxhp() const
{
  int maxhp = itemdesc().maxhp + maxhp_mod();

  if ( maxhp < 1 )
    return 1;
  else if ( maxhp <= USHRT_MAX )
    return static_cast<u16>( maxhp );
  else
    return USHRT_MAX;
}

void Item::printProperties( Clib::StreamWriter& sw ) const
{
  using namespace fmt;

  base::printProperties( sw );

  short maxhp_mod_ = maxhp_mod();
  std::string suffix = name_suffix();

  if ( amount_ != 1 )
    sw() << "\tAmount\t" << amount_ << pf_endl;

  if ( layer != 0 )
    sw() << "\tLayer\t" << (int)layer << pf_endl;

  if ( movable() != default_movable() )
    sw() << "\tMovable\t" << movable() << pf_endl;

  if ( invisible() != default_invisible() )
    sw() << "\tInvisible\t" << invisible() << pf_endl;

  if ( container != nullptr )
    sw() << "\tContainer\t0x" << hex( container->serial ) << pf_endl;

  if ( !on_use_script_.get().empty() )
    sw() << "\tOnUseScript\t" << on_use_script_.get() << pf_endl;

  if ( equip_script_ != itemdesc().equip_script )
    sw() << "\tEquipScript\t" << equip_script_.get() << pf_endl;

  if ( unequip_script_ != itemdesc().unequip_script )
    sw() << "\tUnequipScript\t" << unequip_script_.get() << pf_endl;

  if ( decayat_gameclock_ != 0 )
    sw() << "\tDecayAt\t" << decayat_gameclock_ << pf_endl;

  if ( has_sellprice_() )
    sw() << "\tSellPrice\t" << sellprice_() << pf_endl;
  if ( has_buyprice_() )
    sw() << "\tBuyPrice\t" << buyprice_() << pf_endl;

  if ( newbie() != default_newbie() )
    sw() << "\tNewbie\t" << newbie() << pf_endl;

  if ( insured() != default_insured() )
    sw() << "\tInsured\t" << insured() << pf_endl;
  // new prop stuff
  if ( has_fire_resist() )
    sw() << "\tFireResist\t" << fire_resist().value << pf_endl;
  if ( has_cold_resist() )
    sw() << "\tColdResist\t" << cold_resist().value << pf_endl;
  if ( has_energy_resist() )
    sw() << "\tEnergyResist\t" << energy_resist().value << pf_endl;
  if ( has_poison_resist() )
    sw() << "\tPoisonResist\t" << poison_resist().value << pf_endl;
  if ( has_physical_resist() )
    sw() << "\tPhysicalResist\t" << physical_resist().value << pf_endl;

  if ( has_fire_damage() )
    sw() << "\tFireDamage\t" << fire_damage().value << pf_endl;
  if ( has_cold_damage() )
    sw() << "\tColdDamage\t" << cold_damage().value << pf_endl;
  if ( has_energy_damage() )
    sw() << "\tEnergyDamage\t" << energy_damage().value << pf_endl;
  if ( has_poison_damage() )
    sw() << "\tPoisonDamage\t" << poison_damage().value << pf_endl;
  if ( has_physical_damage() )
    sw() << "\tPhysicalDamage\t" << physical_damage().value << pf_endl;
  if ( has_lower_reagent_cost() )
    sw() << "\tLowerReagentCost\t" << lower_reagent_cost().value << pf_endl;
  if ( has_spell_damage_increase() )
    sw() << "\tSpellDamageIncrease\t" << spell_damage_increase().value << pf_endl;
  if ( has_faster_casting() )
    sw() << "\tFasterCasting\t" << faster_casting().value << pf_endl;
  if ( has_faster_cast_recovery() )
    sw() << "\tFasterCastRecovery\t" << faster_cast_recovery().value << pf_endl;
  if ( has_defence_increase() )
    sw() << "\tDefenceIncrease\t" << defence_increase().value << pf_endl;
  if ( has_defence_increase_cap() )
    sw() << "\tDefenceIncreaseCap\t" << defence_increase_cap().value << pf_endl;
  if ( has_lower_mana_cost() )
    sw() << "\tLowerManaCost\t" << lower_mana_cost().value << pf_endl;
  if ( has_fire_resist_cap() )
    sw() << "\tFireResistCap\t" << fire_resist_cap().value << pf_endl;
  if ( has_cold_resist_cap() )
    sw() << "\tColdResistCap\t" << cold_resist_cap().value << pf_endl;
  if ( has_energy_resist_cap() )
    sw() << "\tEnergyResistCap\t" << energy_resist_cap().value << pf_endl;
  if ( has_physical_resist_cap() )
    sw() << "\tPhysicalResistCap\t" << physical_resist_cap().value << pf_endl;
  if ( has_poison_resist_cap() )
    sw() << "\tPoisonResistCap\t" << poison_resist_cap().value << pf_endl;
  if ( has_luck() )
    sw() << "\tLuck\t" << luck().value << pf_endl;
  // end new prop stuf
  if ( maxhp_mod_ )
    sw() << "\tMaxHp_mod\t" << maxhp_mod_ << pf_endl;
  if ( hp_ != itemdesc().maxhp )
    sw() << "\tHp\t" << hp_ << pf_endl;
  double quali = getQuality();
  if ( quali != getItemdescQuality() )
    sw() << "\tQuality\t" << quali << pf_endl;
  if ( !suffix.empty() )
    sw() << "\tNameSuffix\t" << suffix << pf_endl;
  if ( no_drop() != default_no_drop() )
    sw() << "\tNoDrop\t" << no_drop() << pf_endl;
}

void Item::printDebugProperties( Clib::StreamWriter& sw ) const
{
  base::printDebugProperties( sw );
}

void Item::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );

  // Changed from Valid Color Mask to cfg mask in ssopt.
  color &= Core::settingsManager.ssopt.item_color_mask;

  amount_ = elem.remove_ushort( "AMOUNT", 1 );
  layer = static_cast<unsigned char>( elem.remove_ushort( "LAYER", 0 ) );
  movable( elem.remove_bool( "MOVABLE", default_movable() ) );
  invisible( elem.remove_bool( "INVISIBLE", default_invisible() ) );

  // NOTE, container is handled specially - it is extracted by the creator.

  on_use_script_ = elem.remove_string( "ONUSESCRIPT", "" );
  equip_script_ = elem.remove_string( "EQUIPSCRIPT", equip_script_.get().c_str() );
  unequip_script_ = elem.remove_string( "UNEQUIPSCRIPT", unequip_script_.get().c_str() );

  decayat_gameclock_ = elem.remove_ulong( "DECAYAT", 0 );
  sellprice_( elem.remove_ulong( "SELLPRICE", SELLPRICE_DEFAULT ) );
  buyprice_( elem.remove_ulong( "BUYPRICE", BUYPRICE_DEFAULT ) );

  // buyprice used to be read in with remove_int (which was wrong).
  // the UINT_MAX values used to be written out (which was wrong).
  // when UINT_MAX is read in by atoi, it returned 2147483647 (0x7FFFFFFF)
  // correct for this.
  if ( buyprice_() == 2147483647 )
    buyprice_( BUYPRICE_DEFAULT );
  newbie( elem.remove_bool( "NEWBIE", default_newbie() ) );
  insured( elem.remove_bool( "INSURED", default_insured() ) );
  hp_ = elem.remove_ushort( "HP", itemdesc().maxhp );
  setQuality( elem.remove_double( "QUALITY", itemdesc().quality ) );

  maxhp_mod( static_cast<s16>( elem.remove_int( "MAXHP_MOD", 0 ) ) );
  name_suffix( elem.remove_string( "NAMESUFFIX", "" ) );
  no_drop( elem.remove_bool( "NODROP", default_no_drop() ) );

  s16 value = static_cast<s16>( elem.remove_int( "FIRERESIST", 0 ) );
  if ( value != 0 )
    fire_resist( fire_resist().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "COLDRESIST", 0 ) );
  if ( value != 0 )
    cold_resist( cold_resist().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "ENERGYRESIST", 0 ) );
  if ( value != 0 )
    energy_resist( energy_resist().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "POISONRESIST", 0 ) );
  if ( value != 0 )
    poison_resist( poison_resist().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "PHYSICALRESIST", 0 ) );
  if ( value != 0 )
    physical_resist( physical_resist().setAsValue( value ) );

  value = static_cast<s16>( elem.remove_int( "FIREDAMAGE", 0 ) );
  if ( value != 0 )
    fire_damage( fire_damage().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "COLDDAMAGE", 0 ) );
  if ( value != 0 )
    cold_damage( cold_damage().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "ENERGYDAMAGE", 0 ) );
  if ( value != 0 )
    energy_damage( energy_damage().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "POISONDAMAGE", 0 ) );
  if ( value != 0 )
    poison_damage( poison_damage().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "PHYSICALDAMAGE", 0 ) );
  if ( value != 0 )
    physical_damage( physical_damage().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "DEFENCEINCREASE", 0 ) );
  if ( value != 0 )
    defence_increase( defence_increase().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "DEFENCEINCREASECAP", 0 ) );
  if ( value != 0 )
    defence_increase_cap( defence_increase_cap().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "LOWERMANACOST", 0 ) );
  if ( value != 0 )
    lower_mana_cost( lower_mana_cost().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "HITCHANCE", 0 ) );
  if ( value != 0 )
    hit_chance( hit_chance().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "FIRERESISTCAP", 0 ) );
  if ( value != 0 )
    fire_resist_cap( fire_resist_cap().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "COLDRESISTCAP", 0 ) );
  if ( value != 0 )
    cold_resist_cap( cold_resist_cap().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "ENERGYRESISTCAP", 0 ) );
  if ( value != 0 )
    energy_resist_cap( energy_resist_cap().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "POISONRESISTCAP", 0 ) );
  if ( value != 0 )
    poison_resist_cap( poison_resist_cap().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "PHYSICALRESISTCAP", 0 ) );
  if ( value != 0 )
    physical_resist_cap( physical_resist_cap().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "LOWERREAGENTCOST", 0 ) );
  if ( value != 0 )
    lower_reagent_cost( lower_reagent_cost().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "SPELLDAMAGEINCREASE", 0 ) );
  if ( value != 0 )
    spell_damage_increase( spell_damage_increase().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "FASTERCASTING", 0 ) );
  if ( value != 0 )
    faster_casting( faster_casting().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "FASTERCASTRECOVERY", 0 ) );
  if ( value != 0 )
    faster_cast_recovery( faster_cast_recovery().setAsValue( value ) );
  value = static_cast<s16>( elem.remove_int( "LUCK", 0 ) );
  if ( value != 0 )
    luck( luck().setAsValue( value ) );
}

void Item::builtin_on_use( Network::Client* client )
{
  Core::send_sysmessage( client, "I can't think of a way to use that." );
}

void Item::double_click( Network::Client* client )
{
  const ItemDesc& itemdesc = this->itemdesc();

  if ( itemdesc.requires_attention &&
       ( client->chr->skill_ex_active() || client->chr->casting_spell() ) )
  {
    Core::send_sysmessage( client, "I am already doing something else." );
    return;
  }

  if ( itemdesc.requires_attention && client->chr->hidden() )
    client->chr->unhide();

  ref_ptr<Bscript::EScriptProgram> prog;

  if ( !on_use_script_.get().empty() )
  {
    Core::ScriptDef sd( on_use_script_, nullptr, "" );
    prog = find_script2( sd,
                         true,  // complain if not found
                         Plib::systemstate.config.cache_interactive_scripts );
  }
  else if ( !itemdesc.on_use_script.empty() )
  {
    prog = find_script2( itemdesc.on_use_script, true,
                         Plib::systemstate.config.cache_interactive_scripts );
  }

  if ( prog.get() != nullptr )
  {
    if ( client->chr->start_itemuse_script( prog.get(), this, itemdesc.requires_attention ) )
      return;
    // else log the fact?
  }
  else
  {
    builtin_on_use( client );
  }
}


unsigned short Item::get_senditem_amount() const
{
  return amount_;
}

bool Item::setlayer( unsigned char in_layer )
{
  if ( Plib::tilelayer( graphic ) == in_layer )
  {
    layer = in_layer;
    return true;
  }
  else
  {
    return false;
  }
}

bool Item::stackable() const
{
  return ( Plib::tile_flags( graphic ) & Plib::FLAG::STACKABLE ) ? true : false;
}

void Item::setamount( u16 amount )
{
  set_dirty();
  int oldweight = weight();

  if ( amount > amount_ )  // adding stuff
    remove_resources( objtype_, amount - amount_ );
  else
    return_resources( objtype_, amount_ - amount );

  amount_ = amount;
  int newweight = weight();
  if ( container )
    container->add_bulk( 0, newweight - oldweight );

  increv();
  send_object_cache_to_inrange( this );
}

void Item::subamount( u16 amount_subtract )
{
  setamount( amount_ - amount_subtract );
  increv();
}

void Item::add_to_self( Item*& item )
{
#ifdef PERGON
  ct_merge_stacks_pergon(
      item );  // Pergon: Re-Calculate Property CreateTime after Merging of two Stacks
#endif

  setamount( amount_ + item->amount_ );

  if ( !item->newbie() )
    newbie( false );

  if ( !item->insured() )
    insured( false );

  item->destroy();
  item = nullptr;
}

#ifdef PERGON
// Pergon: Re-Calculate Property CreateTime after Merging of two Stacks
void Item::ct_merge_stacks_pergon( Item*& item_sub )
{
  int time_self, time_sub, time;
  std::string value_self, value_sub;

  // get "ct" of first stack - if error, init
  if ( getprop( "ct", value_self ) )
  {
    Bscript::BObject imp( Bscript::BObjectImp::unpack( value_self.c_str() ) );
    if ( imp.impptr() != nullptr && imp->isa( Bscript::BObjectImp::OTLong ) )
      time_self = static_cast<Bscript::BLong*>( imp.impptr() )->value();
    else
      time_self = Core::read_gameclock();
  }
  else
    time_self = Core::read_gameclock();

  // get "ct" of second stack - if error, init
  if ( item_sub->getprop( "ct", value_sub ) )
  {
    Bscript::BObject imp( Bscript::BObjectImp::unpack( value_sub.c_str() ) );
    if ( imp.impptr() != nullptr && imp->isa( Bscript::BObjectImp::OTLong ) )
      time_sub = static_cast<Bscript::BLong*>( imp.impptr() )->value();
    else
      time_sub = Core::read_gameclock();
  }
  else
    time_sub = Core::read_gameclock();

  // calculate new time
  if ( time_self < time_sub )
  {
    double factor = ( item_sub->amount_ * 1.0 ) / ( amount_ + item_sub->amount_ );
    time = int( ( time_sub - time_self ) * factor + time_self );
  }
  else if ( time_self > time_sub )
  {
    double factor = ( amount_ * 1.0 ) / ( amount_ + item_sub->amount_ );
    time = int( ( time_self - time_sub ) * factor + time_sub );
  }
  else
    time = time_self;

  setprop( "ct", "i" + Clib::tostring( time ) );
  increv();
}

// Pergon: Re-Calculate Property CreateTime after Adding Items to a Stack
void Item::ct_merge_stacks_pergon( u16 amount_sub )
{
  int time_self, time_sub, time;
  std::string value_self;

  // get "ct" of first stack - if error, init
  if ( getprop( "ct", value_self ) )
  {
    Bscript::BObject imp( Bscript::BObjectImp::unpack( value_self.c_str() ) );
    if ( imp.impptr() != nullptr && imp->isa( Bscript::BObjectImp::OTLong ) )
      time_self = static_cast<Bscript::BLong*>( imp.impptr() )->value();
    else
      time_self = Core::read_gameclock();
  }
  else
    time_self = Core::read_gameclock();

  // set "ct" of second new stack
  time_sub = Core::read_gameclock();

  // calculate new time
  if ( time_self != time_sub )
  {
    double factor = ( amount_sub * 1.0 ) / ( amount_ + amount_sub );
    time = int( ( time_sub - time_self ) * factor + time_self );
  }
  else
    time = time_self;

  setprop( "ct", "i" + Clib::tostring( time ) );
  increv();
}
#endif

bool Item::can_add_to_self( unsigned short amount, bool force_stacking ) const
{
  if ( !force_stacking && !stackable() )
    return false;

  unsigned int amount1 = (unsigned int)amount_;
  unsigned int amount2 = (unsigned int)amount;

  if ( ( amount1 + amount2 ) > this->itemdesc().stack_limit )
    return false;

  if ( container != nullptr )
  {
    int more_weight = weight_of( amount_ + amount ) - weight_of( amount_ );
    if ( more_weight > USHRT_MAX /*std::numeric_limits<unsigned short>::max()*/ )
      return false;
    return container->can_add( static_cast<unsigned short>( more_weight ) );
  }

  return true;
}

bool Item::can_add_to_self( const Item& item, bool force_stacking )
    const  // dave 1/26/03 totally changed this function to handle the cprop comparisons.
{
  bool res =
      ( ( item.objtype_ == objtype_ ) && ( item.newbie() == newbie() ) &&
        ( item.insured() == insured() ) && ( item.graphic == graphic ) && ( item.color == color ) &&
        ( !inuse() ) && ( can_add_to_self( item.amount_, force_stacking ) ) );
  if ( res == true )
  {
    // NOTE! this logic is copied in Item::has_only_default_cprops(), so make any necessary changes
    // there too
    Core::PropertyList myprops( getprops() );  // make a copy :(
    myprops -= itemdesc().ignore_cprops;
    myprops -= Core::gamestate.Global_Ignore_CProps;

    Core::PropertyList yourprops( item.getprops() );  // make a copy :(
    yourprops -= item.itemdesc().ignore_cprops;
    yourprops -= Core::gamestate.Global_Ignore_CProps;

    res = ( myprops == yourprops );
  }
  return res;
}

/**
 * @author dave 5/11/3
 */
bool Item::has_only_default_cprops( const ItemDesc* compare ) const
{
  if ( compare == nullptr )
    compare = &( itemdesc() );
  // logic same as Item::can_add_to_self()
  Core::PropertyList myprops( getprops() );  // make a copy :(
  myprops -= itemdesc().ignore_cprops;
  myprops -= Core::gamestate.Global_Ignore_CProps;

  Core::PropertyList yourprops( compare->props );  // make a copy :(
  yourprops -= compare->ignore_cprops;
  yourprops -= Core::gamestate.Global_Ignore_CProps;

  return ( myprops == yourprops );
}

bool Item::amount_to_remove_is_partial( u16 this_item_new_amount ) const
{
  return ( ( this_item_new_amount < amount_ ) &&  // less than what we have
           ( amount_ > 1 ) &&                     // we are a stack
           ( this_item_new_amount > 0 ) );        // if new size 0, assume whole stack
}

/**
 * Slice_stacked_item
 *
 * Used when a portion of a stack is dragged.
 * Given ITEM1, with, for example, amount 75.
 * If 25 units are dragged off, this function will get called
 * with 25.  What happens then:
 * - ITEM2 is created, with Amount 50. It it placed where ITEM1 used to be.
 * - ITEM1's Amount is set to 25.
 *
 * In short, we go from:
 * ITEM1 (Amount:75)
 * TO
 * ITEM1 (Amount:25) -under cursor, ITEM2(Amount:50) where ITEM1 was.
 */
Item* Item::slice_stacked_item( u16 this_item_new_amount )
{
  Item* new_item = clone();
  if ( new_item != nullptr )
  {
    new_item->setamount( new_item->amount_ - this_item_new_amount );
    setamount( this_item_new_amount );
  }
  return new_item;
}

Item* Item::remove_part_of_stack( u16 amount_to_remove )
{
  Item* new_item = clone();
  if ( new_item != nullptr )
  {
    new_item->setamount( amount_to_remove );
    subamount( amount_to_remove );
  }
  return new_item;
}


void Item::set_use_script( const std::string& scriptname )
{
  set_dirty();
  on_use_script_ = scriptname;
}

bool Item::setgraphic( u16 newgraphic )
{
  /// Can't set the graphic of an equipped item, unless the new graphic has the same layer
  if ( layer && layer != Plib::tilelayer( newgraphic ) )
  {
    return false;
  }

  if ( graphic <= Plib::systemstate.config.max_tile_id &&
       newgraphic <= Plib::systemstate.config.max_tile_id )
  {
    set_dirty();
    graphic = newgraphic;
    height = Plib::tileheight( graphic );
    tile_layer = Plib::tilelayer( graphic );

    /// Update facing on graphic change
    const ItemDesc& id = this->itemdesc();

    if ( id.facing == 127 )
      facing = tile_layer;
    else
      facing = id.facing;

    increv();
    update_item_to_inrange( this );
    return true;
  }
  else
  {
    return false;
  }
}

bool Item::setcolor( u16 newcolor )
{
  /// return false if the color is invalid (high nibble set)
  bool res = true;
  u16 theMask = (u16)Core::settingsManager.ssopt.item_color_mask;
  if ( ( newcolor & ( ~theMask ) ) != 0 )
    res = false;

  set_dirty();
  newcolor &= ( theMask );

  if ( color != newcolor )
  {
    color = newcolor;
    on_color_changed();
  }

  return res;
}

void Item::on_color_changed()
{
  update_item_to_inrange( this );
}

void Item::on_movable_changed()
{
  update_item_to_inrange( this );
}

void Item::on_invisible_changed()
{
  update_item_to_inrange( this );
}

void Item::setfacing( u8 newfacing )
{
  /// allow 0-127 (all but MSB)
  if ( newfacing > 127 )
    newfacing = 0;

  if ( facing != newfacing )
  {
    facing = newfacing;
    on_facing_changed();
  }
}

void Item::on_facing_changed()
{
  update_item_to_inrange( this );
}

void Item::extricate()
{
  if ( container != nullptr )
  {
    // hmm, a good place for a virtual?
    if ( Core::IsCharacter( container->serial ) )
    {
      Mobile::Character* chr = chr_from_wornitems( container );
      passert_always( chr != nullptr );  // PRODFIXME linux-crash
      passert_always( chr->is_equipped( this ) );

      chr->unequip( this );  // FIXME: should run unequip script
    }
    else
    {
      container->remove( this );  // FIXME: should probably call can/onRemove for the container
    }
  }
}

void Item::spill_contents( Multi::UMulti* /*multi*/ ) {}

unsigned int Item::weight_of( unsigned short amount ) const
{
  const ItemDesc& id = this->itemdesc();
  unsigned int amt = amount;
  amt *= id.weightmult;
  if ( id.weightdiv != 1 && id.weightdiv != 0 )
  {
    amt = ( amt + id.weightdiv - 1 ) / id.weightdiv;
  }
  return amt;
}

unsigned int Item::weight() const
{
  return weight_of( amount_ );
}
unsigned int Item::item_count() const
{
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Decay-related functions
/////////////////////////////////////////////////////////////////////////////


void Item::set_decay_after( unsigned int seconds )
{
  set_dirty();
  // Why alter it, if it should not have it to begin with??
  if ( decayat_gameclock_ != 0 )
  {
    decayat_gameclock_ = Core::read_gameclock() + seconds;
  }
}

bool Item::can_decay() const
{
  return !inuse() && ( movable() || ( objtype_ == UOBJ_CORPSE ) ) && decayat_gameclock_;
}

bool Item::should_decay( unsigned int gameclock ) const
{
  return can_decay() && ( gameclock > decayat_gameclock_ );
}

void Item::restart_decay_timer()
{
  // Why restart it, if it shouldn't have it to begin with??
  if ( decayat_gameclock_ != 0 )
  {
    unsigned int dtime = itemdesc().decay_time * 60;
    if ( should_decay( Core::read_gameclock() + dtime ) )
    {
      set_decay_after( dtime );
    }
  }
}

void Item::disable_decay()
{
  set_dirty();
  decayat_gameclock_ = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Equip-Script related functions
/////////////////////////////////////////////////////////////////////////////
bool Item::has_equip_script() const
{
  return !equip_script_.get().empty();
}

Bscript::BObjectImp* Item::run_equip_script( Mobile::Character* who, bool startup )
{
  Core::ScriptDef sd;

  try
  {
    sd.config( equip_script_, itemdesc().pkg, "scripts/control/" );
  }
  catch ( std::exception& ex )
  {
    return new Bscript::BError( std::string( "Script descriptor error" ) + ex.what() );
  }

  try
  {
    return Core::run_script_to_completion( sd, new Module::ECharacterRefObjImp( who ),
                                           new Module::EItemRefObjImp( this ),
                                           new Bscript::BLong( startup ? 1 : 0 ) );
  }
  catch ( std::exception& ex )
  {
    return new Bscript::BError( std::string( "Script execution error" ) + ex.what() );
  }
}

Bscript::BObjectImp* Item::run_unequip_script( Mobile::Character* who )
{
  Core::ScriptDef sd;

  try
  {
    sd.config( unequip_script_, itemdesc().pkg, "scripts/control/" );
  }
  catch ( std::exception& ex )
  {
    return new Bscript::BError( std::string( "Script descriptor error: " ) + ex.what() );
  }

  try
  {
    return Core::run_script_to_completion( sd, new Module::ECharacterRefObjImp( who ),
                                           new Module::EItemRefObjImp( this ) );
  }
  catch ( std::exception& ex )
  {
    return new Bscript::BError( std::string( "Script execution error:" ) + ex.what() );
  }
}


bool Item::check_equip_script( Mobile::Character* chr, bool startup )
{
  if ( has_equip_script() )
  {
    Bscript::BObject obj( run_equip_script( chr, startup ) );
    return obj.isTrue();
  }
  else
  {
    return true;
  }
}

bool Item::check_unequip_script()
{
  if ( !unequip_script_.get().empty() && container != nullptr &&
       Core::IsCharacter( container->serial ) )
  {
    Mobile::Character* chr = chr_from_wornitems( container );
    passert_always( chr != nullptr );
    passert_always( chr->is_equipped( this ) );

    Bscript::BObject obj( run_unequip_script( chr ) );
    return obj.isTrue();
  }
  else
  {
    return true;
  }
}

void preload_test_scripts( const std::string& script_ecl )
{
  Core::ScriptDef sd;
  sd.quickconfig( "scripts/misc/" + script_ecl );
  if ( sd.exists() )
  {
    find_script2( sd, true, true );
  }
  for ( const auto& pkg : Plib::systemstate.packages )
  {
    sd.quickconfig( pkg, script_ecl );
    if ( sd.exists() )
    {
      find_script2( sd, true, true );
    }
  }
}
void preload_test_scripts()
{
  preload_test_scripts( "equiptest.ecl" );
  preload_test_scripts( "unequiptest.ecl" );
}

bool Item::check_test_scripts( Mobile::Character* chr, const std::string& script_ecl, bool startup )
{
  Core::ScriptDef sd;
  sd.quickconfig( "scripts/misc/" + script_ecl );
  this->inuse( true );
  if ( script_loaded( sd ) )
  {
    bool res =
        Core::call_script( sd, new Module::ECharacterRefObjImp( chr ),
                           new Module::EItemRefObjImp( this ), new Bscript::BLong( startup ) );
    this->inuse( false );
    if ( !res )
      return false;
  }
  for ( const auto& pkg : Plib::systemstate.packages )
  {
    sd.quickconfig( pkg, script_ecl );
    if ( script_loaded( sd ) )
    {
      bool res =
          Core::call_script( sd, new Module::ECharacterRefObjImp( chr ),
                             new Module::EItemRefObjImp( this ), new Bscript::BLong( startup ) );
      this->inuse( false );
      if ( !res )
        return false;
    }
  }

  this->inuse( false );
  return true;
}

bool Item::check_equiptest_scripts( Mobile::Character* chr, bool startup )
{
  return check_test_scripts( chr, "equiptest.ecl", startup );
}

bool Item::check_unequiptest_scripts( Mobile::Character* chr )
{
  return check_test_scripts( chr, "unequiptest.ecl", false );
}

bool Item::check_unequiptest_scripts()
{
  if ( container != nullptr && Core::IsCharacter( container->serial ) )
  {
    Mobile::Character* chr = chr_from_wornitems( container );
    passert_always( chr != nullptr );
    passert_always( chr->is_equipped( this ) );

    return check_unequiptest_scripts( chr );
  }
  else
  {
    return true;
  }
}

/**
 * Shortcut function to get a pointer to the owner character
 *
 * @author DAVE 11/17
 */
Mobile::Character* Item::GetCharacterOwner()
{
  UObject* top_level_item = toplevel_owner();
  if ( top_level_item->isa( Core::UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    Mobile::Character* chr_owner =
        Core::chr_from_wornitems( static_cast<Core::UContainer*>( top_level_item ) );
    if ( chr_owner != nullptr )
    {
      return chr_owner;
    }
    else
      return nullptr;
  }
  else
    return nullptr;
}

const char* Item::target_tag() const
{
  return "item";
}

double Item::getItemdescQuality() const
{
  return itemdesc().quality;
}

Core::UOExecutor* Item::uoexec_control()
{
  if ( process() != nullptr )
    return &process()->uoexec;

  return nullptr;
}

double Item::getQuality() const
{
  return quality();
}
void Item::setQuality( double value )
{
  quality( value );
}

bool Item::get_method_hook( const char* methodname, Bscript::Executor* ex,
                            Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.item_method_script.get(), methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}

// Event notifications

bool Item::is_visible_to_me( const Mobile::Character* chr ) const
{
  if ( chr == nullptr )
    return false;
  if ( chr->realm != this->realm )
    return false;  // noone can see across different realms.
  if ( !chr->logged_in() )
    return false;

  // Unless the chr is offline or in a different realm,
  // items can see anyone (I don't want to bother with privs now...)
  return true;
}

void Pol::Items::Item::inform_leftarea( Mobile::Character* wholeft )
{
  Core::UOExecutor* ex = uoexec_control();
  if ( ex == nullptr || !ex->listens_to( Core::EVID_LEFTAREA ) )
    return;

  if ( pol_distance( wholeft, this ) > ex->area_size )
    return;

  if ( Core::settingsManager.ssopt.event_visibility_core_checks && !is_visible_to_me( wholeft ) )
    return;

  ex->signal_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, wholeft ) );
}

void Pol::Items::Item::inform_enteredarea( Mobile::Character* whoentered )
{
  Core::UOExecutor* ex = uoexec_control();
  if ( ex == nullptr || !ex->listens_to( Core::EVID_ENTEREDAREA ) )
    return;

  if ( pol_distance( whoentered, this ) > ex->area_size )
    return;

  if ( Core::settingsManager.ssopt.event_visibility_core_checks && !is_visible_to_me( whoentered ) )
    return;

  ex->signal_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, whoentered ) );
}
void Pol::Items::Item::inform_moved( Mobile::Character* moved )
{
  Core::UOExecutor* ex = uoexec_control();
  if ( ex == nullptr || !ex->listens_to( Core::EVID_ENTEREDAREA | Core::EVID_LEFTAREA ) )
    return;

  if ( Core::settingsManager.ssopt.event_visibility_core_checks && !is_visible_to_me( moved ) )
    return;

  const bool are_inrange =
      ( abs( x - moved->x ) <= ex->area_size ) && ( abs( y - moved->y ) <= ex->area_size );

  const bool were_inrange =
      ( abs( x - moved->lastx ) <= ex->area_size ) && ( abs( y - moved->lasty ) <= ex->area_size );

  if ( are_inrange && !were_inrange && ex->listens_to( Core::EVID_ENTEREDAREA ) )
  {
    ex->signal_event( new Module::SourcedEvent( Core::EVID_ENTEREDAREA, moved ) );
  }
  else if ( !are_inrange && were_inrange && ex->listens_to( Core::EVID_LEFTAREA ) )
  {
    ex->signal_event( new Module::SourcedEvent( Core::EVID_LEFTAREA, moved ) );
  }
}

}  // namespace Items
}  // namespace Pol
