/** @file
 *
 * @par History
 * - 2003/12/24 Dave:      on_poison_changed() changed to keep life bars from disappearing when you
 * poison someone.
 * - 2005/03/09 Shinigami: Added Prop Delay_Mod [ms] for WeaponDelay (see schedule_attack too)
 * - 2005/06/01 Shinigami: Added Walking_Mounted and Running_Mounted movecosts
 * - 2005/09/14 Shinigami: Character::resurrect() - Vital.regen_while_dead implemented
 * - 2005/10/14 Shinigami: fixed missing init of Character::dblclick_wait
 * - 2005/11/23 MuadDib:    Added warmode_wait object for characters.
 * - 2005/11/25 MuadDib:   Added realm check to is_visible_to_me.
 * - 2005/12/06 MuadDib:   Added uclang member for storing UC language from client.
 * - 2006/03/10 MuadDib:   Added NoCombat support to checking of justice region.
 * - 2006/05/04 MuadDib:   Removed get_legal_item for no use.
 * - 2006/05/04 MuadDib:   SkillValue() removed for no use.
 * - 2006/05/16 Shinigami: UOBJ_*MALE_GHOST renamed to UOBJ_HUMAN_*MALE_GHOST
 *                         added Prop Race (RACE_* Constants) to support Elfs
 *                         Character::die(), Character::doors_block() and Character::resurrect()
 * updated
 * - 2008/07/08 Turley:    get_flag1() changed to show WarMode of other player again
 *                         get_flag1_aos() removed
 * - 2009/01/14 Nando:     setgraphic() changed to allow graphics up to 2048 (0x800).
 * - 2009/02/01 MuadDib:   Resistance storage added.
 * - 2009/02/25 MuadDib:   on_poison_changed() added UOKR Status bar update for poisoned. Booyah!
 * - 2009/07/20 MuadDib:   Slot checks added to Character::Die()
 * - 2009/07/25 MuadDib:   equippable() now checks if a twohanded is intrinsic or not also.
 * Intrinsic gets ignored
 * - 2009/07/31 Turley:    added check for cmbtcfg::send_swing_packet & reset_swing_onturn
 * - 2009/08/04 MuadDib:   calc_vital_stuff() now checks to see if a vital changed, before using
 * tell_vital_changed()
 * - 2009/08/06 MuadDib:   Addeed gotten_by code for items.
 * - 2009/08/07 MuadDib:   Added new Corpse Layer code to character Die() to put equipped items on
 * correct layer with
 *                         corpse. Ignores items from pack, because in death we only want the items
 * it had equipped in
 *                         life, showing up on the corpse when dead.
 * - 2009/08/09 MuadDib:   on_poison_changed() rewritten for better KR poison support.
 * - 2009/08/16 MuadDib:   fix for die() where checking corpse and not item for slot_index().
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: corpseSlot not used
 *                         STLport-5.2.1 fix: init order changed of party_can_loot_,
 * party_decline_timeout_ and skillcap_
 * - 2009/08/28 Turley:    Crashfix for Character::on_poison_changed()
 * - 2009/09/03 MuadDib:   Changed combat related ssopt stuff to combat_config.
 *                         Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 * - 2009/09/09 Turley:    ServSpecOpt CarryingCapacityMod as * modifier for
 * Character::carrying_capacity()
 * - 2009/09/15 MuadDib:   Cleanup from registered houses on destroy
 *                         u32 registered_house added to store serial of registered multi.
 *                         Multi registration/unregistration support added.
 * - 2009/09/06 Turley:    Changed Version checks to bitfield client->ClientType
 * - 2009/09/18 MuadDib:   Adding save/load of registered house serial
 * - 2009/09/22 MuadDib:   Rewrite for Character/NPC to use ar(), ar_mod(), ar_mod(newvalue)
 * virtuals.
 * - 2009/09/22 Turley:    Added DamagePacket support & repsys param to applydamage
 * - 2009/10/14 Turley:    new priv canbeheardasghost
 * - 2009/10/14 Turley:    Added char.deaf() methods & char.deafened member
 * - 2009/10/17 Turley:    PrivUpdater for "seehidden", "seeghosts", "seeinvisitems" and "invul" -
 * Tomi
 *                         fixed "all" priv
 *                         PrivUpdater class cleanup, removed duplicate stuff
 * - 2009/10/22 Turley:    added OuchHook call if lastz-z>21 (clientside value)
 * - 2009/11/16 Turley:    added NpcPropagateEnteredArea()/inform_enteredarea() for event on
 * resurrection
 * - 2009/11/19 Turley:    lightlevel now supports endless duration - Tomi
 * - 2009/11/20 Turley:    RecalcVitals can update single Attributes/Vitals - based on Tomi
 * - 2009/11/26 Turley:    Syshook CanDie(mobile)
 * - 2009/11/30 Turley:    fixed calc_single_vital doesnt check changed maximum value
 * - 2009/12/02 Turley:    added gargoyle & face support
 * - 2009/12/03 Turley:    fixed client>=7 poison/flying flag, basic flying support
 * - 2010/01/14 Turley:    AttackWhileFrozen check
 * - 2010/01/15 Turley:    (Tomi) priv runwhilestealth
 * - 2010/01/22 Turley:    Speedhack Prevention System
 * - 2011/11/12 Tomi:      added extobj.mount and extobj.secure_trade_container
 * - 2012/02/06 MuadDib:   Added serial check at root of Character::get_from_ground to make sure not
 * trying to move orphaned items.
 *                         In loop, if an found is an orphan it logs it and skips rest of individual
 * itr to ensure movement not attempted.
 */


#include "pol_global_config.h"

#include "charactr.h"

#include <stdlib.h>
#include <string>

#include "../../bscript/executor.h"
#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/clib.h"
#include "../../clib/clib_endian.h"
#include "../../clib/esignal.h"
#include "../../clib/fileutil.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/random.h"
#include "../../clib/stlutil.h"
#include "../../clib/streamsaver.h"
#include "../../plib/clidata.h"
#include "../../plib/mapcell.h"
#include "../../plib/systemstate.h"
#include "../accounts/account.h"
#include "../accounts/accounts.h"
#include "../checkpnt.h"
#include "../cmbtcfg.h"
#include "../cmdlevel.h"
#include "../containr.h"
#include "../dice.h"
#include "../extobj.h"
#include "../fnsearch.h"
#include "../globals/settings.h"
#include "../globals/state.h"
#include "../globals/uvars.h"
#include "../guilds.h"
#include "../item/armor.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../item/weapon.h"
#include "../item/wepntmpl.h"
#include "../layers.h"
#include "../mdelta.h"
#include "../mkscrobj.h"
#include "../module/uomod.h"
#include "../movecost.h"
#include "../multi/customhouses.h"
#include "../multi/house.h"
#include "../multi/multi.h"
#include "../multi/multidef.h"
#include "../network/cgdata.h"
#include "../network/client.h"
#include "../network/cliface.h"
#include "../network/packetdefs.h"
#include "../network/packethelper.h"
#include "../network/packets.h"
#include "../network/pktdef.h"
#include "../objtype.h"
#include "../party.h"
#include "../polclass.h"
#include "../polsig.h"
#include "../polvar.h"
#include "../profile.h"
#include "../realms/WorldChangeReasons.h"
#include "../realms/realm.h"
#include "../schedule.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../sfx.h"
#include "../skilladv.h"
#include "../spelbook.h"
#include "../statmsg.h"
#include "../syshook.h"
#include "../syshookscript.h"
#include "../ufunc.h"
#include "../ufuncstd.h"
#include "../uobjcnt.h"
#include "../uoexec.h"
#include "../uoscrobj.h"
#include "../uworld.h"
#include "../vital.h"
#include "attribute.h"
#include "corpse.h"
#include "privupdater.h"
#include "regions/guardrgn.h"
#include "regions/miscrgn.h"
#include "regions/musicrgn.h"
#include "wornitems.h"

#ifdef _MSC_VER
#pragma warning( disable : 4505 )  // unreferenced local function has been removed
#endif

namespace Pol
{
namespace Core
{
void cancel_trade( Mobile::Character* chr1 );
}
namespace Mobile
{
unsigned short layer_to_zone( unsigned short layer )
{
  for ( unsigned short zone = 0; zone < Core::gamestate.armorzones.size(); ++zone )
  {
    for ( unsigned i = 0; i < Core::gamestate.armorzones[zone].layers.size(); ++i )
    {
      if ( Core::gamestate.armorzones[zone].layers[i] == layer )
        return zone;
    }
  }
  ERROR_PRINT << "Couldn't find an Armor Zone in armrzone.cfg for layer " << layer << "\n";
  throw std::runtime_error( "Configuration file error" );
}

const char* zone_to_zone_name( unsigned short zone )
{
  return Core::gamestate.armorzones[zone].name.c_str();
}

unsigned short zone_name_to_zone( const char* zname )
{
  for ( unsigned short zone = 0; zone < Core::gamestate.armorzones.size(); ++zone )
  {
    if ( stricmp( Core::gamestate.armorzones[zone].name.c_str(), zname ) == 0 )
    {
      return zone;
    }
  }
  ERROR_PRINT << "Couldn't find an armrzone.cfg config elem named '" << zname << "'\n";

  throw std::runtime_error( "Configuration file error" );
}
void load_armor_zones()
{
  if ( !Clib::FileExists( "config/armrzone.cfg" ) )
  {
    if ( Plib::systemstate.config.loglevel > 1 )
      INFO_PRINT << "File config/armrzone.cfg not found, skipping.\n";
    return;
  }
  Clib::ConfigFile cf( "config/armrzone.cfg" );
  Clib::ConfigElem elem;

  Core::gamestate.armor_zone_chance_sum = 0;
  Core::gamestate.armorzones.clear();

  while ( cf.read( elem ) )
  {
    // armorzones.push_back( ArmorZone( elem ) );
    Core::ArmorZone az;
    az.name = elem.remove_string( "NAME" );
    az.chance = static_cast<double>( elem.remove_ushort( "CHANCE" ) ) / 100.0;
    unsigned short in_layer;
    while ( elem.remove_prop( "LAYER", &in_layer ) )
    {
      if ( in_layer < Core::LOWEST_LAYER || in_layer > Core::HIGHEST_LAYER )
      {
        ERROR_PRINT << "ArmorZone " << az.name << ": Layer " << in_layer << " is out of range.\n"
                    << "Valid range is " << Core::LOWEST_LAYER << " to " << Core::HIGHEST_LAYER
                    << "\n";
        throw std::runtime_error( "Configuration file error" );
      }
      az.layers.push_back( in_layer );
    }

    Core::gamestate.armorzones.push_back( az );
    Core::gamestate.armor_zone_chance_sum += az.chance;
  }
}

void unload_armor_zones()
{
  Core::gamestate.armorzones.clear();
  Core::gamestate.armor_zone_chance_sum = 0;
}


Character::Character( u32 objtype, Core::UOBJ_CLASS uobj_class )
    : UObject( objtype, uobj_class ),
      // NPC
      // EQUIPMENT / ITEMS
      weapon( Core::gamestate.wrestling_weapon ),
      shield( nullptr ),
      armor_( Core::gamestate.armorzones.size() ),
      wornitems( new Core::WornItemsContainer ),  // default objtype is in containr.cpp,
                                                  // WornItemsContainer class
      gotten_item_source( GOTTEN_ITEM_ON_GROUND ),
      remote_containers_(),
      // MOVEMENT
      dir( 0 ),
      gradual_boost( 0 ),
      lastx( 0 ),
      lasty( 0 ),
      lastz( 0 ),
      move_reason( OTHER ),
      movemode( Plib::MOVEMODE_LAND ),
      // COMBAT
      warmode_wait( 0 ),
      ar_( 0 ),
      opponent_( nullptr ),
      opponent_of(),
      swing_timer_start_clock_( 0 ),
      swing_task( nullptr ),
      // ATTRIBUTES / VITALS
      disable_regeneration_until( 0 ),
      attributes( Core::gamestate.numAttributes ),
      vitals( Core::gamestate.numVitals ),
      // REPUTATION
      aggressor_to_(),
      lawfully_damaged_(),
      criminal_until_( 0 ),
      repsys_task_( nullptr ),
      to_be_reportable_(),
      reportable_(),
      // GUILD
      // PARTY
      party_decline_timeout_( nullptr ),
      // SECURE TRADING
      trading_cont(),
      trading_with( nullptr ),
      // SCRIPT
      tcursor2( nullptr ),
      menu( nullptr ),
      on_menu_selection( nullptr ),
      on_popup_menu_selection( nullptr ),
      script_ex( nullptr ),
      spell_task( nullptr ),
      // CLIENT
      client( nullptr ),
      uclang( "enu" ),
      _last_textcolor( 0 ),
      // PRIVS SETTINGS STATUS
      cmdlevel_( 0 ),
      concealed_( 0 ),
      stealthsteps_( 0 ),
      mountedsteps_( 0 ),
      privs(),
      settings(),
      cached_settings(),
      mob_flags_(),
      // SERIALIZATION
      // CREATION
      created_at( 0 ),
      // BUFFS
      buffs_(),
      // MISC
      acct( nullptr ),
      registered_house( 0 ),
      truecolor( 0 ),
      trueobjtype( 0 ),
      // Note, Item uses the named constructor idiom, but here, it is not used.
      // this is probably okay, but something to keep in mind.
      gender( Plib::GENDER_MALE ),
      race( Plib::RACE_HUMAN ),
      last_corpse( 0 )
{
  logged_in( true );  // so initialization scripts etc can see

  height = Core::settingsManager.ssopt
               .default_character_height;  // this gets overwritten in UObject::readProperties!
  wornitems->chr_owner = this;             // FIXME, dangerous.

  set_caps_to_default();

  // vector
  refresh_cached_settings( false );
  ++Core::stateManager.uobjcount.ucharacter_count;
}

Character::~Character()
{
  if ( acct.get() )
  {
    for ( int i = 0; i < Plib::systemstate.config.character_slots; i++ )
    {
      if ( acct->get_character( i ) == this )
      {
        acct->clear_character( i );
      }
    }
  }
  acct.clear();

  if ( client && ( client->chr == this ) )
    client->chr = nullptr;
  client = nullptr;

  // It might be nice to do this only when the system isn't shutting down...
  // if (!opponent_of.empty())
  //{
  //  Clib::Log( "Destroying character with nonempty opponent_of! (But cleaning up..)\n" );
  //}

  removal_cleanup();

  // clean up wornitems, so it can be reaped by the objecthash later
  wornitems->destroy();

  // clean up trade container if it exists
  if ( trading_cont != nullptr )
    trading_cont->destroy();

  if ( repsys_task_ != nullptr )
    repsys_task_->cancel();

  if ( party_decline_timeout_ != nullptr )
    party_decline_timeout_->cancel();

  --Core::stateManager.uobjcount.ucharacter_count;
}

void Character::removal_cleanup()
{
  clear_opponent_of();

  /* This used to be a call to set_opponent(nullptr),
     which was slick,
     but that was sending disengage events, which were
     trying to resurrect this object. (C++)
     */
  if ( opponent_ != nullptr )
  {
    opponent_->opponent_of.erase( this );
    //    This is cleanup, wtf we doing trying to send highlights?!
    //    opponent_->send_highlight();
    //    opponent_->schedule_attack();
    opponent_ = nullptr;
  }

  if ( swing_task != nullptr )
    swing_task->cancel();

  disconnect_cleanup();
}

void Character::disconnect_cleanup()
{
  if ( is_trading() )
    Core::cancel_trade( this );

  tcursor2 = nullptr;

  stop_skill_script();
  on_loggoff_party( this );
}

bool Character::logged_in() const
{
  return mob_flags_.get( MOB_FLAGS::LOGGED_IN );
}

void Character::logged_in( bool newvalue )
{
  mob_flags_.change( MOB_FLAGS::LOGGED_IN, newvalue );
}

bool Character::connected() const
{
  return mob_flags_.get( MOB_FLAGS::CONNECTED );
}

void Character::connected( bool newvalue )
{
  mob_flags_.change( MOB_FLAGS::CONNECTED, newvalue );
}

bool Character::has_active_client() const
{
  return ( client != nullptr && client->isActive() );
}

bool Character::has_active_prompt() const
{
  return ( client != nullptr && client->gd != nullptr &&
           ( client->gd->prompt_uniemod || client->gd->prompt_uoemod ) );
}

bool Character::has_active_gump() const
{
  return ( client != nullptr && client->gd != nullptr && !client->gd->gumpmods.empty() );
}

bool Character::has_active_textentry() const
{
  return ( client != nullptr && client->gd != nullptr && client->gd->textentry_uoemod != nullptr );
}

bool Character::is_house_editing() const
{
  return ( client != nullptr && client->gd != nullptr && client->gd->custom_house_serial != 0 );
}

void Character::clear_gotten_item()
{
  auto item = gotten_item();
  if ( item != nullptr )
  {
    gotten_item( nullptr );
    item->inuse( false );
    if ( connected() )
      Core::send_item_move_failure( client, MOVE_ITEM_FAILURE_UNKNOWN );
    undo_get_item( this, item );
  }
}

void Character::destroy()
{
  stop_skill_script();
  if ( registered_house > 0 )
  {
    Multi::UMulti* multi = Core::system_find_multi( registered_house );
    if ( multi != nullptr )
    {
      multi->unregister_object( (UObject*)this );
    }
    registered_house = 0;
  }
  base::destroy();
}

void Character::stop_skill_script()
{
  if ( script_ex != nullptr )
  {
    if ( script_ex->survive_attached_disconnect )
    {
      /*
      We only want to resume those scripts that are paused indefinitely (eg. Target()).
      Scripts that will eventually resume (eg. Sleep()) or those that are in debugging
      state remain untouched.
      */
      if ( script_ex->in_hold_list() == Core::HoldListType::NOTIMEOUT_LIST )
      {
        script_ex->revive();
      }
    }
    else
    {
      // this will force the execution engine to stop running this script immediately
      // dont delete the executor here, since it could currently run
      script_ex->seterror( true );
      script_ex->revive();
      if ( script_ex->in_debugger_holdlist() )
        script_ex->revive_debugged();
    }
  }
}

///
/// A Mobile's weight is 10 stones + the weight of their equipment.
///
unsigned int Character::weight() const
{
  unsigned int wt = 10 + wornitems->weight();
  if ( has_gotten_item() )
    wt += gotten_item()->weight();
  if ( trading_cont.get() )
    wt += trading_cont->weight();
  return wt;
}

///
/// A Mobile's carrying capacity is (40 + 3.5*STR +
/// chr.carrying_capacity_mod)*ssopt.carrying_capacity_mod stones.
///
unsigned short Character::carrying_capacity() const
{
  return static_cast<u16>( floor( ( 40 + strength() * 7 / 2 + carrying_capacity_mod() ) *
                                  Core::settingsManager.ssopt.carrying_capacity_mod ) );
}

int Character::charindex() const
{
  if ( acct == nullptr )
    return -1;

  for ( int i = 0; i < Plib::systemstate.config.character_slots; i++ )
  {
    if ( acct->get_character( i ) == this )
      return i;
  }

  POLLOG_INFO << "Warning: Can't find charidx for Character 0x" << fmt::hexu( serial ) << "\n";
  return -1;
}


void Character::printProperties( Clib::StreamWriter& sw ) const
{
  using namespace fmt;

  if ( acct != nullptr )
  {
    sw() << "\tAccount\t" << acct->name() << pf_endl;
    sw() << "\tCharIdx\t" << charindex() << pf_endl;
  }

  base::printProperties( sw );

  if ( cmdlevel_ )
  {
    sw() << "\tCmdLevel\t" << Core::gamestate.cmdlevels[cmdlevel_].name << pf_endl;
  }
  if ( concealed_ )
  {
    sw() << "\tConcealed\t" << int( concealed_ ) << pf_endl;
  }
  sw() << "\tTrueColor\t0x" << hex( truecolor ) << pf_endl;
  sw() << "\tTrueObjtype\t0x" << hex( trueobjtype ) << pf_endl;

  if ( registered_house )
    sw() << "\tRegisteredHouse\t0x" << hex( registered_house ) << pf_endl;

  sw() << "\tGender\t" << static_cast<int>( gender ) << pf_endl;
  sw() << "\tRace\t" << static_cast<int>( race ) << pf_endl;

  if ( dead() )
    sw() << "\tDead\t" << static_cast<int>( dead() ) << pf_endl;

  if ( mountedsteps_ )
    sw() << "\tMountedSteps\t" << static_cast<unsigned int>( mountedsteps_ ) << pf_endl;

  if ( hidden() )
    sw() << "\tHidden\t" << static_cast<int>( hidden() ) << pf_endl;

  if ( frozen() )
    sw() << "\tFrozen\t" << static_cast<int>( frozen() ) << pf_endl;

  if ( has_movement_cost() )
  {
    auto movecost_value = movement_cost();
    if ( movecost_value.walk != Core::MovementCostMod::DEFAULT.walk )
      sw() << "\tMovementWalkMod\t" << static_cast<double>( movecost_value.walk ) << pf_endl;
    if ( movecost_value.run != Core::MovementCostMod::DEFAULT.run )
      sw() << "\tMovementRunMod\t" << static_cast<double>( movecost_value.run ) << pf_endl;
    if ( movecost_value.walk_mounted != Core::MovementCostMod::DEFAULT.walk_mounted )
      sw() << "\tMovementWalkMountedMod\t" << static_cast<double>( movecost_value.walk_mounted )
           << pf_endl;
    if ( movecost_value.run_mounted != Core::MovementCostMod::DEFAULT.run_mounted )
      sw() << "\tMovementRunMountedMod\t" << static_cast<double>( movecost_value.run_mounted )
           << pf_endl;
  }
  if ( has_carrying_capacity_mod() )
    sw() << "\tCarryingCapacityMod\t" << static_cast<int>( carrying_capacity_mod() ) << pf_endl;


  // output Attributes
  for ( Attribute* pAttr : Core::gamestate.attributes )
  {
    const AttributeValue& av = attribute( pAttr->attrid );
    short lock = av.lock();
    unsigned cap = av.cap();

    if ( av.base() || lock ||
         cap != pAttr->default_cap )  // it kind of floods the file... but... :/ (Nando)
    {
      unsigned ones = av.base() / 10;
      unsigned tenths = av.base() % 10;
      sw() << "\t" << pAttr->name << "\t" << ones;
      if ( tenths )
        sw() << "." << tenths;

      if ( cap != pAttr->default_cap )
      {
        unsigned cap_ones = cap / 10;
        unsigned cap_tenths = cap % 10;

        sw() << ":" << cap_ones;
        if ( tenths )
          sw() << "." << cap_tenths;
      }

      if ( lock )
        sw() << ";" << lock;

      sw() << pf_endl;
    }
  }

  // output Vitals
  for ( Core::Vital* pVital : Core::gamestate.vitals )
  {
    const VitalValue& vv = vital( pVital->vitalid );
    if ( vv.current_ones() )
    {
      sw() << "\t" << pVital->name << "\t" << vv.current_ones() << pf_endl;
    }
  }

  if ( has_skillstatcap() )
  {
    auto cap_value = skillstatcap();
    if ( cap_value.statcap != Core::SkillStatCap::DEFAULT.statcap )
      sw() << "\tStatcap\t" << static_cast<int>( cap_value.statcap ) << pf_endl;
    if ( cap_value.skillcap != Core::SkillStatCap::DEFAULT.skillcap )
      sw() << "\tSkillcap\t" << static_cast<int>( cap_value.skillcap ) << pf_endl;
  }

  if ( has_followers() )
  {
    auto followers_value = followers();
    if ( followers_value.followers_max != Core::ExtStatBarFollowers::DEFAULT.followers_max )
      sw() << "\tFollowersMax\t" << static_cast<int>( followers_value.followers_max ) << pf_endl;
    if ( followers_value.followers != Core::ExtStatBarFollowers::DEFAULT.followers )
      sw() << "\tFollowers\t" << static_cast<int>( followers_value.followers ) << pf_endl;
  }
  if ( has_tithing() )
    sw() << "\tTithing\t" << static_cast<int>( tithing() ) << pf_endl;


  if ( movemode != Plib::MOVEMODE_LAND )
    sw() << "\tMoveMode\t" << encode_movemode( movemode ) << pf_endl;

  if ( !privs.empty() )
  {
    sw() << "\tPrivs\t" << privs.extract() << pf_endl;
  }
  if ( !settings.empty() )
  {
    sw() << "\tSettings\t" << settings.extract() << pf_endl;
  }

  sw() << "\tCreatedAt\t" << created_at << pf_endl;

  if ( has_squelched_until() )
    sw() << "\tSquelchedUntil\t" << squelched_until() << pf_endl;
  if ( has_deafened_until() )
    sw() << "\tDeafenedUntil\t" << deafened_until() << pf_endl;

  if ( has_title_prefix() )
    sw() << "\tTitlePrefix\t" << Clib::getencodedquotedstring( title_prefix() ) << pf_endl;
  if ( has_title_suffix() )
    sw() << "\tTitleSuffix\t" << Clib::getencodedquotedstring( title_suffix() ) << pf_endl;
  if ( has_title_guild() )
    sw() << "\tTitleGuild\t" << Clib::getencodedquotedstring( title_guild() ) << pf_endl;
  if ( has_title_race() )
    sw() << "\tTitleRace\t" << Clib::getencodedquotedstring( title_race() ) << pf_endl;

  if ( is_murderer() )
    sw() << "\tMurderer\t" << is_murderer() << pf_endl;
  if ( party_can_loot() )
    sw() << "\tPartyCanLoot\t" << party_can_loot() << pf_endl;
  for ( const auto& rt : reportable_ )
  {
    sw() << "\tReportable\t" << Clib::hexint( rt.serial ) << " " << rt.polclock << pf_endl;
  }

  Core::UCorpse* corpse_obj = static_cast<Core::UCorpse*>( Core::system_find_item( last_corpse ) );
  if ( corpse_obj != nullptr && !corpse_obj->orphan() )
    sw() << "\tLastCorpse\t" << last_corpse << pf_endl;
}

void Character::printDebugProperties( Clib::StreamWriter& sw ) const
{
  base::printDebugProperties( sw );
}

const char* Character::classname() const
{
  return "Character";
}

void Character::printSelfOn( Clib::StreamWriter& sw ) const
{
  base::printOn( sw );
}

void Character::printOn( Clib::StreamWriter& sw ) const
{
  base::printOn( sw );
}
void Character::printWornItems( Clib::StreamWriter& sw_pc, Clib::StreamWriter& sw_equip ) const
{
  wornitems->print( sw_pc, sw_equip );
}

Plib::MOVEMODE Character::decode_movemode( const std::string& str )
{
  Plib::MOVEMODE mm = Plib::MOVEMODE_NONE;

  const auto not_found = std::string::npos;
  if ( str.find( 'L' ) != not_found )
    mm = static_cast<Plib::MOVEMODE>( mm + Plib::MOVEMODE_LAND );
  if ( str.find( 'S' ) != not_found )
    mm = static_cast<Plib::MOVEMODE>( mm + Plib::MOVEMODE_SEA );
  if ( str.find( 'A' ) != not_found )
    mm = static_cast<Plib::MOVEMODE>( mm + Plib::MOVEMODE_AIR );
  if ( str.find( 'F' ) != not_found )
    mm = static_cast<Plib::MOVEMODE>( mm + Plib::MOVEMODE_FLY );
  return mm;
}

std::string Character::encode_movemode( Plib::MOVEMODE mm )
{
  std::string res;
  if ( mm & Plib::MOVEMODE_LAND )
    res += "L";
  if ( mm & Plib::MOVEMODE_SEA )
    res += "S";
  if ( mm & Plib::MOVEMODE_AIR )
    res += "A";
  if ( mm & Plib::MOVEMODE_FLY )
    res += "F";
  return res;
}

void Character::readCommonProperties( Clib::ConfigElem& elem )
{
  serial = elem.remove_ulong( "SERIAL" );

  if ( Plib::systemstate.config.check_integrity )
  {
    if ( Core::system_find_mobile( serial ) )
    {
      ERROR_PRINT.Format( "Character 0x{:X} defined more than once.\n" ) << serial;
      throw std::runtime_error( "Data integrity error" );
    }
  }
  serial_ext = ctBEu32( serial );
  Core::UseCharSerialNumber( serial );

  std::string acctname;
  if ( elem.remove_prop( "ACCOUNT", &acctname ) )
  {
    unsigned short charindex;
    charindex = elem.remove_ushort( "CHARIDX" );

    if ( charindex >= Plib::systemstate.config.character_slots )
    {
      ERROR_PRINT << "Account " << acctname << ": "
                  << "CHARIDX of " << charindex << " is too high for character serial (0x"
                  << fmt::hexu( serial ) << ")\n";

      throw std::runtime_error( "Data integrity error" );
    }
    Accounts::Account* search_acct = Accounts::find_account( acctname.c_str() );
    if ( search_acct == nullptr )
    {
      ERROR_PRINT << "Character '" << name() << "': "
                  << "Account '" << acctname << "' doesn't exist.\n";
      throw std::runtime_error( "Data integrity error" );
    }
    if ( search_acct->get_character( charindex ) != nullptr )
    {
      ERROR_PRINT << "Account " << acctname << " has two characters with CHARIDX of " << charindex
                  << "\n";
      throw std::runtime_error( "Data integrity error" );
    }

    acct.set( search_acct );
    acct->set_character( charindex, this );
  }

  trueobjtype = elem.remove_unsigned( "TRUEOBJTYPE", objtype_ );  // dave 1/30/3
  graphic = static_cast<u16>( objtype_ );

  registered_house = elem.remove_ulong( "REGISTEREDHOUSE", 0 );

  base::readProperties( elem );

  if ( name_ == "" )
  {
    ERROR_PRINT << "Character '0x" << fmt::hexu( serial ) << "' has no name!\n";
    throw std::runtime_error( "Data integrity error" );
  }
  wornitems->serial = serial;
  wornitems->serial_ext = serial_ext;
  position_changed();

  std::string cmdaccstr = elem.remove_string( "CMDLEVEL", "player" );
  Core::CmdLevel* cmdlevel_search = Core::find_cmdlevel( cmdaccstr.c_str() );
  if ( cmdlevel_search == nullptr )
    elem.throw_error( "Didn't understand cmdlevel of '" + cmdaccstr + "'" );
  cmdlevel_ = cmdlevel_search->cmdlevel;

  movemode = decode_movemode( elem.remove_string( "MOVEMODE", "L" ) );
  concealed_ = static_cast<unsigned char>( elem.remove_ushort(
      "CONCEALED", 0 ) );  // DAVE changed from remove_bool 11/25. concealed is a char, not a bool!
  //  if (concealed_ > cmdlevel)
  //    concealed_ = cmdlevel;

  truecolor = elem.remove_ushort( "TRUECOLOR" );

  mountedsteps_ = elem.remove_ulong( "MOUNTEDSTEPS", 0 );

  gender = static_cast<Plib::UGENDER>( elem.remove_ushort( "GENDER" ) );
  race = static_cast<Plib::URACE>( elem.remove_ushort( "RACE", Plib::RACE_HUMAN ) );

  if ( elem.remove_bool( "DEAD", false ) )
    mob_flags_.set( MOB_FLAGS::DEAD );
  if ( elem.remove_bool( "HIDDEN", false ) )
    mob_flags_.set( MOB_FLAGS::HIDDEN );
  if ( elem.remove_bool( "FROZEN", false ) )
    mob_flags_.set( MOB_FLAGS::FROZEN );

  movement_cost( Core::MovementCostMod(
      elem.remove_double( "MovementWalkMod", Core::MovementCostMod::DEFAULT.walk ),
      elem.remove_double( "MovementRunMod", Core::MovementCostMod::DEFAULT.run ),
      elem.remove_double( "MovementWalkMountedMod", Core::MovementCostMod::DEFAULT.walk_mounted ),
      elem.remove_double( "MovementRunMountedMod", Core::MovementCostMod::DEFAULT.run_mounted ) ) );

  carrying_capacity_mod( static_cast<s16>( elem.remove_int( "CarryingCapacityMod", 0 ) ) );

  height = Core::settingsManager.ssopt.default_character_height;  // no really, height is 9

  created_at = elem.remove_ulong( "CreatedAt", 0 );
  squelched_until( elem.remove_ulong( "SquelchedUntil", 0 ) );
  deafened_until( elem.remove_ulong( "DeafenedUntil", 0 ) );

  title_prefix( elem.remove_string( "TITLEPREFIX", "" ) );
  title_suffix( elem.remove_string( "TITLESUFFIX", "" ) );
  title_guild( elem.remove_string( "TITLEGUILD", "" ) );
  title_race( elem.remove_string( "TITLERACE", "" ) );

  unsigned int tmp_guildid;
  if ( elem.remove_prop( "GUILDID", &tmp_guildid ) )
    guild( Core::Guild::FindOrCreateGuild( tmp_guildid, serial ) );

  if ( elem.remove_bool( "MURDERER", false ) )
    mob_flags_.set( MOB_FLAGS::MURDERER );
  if ( elem.remove_bool( "PARTYCANLOOT", false ) )
    mob_flags_.set( MOB_FLAGS::PARTY_CAN_LOOT );

  std::string rt;
  while ( elem.remove_prop( "REPORTABLE", &rt ) )
  {
    ISTRINGSTREAM is( rt );
    reportable_t rt_t;
    if ( is >> rt_t.serial >> rt_t.polclock )
    {
      reportable_.insert( rt_t );
    }
  }

  uclang = elem.remove_string( "UCLang", "enu" );
  skillstatcap( Core::SkillStatCap(
      static_cast<s16>( elem.remove_int( "STATCAP", Core::SkillStatCap::DEFAULT.statcap ) ),
      static_cast<u16>( elem.remove_int( "SKILLCAP", Core::SkillStatCap::DEFAULT.skillcap ) ) ) );
  followers( Core::ExtStatBarFollowers(
      static_cast<s8>(
          elem.remove_int( "FOLLOWERS", Core::ExtStatBarFollowers::DEFAULT.followers ) ),
      static_cast<s8>(
          elem.remove_int( "FOLLOWERSMAX", Core::ExtStatBarFollowers::DEFAULT.followers_max ) ) ) );
  tithing( elem.remove_int( "TITHING", 0 ) );

  privs.readfrom( elem.remove_string( "Privs", "" ) );
  settings.readfrom( elem.remove_string( "Settings", "" ) );
  refresh_cached_settings();
}

// these are read here to allow NPCs to have "die-roll" type values
void Character::readAttributesAndVitals( Clib::ConfigElem& elem )
{
  // read Attributes
  for ( Attribute* pAttr : Core::gamestate.attributes )
  {
    AttributeValue& av = attribute( pAttr->attrid );

    for ( unsigned i = 0; i < pAttr->aliases.size(); ++i )
    {
      std::string temp;
      if ( elem.remove_prop( pAttr->aliases[i].c_str(), &temp ) )
      {
        // read
        unsigned int base;
        unsigned int cap = pAttr->default_cap;
        unsigned char lock = 0;
        if ( Core::settingsManager.polvar.DataWrittenBy93 &&
             Core::stateManager.gflag_in_system_load )
        {
          unsigned int raw = strtoul( temp.c_str(), nullptr, 10 );
          base = Core::raw_to_base( raw );
        }
        else
        {
          if ( Core::settingsManager.polvar.DataWrittenBy.empty() &&
               Core::stateManager.gflag_in_system_load )
          {
            elem.throw_error( "Pol.txt 'System' element needs to specify CoreVersion" );
          }

          const char* pval = temp.c_str();
          char* pdot = nullptr;
          unsigned int ones = strtoul( pval, &pdot, 10 );
          unsigned int tenths = 0;
          if ( pdot && *pdot == '.' )
            tenths = strtoul( pdot + 1, &pdot, 10 );
          base = ones * 10 + tenths;

          // Do we have caps?
          if ( pdot && *pdot == ':' )
          {
            unsigned int cap_ones = strtoul( pdot + 1, &pdot, 10 );
            unsigned int cap_tenths = 0;

            // Tenths in cap?
            if ( pdot && *pdot == '.' )
              cap_tenths = strtoul( pdot + 1, &pdot, 10 );

            cap = cap_ones * 10 + cap_tenths;
          }

          if ( pdot && *pdot == ';' )
            lock = (unsigned char)strtoul( pdot + 1, nullptr, 10 );
        }
        if ( /*base < ATTRIBUTE_MIN_BASE ||*/  // ATTRIBUTE_MIN_BASE is currently 0
             base > ATTRIBUTE_MAX_BASE )
          elem.throw_error( "Character " + Clib::hexint( serial ) + " attribute " +
                            pAttr->aliases[i] + " is out of range" );
        av.base( static_cast<unsigned short>( base ) );

        if ( /*cap < ATTRIBUTE_MIN_BASE ||*/  // ATTRIBUTE_MIN_BASE is currently 0
             cap > ATTRIBUTE_MAX_BASE )
          elem.throw_error( "Character " + Clib::hexint( serial ) + " attribute cap from " +
                            pAttr->aliases[i] + " is out of range" );
        av.cap( static_cast<unsigned short>( cap ) );


        if ( lock > 2 )  // FIXME: HardCoded Value
          elem.throw_error( "Character " + Clib::hexint( serial ) + " attribute " +
                            pAttr->aliases[i] + " has illegal lock state!" );

        av.lock( lock );

        break;
      }
    }
  }

  calc_vital_stuff();

  // read Vitals
  for ( Core::Vital* pVital : Core::gamestate.vitals )
  {
    VitalValue& vv = vital( pVital->vitalid );
    for ( const auto& _i : pVital->aliases )
    {
      unsigned int temp;
      if ( elem.remove_prop( _i.c_str(), &temp ) )
      {
        // read
        // these are always just stored as points
        if ( /*temp < Core::VITAL_MIN_VALUE ||*/  // VITAL_MIN_VALUE is currently 0
             temp > Core::VITAL_MAX_VALUE )
          elem.throw_error( "Character " + Clib::hexint( serial ) + " vital " + _i +
                            " is out of range" );
        vv.current_ones( temp );
        break;
      }
    }
  }
}

void Character::readProperties( Clib::ConfigElem& elem )
{
  readCommonProperties( elem );
  readAttributesAndVitals( elem );

  last_corpse = elem.remove_ulong( "LastCorpse", 0 );
}


bool Character::has_privilege( const char* priv ) const
{
  return privs.contains( priv ) || privs.contains( "all" );
}

bool Character::setting_enabled( const char* setting ) const
{
  return cached_settings.get( PRIV_FLAGS::ALL ) || settings.contains( setting );
}

void Character::refresh_cached_settings( bool update )
{
  PrivUpdater invulwatch( update ? this : nullptr, PRIV_FLAGS::INVUL );
  PrivUpdater seeghostswatch( update ? this : nullptr, PRIV_FLAGS::SEE_GHOSTS );
  PrivUpdater seehiddenwatch( update ? this : nullptr, PRIV_FLAGS::SEE_HIDDEN );
  PrivUpdater seeinvisitemswatch( update ? this : nullptr, PRIV_FLAGS::SEE_INVIS_ITEMS );

  cached_settings.reset();

  if ( setting_enabled( "all" ) )
  {
    cached_settings.set( PRIV_FLAGS::ALL );
    return;
  }
  if ( setting_enabled( "clotheany" ) )
    cached_settings.set( PRIV_FLAGS::CLOTHE_ANY );
  if ( setting_enabled( "dblclickany" ) )
    cached_settings.set( PRIV_FLAGS::DBLCLICK_ANY );
  if ( setting_enabled( "hearghosts" ) )
    cached_settings.set( PRIV_FLAGS::HEAR_GHOSTS );
  if ( setting_enabled( "invul" ) )
    cached_settings.set( PRIV_FLAGS::INVUL );
  if ( setting_enabled( "losany" ) )
    cached_settings.set( PRIV_FLAGS::LOS_ANY );
  if ( setting_enabled( "moveany" ) )
    cached_settings.set( PRIV_FLAGS::MOVE_ANY );
  if ( setting_enabled( "renameany" ) )
    cached_settings.set( PRIV_FLAGS::RENAME_ANY );
  if ( setting_enabled( "seeghosts" ) )
    cached_settings.set( PRIV_FLAGS::SEE_GHOSTS );
  if ( setting_enabled( "seehidden" ) )
    cached_settings.set( PRIV_FLAGS::SEE_HIDDEN );
  if ( setting_enabled( "seeinvisitems" ) )
    cached_settings.set( PRIV_FLAGS::SEE_INVIS_ITEMS );
  if ( setting_enabled( "ignoredoors" ) )
    cached_settings.set( PRIV_FLAGS::IGNORE_DOORS );
  if ( setting_enabled( "freemove" ) )
    cached_settings.set( PRIV_FLAGS::FREEMOVE );
  if ( setting_enabled( "firewhilemoving" ) )
    cached_settings.set( PRIV_FLAGS::FIRE_WHILE_MOVING );
  if ( setting_enabled( "attackhidden" ) )
    cached_settings.set( PRIV_FLAGS::ATTACK_HIDDEN );
  if ( setting_enabled( "hiddenattack" ) )
    cached_settings.set( PRIV_FLAGS::HIDDEN_ATTACK );
  if ( setting_enabled( "plogany" ) )
    cached_settings.set( PRIV_FLAGS::PLOG_ANY );
  if ( setting_enabled( "moveanydist" ) )
    cached_settings.set( PRIV_FLAGS::MOVE_ANY_DIST );
  if ( setting_enabled( "canbeheardasghost" ) )
    cached_settings.set( PRIV_FLAGS::CAN_BE_HEARD_AS_GHOST );
  if ( setting_enabled( "runwhilestealth" ) )
    cached_settings.set( PRIV_FLAGS::RUN_WHILE_STEALTH );
  if ( setting_enabled( "speedhack" ) )
    cached_settings.set( PRIV_FLAGS::SPEEDHACK );
}

void Character::set_setting( const char* setting, bool value )
{
  set_dirty();
  if ( value == false )
    settings.remove( setting );
  else if ( has_privilege( setting ) )
    settings.add( setting );

  refresh_cached_settings();
}

std::string Character::all_settings() const
{
  return settings.extract();
}

std::string Character::all_privs() const
{
  return privs.extract();
}

void Character::set_privs( const std::string& privlist )
{
  set_dirty();
  privs.readfrom( privlist );
}

void Character::grant_privilege( const char* priv )
{
  if ( !privs.contains( priv ) )
  {
    set_dirty();
    privs.add( priv );
  }
}

void Character::revoke_privilege( const char* priv )
{
  set_dirty();
  privs.remove( priv );
  settings.remove( priv );
  refresh_cached_settings();
}

bool Character::can_access( const Items::Item* item, int range ) const
{
  // TODO: find_legal_item() is awful, we should just check the item
  //       properties directly instead of going around searching for a given serial in the world

  // Range < 0 has special meaning. -1 is the default accessible range,
  // anything smaller ignores the range check.
  if ( range == -1 )
    range = Core::settingsManager.ssopt.default_accessible_range;

  const bool within_range = ( range < -1 ) || pol_distance( this, item ) <= range;
  if ( within_range && ( find_legal_item( this, item->serial ) != nullptr ) )
    return true;

  return false;
}

bool Character::can_move( const Items::Item* item ) const
{
  if ( item->objtype_ != UOBJ_CORPSE )
  {
    return cached_settings.get( PRIV_FLAGS::MOVE_ANY ) || item->movable();
  }
  else
  {
    return false;
  }
}

bool Character::can_be_renamed_by( const Character* /*chr*/ ) const
{
  // consider command levels?
  return false;
}

bool Character::can_rename( const Character* chr ) const
{
  return cached_settings.get( PRIV_FLAGS::RENAME_ANY ) || chr->can_be_renamed_by( this );
}

bool Character::can_clothe( const Character* chr ) const
{
  return ( ( chr == this ) || cached_settings.get( PRIV_FLAGS::CLOTHE_ANY ) );
}

bool Character::can_hearghosts() const
{
  return cached_settings.get( PRIV_FLAGS::HEAR_GHOSTS );
}

bool Character::can_be_heard_as_ghost() const
{
  return cached_settings.get( PRIV_FLAGS::CAN_BE_HEARD_AS_GHOST );
}

bool Character::can_moveanydist() const
{
  return cached_settings.get( PRIV_FLAGS::MOVE_ANY_DIST );
}

bool Character::can_plogany() const
{
  return cached_settings.get( PRIV_FLAGS::PLOG_ANY );
}

bool Character::can_speedhack() const
{
  return cached_settings.get( PRIV_FLAGS::SPEEDHACK );
}

bool Character::can_freemove() const
{
  return cached_settings.get( PRIV_FLAGS::FREEMOVE );
}

Core::UContainer* Character::backpack() const
{
  return static_cast<Core::UContainer*>( wornitems->GetItemOnLayer( Core::LAYER_BACKPACK ) );
}

Core::Spellbook* Character::spellbook( u8 school ) const
{
  Items::Item* _item = wornitem( Core::LAYER_HAND1 );
  if ( _item != nullptr && _item->script_isa( Core::POLCLASS_SPELLBOOK ) )
  {
    Core::Spellbook* book = static_cast<Core::Spellbook*>( _item );
    if ( book->spell_school == school )
      return book;
  }

  Core::UContainer* cont = backpack();
  if ( cont != nullptr )
  {
    for ( Core::UContainer::const_iterator itr = cont->begin(); itr != cont->end(); ++itr )
    {
      const Items::Item* item = GET_ITEM_PTR( itr );

      if ( item != nullptr && item->script_isa( Core::POLCLASS_SPELLBOOK ) )
      {
        const Core::Spellbook* book = static_cast<const Core::Spellbook*>( item );
        if ( book->spell_school == school )
          return const_cast<Core::Spellbook*>( book );
      }
    }
  }
  return nullptr;
}

unsigned int Character::gold_carried() const
{
  Core::UContainer* bp = backpack();
  if ( bp != nullptr )
    return bp->find_sumof_objtype_noninuse( UOBJ_GOLD_COIN );
  else
    return 0;
}

// TODO: This could be more efficient, by inlining 'spend' logic
// in a recursive function

void Character::spend_gold( unsigned int amount )
{
  passert( gold_carried() >= amount );

  Core::UContainer* bp = backpack();
  if ( bp != nullptr )
    bp->consume_sumof_objtype_noninuse( UOBJ_GOLD_COIN, amount );
  if ( client != nullptr )
    send_full_statmsg( client, this );
}

Items::Item* Character::wornitem( int layer ) const
{
  return wornitems->GetItemOnLayer( layer );
}

bool Character::layer_is_equipped( int layer ) const
{
  return ( wornitems->GetItemOnLayer( layer ) != nullptr );
}

bool Character::is_equipped( const Items::Item* item ) const
{
  if ( !Items::valid_equip_layer( item ) )
    return false;

  return ( wornitems->GetItemOnLayer( item->tile_layer ) == item );
}

bool Character::strong_enough_to_equip( const Items::Item* item ) const
{
  const Items::ItemDesc& desc = item->itemdesc();
  return ( attribute( Core::gamestate.pAttrStrength->attrid ).base() >= desc.base_str_req );
}

bool Character::equippable( const Items::Item* item ) const
{
  if ( !Items::valid_equip_layer( item ) )
  {
    if ( item->objtype_ == Core::settingsManager.extobj.mount )
    {
      POLLOG_INFO.Format(
          "\nWarning: Character 0x{:X} tried to mount Item 0x{:X}, but it doesn't have a mount "
          "graphic (current graphic: 0x{:X}). Check that the list of mounts in uoconvert.cfg is "
          "correct and re-run uoconvert if necessary.\n" )
          << this->serial << item->serial << item->graphic;
    }

    return false;
  }
  if ( layer_is_equipped( item->tile_layer ) )
  {
    return false;
  }
  if ( ( item->tile_layer == Core::LAYER_BACKPACK ) &&
       !item->isa( Core::UOBJ_CLASS::CLASS_CONTAINER ) )
  {
    return false;
  }

  // Only allow mounts if they have the mount objtype as defined in extobj.cfg
  if ( item->tile_layer == Core::LAYER_MOUNT && Plib::systemstate.config.enforce_mount_objtype &&
       item->objtype_ != Core::settingsManager.extobj.mount )
  {
    POLLOG_INFO.Format(
        "\nWarning: Character 0x{:X} tried to mount Item 0x{:X}, but it doesn't have the mount "
        "objtype (as defined in extobj.cfg) and EnforceMountObjtype in pol.cfg is true.\n" )
        << this->serial << item->serial;
    return false;
  }

  if ( ~Plib::tile_flags( item->graphic ) & Plib::FLAG::EQUIPPABLE )
  {
    return false;
  }
  // redundant sanity check
  if ( Plib::tilelayer( item->graphic ) != item->tile_layer )
  {
    return false;
  }

  const Items::ItemDesc& desc = item->itemdesc();
  if ( attribute( Core::gamestate.pAttrStrength->attrid ).base() < desc.base_str_req )
  {
    return false;
  }

  if ( item->tile_layer == Core::LAYER_HAND1 || item->tile_layer == Core::LAYER_HAND2 )
  {
    if ( weapon != nullptr )
    {
      if ( weapon->descriptor().two_handed && !weapon->is_intrinsic() )
      {
        return false;
      }
    }
    if ( item->isa( Core::UOBJ_CLASS::CLASS_WEAPON ) )
    {
      const Items::UWeapon* wpn_item = static_cast<const Items::UWeapon*>( item );
      if ( wpn_item->descriptor().two_handed )
      {
        if ( layer_is_equipped( Core::LAYER_HAND1 ) || layer_is_equipped( Core::LAYER_HAND2 ) )
        {
          return false;
        }
      }
    }
  }

  return true;
}

void Character::equip( Items::Item* item )
{
  passert_r( equippable( item ),
             "It is impossible to equip Item with ObjType " + Clib::hexint( item->objtype_ ) );

  wornitems->PutItemOnLayer( item );

  // PutItemOnLayer sets the layer, so we can go on now
  // checking item->layer instead of item->tile_layer
  if ( item->isa( Core::UOBJ_CLASS::CLASS_WEAPON ) &&
       ( item->layer == Core::LAYER_HAND1 || item->layer == Core::LAYER_HAND2 ) )
  {
    weapon = static_cast<Items::UWeapon*>( item );
    reset_swing_timer();
  }
  else if ( item->isa( Core::UOBJ_CLASS::CLASS_ARMOR ) )
  {
    if ( item->layer == Core::LAYER_HAND1 || item->layer == Core::LAYER_HAND2 )
    {
      shield = static_cast<Items::UArmor*>( item );
    }
    reset_swing_timer();
  }
  refresh_ar();
}

Items::UWeapon* Character::intrinsic_weapon()
{
  return Core::gamestate.wrestling_weapon;
}

void Character::unequip( Items::Item* item )
{
  passert( Items::valid_equip_layer( item ) );
  // assume any item being de-equipped is in fact being worn.
  passert( item->container == wornitems.get() );
  passert( is_equipped( item ) );

  wornitems->RemoveItemFromLayer( item );

  if ( item == weapon )
  {
    weapon = intrinsic_weapon();
    /* we don't reset the swing timer here, 'cause
       you can switch from weapon to H2H quickly.
       Note, this _could_ let you use a faster weapon
       to wrestle faster, hmm.
       */
  }
  else if ( item == shield )
  {
    shield = nullptr;
    reset_swing_timer();
  }
  refresh_ar();
}

bool Character::on_mount() const
{
  if ( race == Plib::RACE_GARGOYLE )
    return ( movemode & Plib::MOVEMODE_FLY ) == 0 ? false : true;

  return layer_is_equipped( Core::LAYER_MOUNT );
}


Items::Item* Character::find_wornitem( u32 find_serial ) const
{
  for ( unsigned layer = Core::LAYER_EQUIP__LOWEST; layer <= Core::LAYER_EQUIP__HIGHEST; layer++ )
  {
    Items::Item* item = wornitems->GetItemOnLayer( layer );
    if ( item )
    {
      if ( item->serial == find_serial )
        return item;
      // 4/2007 - MuadDib
      // Added cont check and using cont->find to check here
      // for equipped cont items like the ML added Quivers.
      // Using redundant null check.
      if ( item != nullptr && item->script_isa( Core::POLCLASS_CONTAINER ) )
      {
        if ( layer != Core::LAYER_HAIR && layer != Core::LAYER_FACE && layer != Core::LAYER_BEARD &&
             layer != Core::LAYER_BACKPACK && layer != Core::LAYER_MOUNT )
        {
          Core::UContainer* cont = static_cast<Core::UContainer*>( item );
          item = nullptr;
          item = cont->find( find_serial );
          if ( item != nullptr )
            return item;
        }
      }
    }
  }
  return nullptr;
}

void Character::produce( const Core::Vital* pVital, VitalValue& vv, unsigned int amt )
{
  int start_ones = vv.current_ones();
  set_dirty();
  vv.produce( amt );
  if ( start_ones != vv.current_ones() )
    Network::ClientInterface::tell_vital_changed( this, pVital );
}

bool Character::consume( const Core::Vital* pVital, VitalValue& vv, unsigned int amt,
                         VitalDepletedReason reason )
{
  int start_ones = vv.current_ones();
  set_dirty();
  bool res = vv.consume( amt );
  if ( start_ones != vv.current_ones() )
  {
    Network::ClientInterface::tell_vital_changed( this, pVital );
    if ( start_ones != 0 && vv.current_ones() == 0 && pVital->depleted_func != nullptr )
      pVital->depleted_func->call( new Module::ECharacterRefObjImp( this ),
                                   new Bscript::BLong( static_cast<int>( reason ) ) );
  }
  return res;
}

void Character::set_current_ones( const Core::Vital* pVital, VitalValue& vv, unsigned int ones,
                                  VitalDepletedReason reason )
{
  int start_ones = vv.current_ones();
  set_dirty();
  vv.current_ones( ones );
  Network::ClientInterface::tell_vital_changed( this, pVital );
  if ( start_ones != 0 && vv.current_ones() == 0 && pVital->depleted_func != nullptr )
    pVital->depleted_func->call( new Module::ECharacterRefObjImp( this ),
                                 new Bscript::BLong( static_cast<int>( reason ) ) );
}

void Character::set_current( const Core::Vital* pVital, VitalValue& vv, unsigned int ones,
                             VitalDepletedReason reason )
{
  int start_ones = vv.current_ones();
  set_dirty();
  vv.current( ones );
  Network::ClientInterface::tell_vital_changed( this, pVital );
  if ( start_ones != 0 && vv.current_ones() == 0 && pVital->depleted_func != nullptr )
    pVital->depleted_func->call( new Module::ECharacterRefObjImp( this ),
                                 new Bscript::BLong( static_cast<int>( reason ) ) );
}

void Character::regen_vital( const Core::Vital* pVital )
{
  VitalValue& vv = vital( pVital->vitalid );
  int rr = vv.regenrate();
  if ( rr > 0 )
    produce( pVital, vv, rr / 12 );
  else if ( rr < 0 )
    consume( pVital, vv, -rr / 12, VitalDepletedReason::REGENERATE );
}

void Character::calc_vital_stuff( bool i_mod, bool v_mod )
{
  if ( i_mod )
  {
    for ( unsigned ai = 0; ai < Core::gamestate.numAttributes; ++ai )
    {
      calc_single_attribute( Core::gamestate.attributes[ai] );
    }
  }

  if ( v_mod )
  {
    for ( unsigned vi = 0; vi < Core::gamestate.numVitals; ++vi )
    {
      calc_single_vital( Core::gamestate.vitals[vi] );
    }
  }
}

void Character::calc_single_vital( const Core::Vital* pVital )
{
  VitalValue& vv = vital( pVital->vitalid );

  int start_ones = vv.current_ones();
  int start_max = vv.maximum_ones();

  // dave change the order of maximum and regen function 3/19/3
  int mv = pVital->get_maximum_func->call_long( new Module::ECharacterRefObjImp( this ) );

  if ( mv < static_cast<int>( Core::VITAL_LOWEST_MAX_HUNDREDTHS ) )
    mv = Core::VITAL_LOWEST_MAX_HUNDREDTHS;
  if ( mv > static_cast<int>( Core::VITAL_HIGHEST_MAX_HUNDREDTHS ) )
    mv = Core::VITAL_HIGHEST_MAX_HUNDREDTHS;

  vv.maximum( mv );

  int rr = pVital->get_regenrate_func->call_long( new Module::ECharacterRefObjImp( this ) );

  if ( rr < Core::VITAL_LOWEST_REGENRATE )
    rr = Core::VITAL_LOWEST_REGENRATE;
  if ( rr > Core::VITAL_HIGHEST_REGENRATE )
    rr = Core::VITAL_HIGHEST_REGENRATE;

  vv.regenrate( rr );

  if ( ( start_ones != vv.current_ones() ) || ( start_max != vv.maximum_ones() ) )
    Network::ClientInterface::tell_vital_changed( this, pVital );
}

void Character::calc_single_attribute( const Attribute* pAttr )
{
  AttributeValue& av = attribute( pAttr->attrid );

  if ( pAttr->getintrinsicmod_func )
  {
    int im = pAttr->getintrinsicmod_func->call_long( new Module::ECharacterRefObjImp( this ) );

    if ( im < ATTRIBUTE_MIN_INTRINSIC_MOD )
      im = ATTRIBUTE_MIN_INTRINSIC_MOD;
    if ( im > ATTRIBUTE_MAX_INTRINSIC_MOD )
      im = ATTRIBUTE_MAX_INTRINSIC_MOD;

    av.intrinsic_mod( static_cast<short>( im ) );
  }
}

void Character::set_vitals_to_maximum()  // throw()
{
  set_dirty();
  for ( unsigned vi = 0; vi < Core::gamestate.numVitals; ++vi )
  {
    VitalValue& vv = vital( vi );
    vv.current( vv.maximum() );

    Network::ClientInterface::tell_vital_changed( this, Core::gamestate.vitals[vi] );
  }
}

bool Character::setgraphic( u16 newgraphic )
{
  if ( newgraphic < 1 ||
       newgraphic >
           0x800 )  // Maximum graphic: 2048, changed to allow new graphics -- Nando - 2009-01-14
    return false;

  set_dirty();
  graphic = newgraphic;
  send_remove_character_to_nearby_cansee( this );
  if ( client )
    send_goxyz( client, client->chr );
  send_create_mobile_to_nearby_cansee( this );

  return true;
}

void Character::on_color_changed()
{
  send_remove_character_to_nearby_cansee( this );
  if ( client )
    send_goxyz( client, client->chr );
  send_create_mobile_to_nearby_cansee( this );
}

void Character::on_poison_changed()
{
  send_move_mobile_to_nearby_cansee( this );

  // only if client is active or for npcs
  if ( ( client ) || ( this->isa( Core::UOBJ_CLASS::CLASS_NPC ) ) )
  {
    if ( client )
    {
      send_goxyz( client, client->chr );
      // if poisoned send_goxyz handles 0x17 packet
      if ( !poisoned() )
        send_poisonhealthbar( client, client->chr );
    }
    // This is a KR only packet, so transmit it only to KR clients
    // who are in range.
    // if poisoned send_move_mobile_to_nearby_cansee handles 0x17 packet
    if ( !poisoned() )
    {
      Network::HealthBarStatusUpdate msg( serial_ext, Network::HealthBarStatusUpdate::Color::GREEN,
                                          poisoned() );
      Core::WorldIterator<Core::OnlinePlayerFilter>::InVisualRange(
          this, [&]( Character* zonechr ) { msg.Send( zonechr->client ); } );
    }
  }
}

void Character::on_hidden_changed()
{
  if ( hidden() )
  {
    set_stealthsteps( 0 );
    if ( client )
      send_move( client, this );
    send_remove_character_to_nearby_cantsee( this );
    send_create_mobile_to_nearby_cansee( this );
  }
  else
  {
    unhide();
    set_stealthsteps( 0 );
  }
}

void Character::on_concealed_changed()
{
  if ( concealed() )
  {
    if ( client )
      send_move( client, this );
    send_remove_character_to_nearby_cantsee( this );
    send_create_mobile_to_nearby_cansee( this );
  }
  else if ( is_visible() )
    unhide();
  set_stealthsteps( 0 );
}

void Character::on_cmdlevel_changed()
{
  send_remove_character_to_nearby_cantsee( this );
  send_create_mobile_to_nearby_cansee( this );
}

void Character::on_facing_changed()
{
  if ( client )
    send_goxyz( client, client->chr );
  send_move_mobile_to_nearby_cansee( this );
}

void Character::on_aos_ext_stat_changed()
{
  if ( client )
  {
    if ( client->UOExpansionFlag & Network::AOS && has_active_client() )
    {
      send_full_statmsg( client, client->chr );
    }
  }
}

void Character::setfacing( u8 newfacing )
{
  facing = newfacing & 7;
}

u8 Character::get_flag1( Network::Client* other_client ) const
{
  // Breaks paperdoll
  u8 flag1 = 0;
  if ( gender )
    flag1 |= Core::CHAR_FLAG1_GENDER;
  if ( ( poisoned() ) &&
       ( ~other_client->ClientType &
         Network::CLIENTTYPE_7000 ) )  // client >=7 receive the poisonflag with 0x17
    flag1 |= Core::CHAR_FLAG1_POISONED;
  if ( ( movemode & Plib::MOVEMODE_FLY ) &&
       ( other_client->ClientType & Network::CLIENTTYPE_7000 ) )
    flag1 |= Core::CHAR_FLAG1_FLYING;
  if ( ( Core::settingsManager.ssopt.invul_tag == 2 ) && ( invul() ) )
    flag1 |= Core::CHAR_FLAG1_YELLOWHEALTH;
  if ( warmode() )
    flag1 |= Core::CHAR_FLAG1_WARMODE;
  if ( !is_visible() )
    flag1 |= Core::CHAR_FLAG1_INVISIBLE;

  return flag1;
}

void Character::apply_raw_damage_hundredths( unsigned int amount, Character* source, bool userepsys,
                                             bool send_damage_packet )
{
  if ( dead() )
  {
    // cerr << "Waah! " << name() << " " << hexint(serial) << " is dead, but taking damage?" <<
    // endl;
    return;
  }

  if ( ( source ) && ( userepsys ) )
    source->repsys_on_attack( this );

  if ( ( amount == 0 ) || cached_settings.get( PRIV_FLAGS::INVUL ) )
    return;

  set_dirty();
  if ( hidden() )
    unhide();

  if ( send_damage_packet && source )
  {
    u16 showdmg;
    if ( amount > 6553500 )  // 0xFFFF*100
      showdmg = 0xFFFF;
    else
      showdmg = static_cast<u16>( amount / 100 );
    send_damage( source, this, showdmg );
  }

  if ( paralyzed() )
    mob_flags_.remove( MOB_FLAGS::PARALYZED );

  disable_regeneration_for( 2 );  // FIXME depend on amount?

  // 0.xx is still 0
  VitalValue& vv = vital( Core::gamestate.pVitalLife->vitalid );
  if ( vv.current() - amount <= 99 )
    amount = vv.current();  // be greedy with that last point
  consume( Core::gamestate.pVitalLife, vv, amount, VitalDepletedReason::DAMAGE );

  if ( ( source ) && ( userepsys ) )
    source->repsys_on_damage( this );

  send_update_hits_to_inrange( this );

  if ( vv.current() == 0 )
    die();
}

// keep this in sync with NPC::armor_absorb_damage


unsigned short calc_thru_damage( double damage, unsigned short ar )
{
  int blocked = ar;
  if ( blocked < 0 )
    blocked = 0;

  int absorbed = blocked / 2;

  blocked -= absorbed;
  absorbed += Clib::random_int( blocked );
  damage -= absorbed;

  if ( damage >= 2.0 )
  {
    return static_cast<unsigned short>( damage * 0.5 );
  }
  else
  {
    int dmg = static_cast<int>( damage );
    if ( dmg >= 0 )
      return static_cast<unsigned short>( dmg );
    else
      return 0;
  }
}


double Character::armor_absorb_damage( double damage )
{
  Items::UArmor* armor = choose_armor();
  if ( armor != nullptr )
  {
    damage = calc_thru_damage( damage, armor->ar() + ar_mod() );

    armor->reduce_hp_from_hit();
  }
  return damage;
}

double Character::apply_damage( double damage, Character* source, bool userepsys,
                                bool send_damage_packet )
{
  damage = armor_absorb_damage( damage );
  if ( Core::settingsManager.watch.combat )
    INFO_PRINT << "Final damage: " << damage << "\n";
  do_imhit_effects();
  apply_raw_damage_hundredths( static_cast<unsigned int>( damage * 100 ), source, userepsys,
                               send_damage_packet );

  return damage;
}

void Character::get_hitscript_params( double damage, Items::UArmor** parmor,
                                      unsigned short* rawdamage )
{
  Items::UArmor* armor = choose_armor();
  if ( armor )
  {
    *rawdamage = calc_thru_damage( damage, armor->ar() + ar_mod() );
  }
  else
  {
    *rawdamage = static_cast<unsigned short>( damage );
  }
  *parmor = armor;
}

void Character::run_hit_script( Character* defender, double damage )
{
  ref_ptr<Bscript::EScriptProgram> prog = find_script2(
      weapon->hit_script(), true, Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() == nullptr )
    return;

  std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor() );
  auto uoemod = new Module::UOExecutorModule( *ex );
  ex->addModule( uoemod );

  unsigned short rawdamage = 0;
  unsigned short basedamage = static_cast<unsigned short>( damage );

  Items::UArmor* armor = nullptr;

  defender->get_hitscript_params( damage, &armor, &rawdamage );


  ex->pushArg( new Bscript::BLong( rawdamage ) );
  ex->pushArg( new Bscript::BLong( basedamage ) );
  if ( armor )
    ex->pushArg( new Module::EItemRefObjImp( armor ) );
  else
    ex->pushArg( new Bscript::BLong( 0 ) );
  ex->pushArg( new Module::EItemRefObjImp( weapon ) );
  ex->pushArg( new Module::ECharacterRefObjImp( defender ) );
  ex->pushArg( new Module::ECharacterRefObjImp( this ) );

  ex->priority( 100 );

  if ( ex->setProgram( prog.get() ) )
  {
    uoemod->controller_.set( this );
    schedule_executor( ex.release() );
  }
  else
  {
    POLLOG << "Blech, couldn't start hitscript " << weapon->hit_script().name() << "\n";
  }
}

///
/// Clear a Mobile's ToBeReportable list when all of the following are true:
///   1) hits are at maximum
///   2) mobile is not poisoned
///   3) mobile is not paralyzed
///
void Character::check_undamaged()
{
  if ( vital( Core::gamestate.pVitalLife->vitalid ).is_at_maximum() && !poisoned() && !paralyzed() )
  {
    clear_to_be_reportables();
  }
}


///
/// When a Mobile is Healed
///
///   if Amy's hits are at maximum,
///     Clear Amy's ToBeReportable list
///
///   (note, poisoned and paralyzed flags are not checked)
///
void Character::heal_damage_hundredths( unsigned int amount )
{
  if ( dead() )
  {
    // cerr << "Waah! " << name() << " is dead, but healing damage?" << endl;
    return;
  }

  if ( amount == 0 )
    return;

  produce( Core::gamestate.pVitalLife, vital( Core::gamestate.pVitalLife->vitalid ), amount );

  check_undamaged();

  send_update_hits_to_inrange( this );
}

Items::Item* create_death_shroud()
{
  Items::Item* item = Items::Item::create( UOBJ_DEATH_SHROUD );
  item->layer = Core::LAYER_ROBE_DRESS;
  return item;
}
Items::Item* create_death_robe()
{
  Items::Item* item = Items::Item::create( UOBJ_DEATH_ROBE );
  item->layer = Core::LAYER_ROBE_DRESS;
  return item;
}
Items::Item* create_backpack()
{
  Items::Item* item = Items::Item::create( UOBJ_BACKPACK );
  item->layer = Core::LAYER_BACKPACK;
  return item;
}


void Character::send_warmode()
{
  Network::PktHelper::PacketOut<Network::PktOut_72> msg;
  msg->Write<u8>( warmode() ? 1u : 0u );
  msg->offset++;  // u8 unk2
  msg->Write<u8>( 0x32u );
  msg->offset++;  // u8 unk4
  msg.Send( client );
}

void send_remove_if_hidden_ghost( Character* chr, Network::Client* client )
{
  if ( !inrange( chr, client->chr ) )
    return;

  if ( !client->chr->is_visible_to_me( chr ) )
  {
    send_remove_character( client, chr );
  }
}
void send_create_ghost( Character* chr, Network::Client* client )
{
  if ( !inrange( chr, client->chr ) )
    return;

  if ( chr->dead() && client->chr->is_visible_to_me( chr ) )
  {
    send_owncreate( client, chr );
  }
}

void Character::resurrect()
{
  if ( !dead() )
  {
    ERROR_PRINT << "uh, trying to resurrect " << name() << ", who isn't dead.\n";
    return;
  }
  set_dirty();
  if ( graphic == UOBJ_HUMAN_MALE_GHOST )
    graphic = UOBJ_HUMAN_MALE;
  else if ( graphic == UOBJ_HUMAN_FEMALE_GHOST )
    graphic = UOBJ_HUMAN_FEMALE;
  else if ( graphic == UOBJ_ELF_MALE_GHOST )
    graphic = UOBJ_ELF_MALE;
  else if ( graphic == UOBJ_ELF_FEMALE_GHOST )
    graphic = UOBJ_ELF_FEMALE;
  else if ( graphic == UOBJ_GARGOYLE_MALE_GHOST )
    graphic = UOBJ_GARGOYLE_MALE;
  else if ( graphic == UOBJ_GARGOYLE_FEMALE_GHOST )
    graphic = UOBJ_GARGOYLE_FEMALE;

  mob_flags_.remove( MOB_FLAGS::DEAD );
  mob_flags_.remove( MOB_FLAGS::WARMODE );
  mob_flags_.remove( MOB_FLAGS::FROZEN );
  mob_flags_.remove( MOB_FLAGS::PARALYZED );

  color = truecolor;

  if ( Core::gamestate.pVitalLife->regen_while_dead )
  {
    VitalValue& vv = vital( Core::gamestate.pVitalLife->vitalid );
    if ( vv._current == 0 )  // set in die()
      set_current_ones( Core::gamestate.pVitalLife, vv, 1, VitalDepletedReason::RESURRECT );
  }
  else
    set_current_ones( Core::gamestate.pVitalLife, vital( Core::gamestate.pVitalLife->vitalid ), 1,
                      VitalDepletedReason::RESURRECT );

  if ( !Core::gamestate.pVitalMana->regen_while_dead )
    set_current_ones( Core::gamestate.pVitalMana, vital( Core::gamestate.pVitalMana->vitalid ), 0,
                      VitalDepletedReason::RESURRECT );

  if ( !Core::gamestate.pVitalStamina->regen_while_dead )
    set_current_ones( Core::gamestate.pVitalStamina,
                      vital( Core::gamestate.pVitalStamina->vitalid ), 1,
                      VitalDepletedReason::RESURRECT );

  // Replace the death shroud with a death robe
  bool equip_death_robe = true;
  if ( layer_is_equipped( Core::LAYER_ROBE_DRESS ) )
  {
    Items::Item* death_shroud = wornitems->GetItemOnLayer( Core::LAYER_ROBE_DRESS );
    if ( death_shroud->objtype_ == UOBJ_DEATH_SHROUD )
    {
      unequip( death_shroud );
      death_shroud->destroy();
      death_shroud = nullptr;
    }
    else
    {
      // Do not destroy and replace the already equipped robe
      equip_death_robe = false;
    }
  }
  if ( equip_death_robe )
  {
    Items::Item* death_robe = create_death_robe();
    death_robe->setposition( pos() );
    equip( death_robe );
  }

  // equip( create_backpack() );

  // if this has a connected client, tell him his new position.
  if ( client )
  {
    client->pause();
    send_warmode();
    send_goxyz( client, this );
    send_owncreate( client, this );
    Core::WorldIterator<Core::MobileFilter>::InVisualRange(
        client->chr,
        [&]( Character* zonechr ) { send_remove_if_hidden_ghost( zonechr, client ); } );
    client->restart();
  }

  // Tell other connected players, if in range, about this character.
  send_remove_character_to_nearby_cansee( this );
  send_create_mobile_to_nearby_cansee( this );
  realm()->notify_resurrected( *this );
}

void Character::on_death( Items::Item* corpse )
{
  Items::Item* death_shroud = create_death_shroud();
  death_shroud->setposition( pos() );
  if ( equippable( death_shroud ) )  // check it or passert will trigger
  {
    equip( death_shroud );
    send_wornitem_to_inrange( this, death_shroud );
  }
  else
  {
    ERROR_PRINT.Format( "Create Character: Failed to equip death shroud 0x{:X}\n" )
        << death_shroud->graphic;
    death_shroud->destroy();
  }

  if ( client != nullptr )
  {
    if ( opponent_ )
      opponent_->inform_disengaged( this );

    client->pause();
    send_warmode();

    // Sends the complete corpse to the client itself, so he knows where his
    // items went.
    send_full_corpse( client, corpse );

    send_goxyz( client, this );
    Core::WorldIterator<Core::MobileFilter>::InVisualRange(
        client->chr, [&]( Character* zonechr ) { send_create_ghost( zonechr, client ); } );

    client->restart();
  }

  // change self to ghost for ghosts, remove self for living
  send_remove_character_to_nearby_cantsee( this );

  send_create_mobile_to_nearby_cansee( this );

  if ( Clib::FileExists( "scripts/misc/chrdeath.ecl" ) )
    Core::start_script( "misc/chrdeath", new Module::EItemRefObjImp( corpse ),
                        new Module::ECharacterRefObjImp( this ) );
}

void Character::clear_opponent_of()
{
  while ( !opponent_of.empty() )
  {
    Character* chr = *opponent_of.begin();
    // note that chr->set_opponent is going to remove
    // its entry from our opponent_of collection,
    // so eventually this loop will exit.
    chr->set_opponent( nullptr, false );
  }
}

void Character::die()
{
  if ( Core::gamestate.system_hooks.can_die )
  {
    if ( !Core::gamestate.system_hooks.can_die->call( make_mobileref( this ) ) )
      return;
  }

  set_current_ones( Core::gamestate.pVitalLife, vital( Core::gamestate.pVitalLife->vitalid ), 0,
                    VitalDepletedReason::DEATH );
  clear_my_aggressors();
  clear_my_lawful_damagers();
  commit_to_reportables();


  u16 save_graphic = graphic;

  if ( graphic == UOBJ_HUMAN_MALE || graphic == UOBJ_HUMAN_FEMALE || graphic == UOBJ_ELF_MALE ||
       graphic == UOBJ_ELF_FEMALE || graphic == UOBJ_GARGOYLE_MALE ||
       graphic == UOBJ_GARGOYLE_FEMALE )
  {
    switch ( race )
    {
    case Plib::RACE_HUMAN:
      graphic = ( gender == Plib::GENDER_MALE ) ? UOBJ_HUMAN_MALE_GHOST : UOBJ_HUMAN_FEMALE_GHOST;
      break;
    case Plib::RACE_ELF:
      graphic = ( gender == Plib::GENDER_MALE ) ? UOBJ_ELF_MALE_GHOST : UOBJ_ELF_FEMALE_GHOST;
      break;
    case Plib::RACE_GARGOYLE:
      graphic =
          ( gender == Plib::GENDER_MALE ) ? UOBJ_GARGOYLE_MALE_GHOST : UOBJ_GARGOYLE_FEMALE_GHOST;
      break;
    }
  }
  DECLARE_CHECKPOINT;

  mob_flags_.set( MOB_FLAGS::DEAD );
  mob_flags_.remove( MOB_FLAGS::WARMODE );
  mob_flags_.remove( MOB_FLAGS::FROZEN );
  mob_flags_.remove( MOB_FLAGS::PARALYZED );

  UPDATE_CHECKPOINT();
  /* FIXME: corpse container difficulties.

     What am I gonna do about the corpse.  I've said that
     MAX_CONTAINER_ITEMS can be in a container, but if I put
     worn items, plus what's in the backpack, in the
     corpse 'container' I'm going to violate this.  If
     I put everything in a backpack in the corpse,
     it's too easy to grab everything.
     Maybe I have to limit the backpack to MAX_CONTAINER_ITEM
     - NUM_LAYERS or something, or grow the "send container"
     message to be able to hold a full backpack, plus
     all worn items.  That doesn't work, tho, 'cause it
     overfills the corpse container. ick.

     The current solution is to put the backpack
     on the corpse.
     */

  Core::UCorpse* corpse = static_cast<Core::UCorpse*>( Items::Item::create( UOBJ_CORPSE ) );
  this->last_corpse = corpse->serial;

  corpse->ownerserial = this->serial;
  corpse->setname( "A corpse of " + name_.get() );
  corpse->take_contents_to_grave( acct == nullptr );

  UPDATE_CHECKPOINT();

  corpse->color = truecolor;
  corpse->setposition( pos() );
  corpse->facing = facing;
  corpse->corpsetype = save_graphic;
  // corpse->dir = dir;
  UPDATE_CHECKPOINT();

  register_with_supporting_multi( corpse );
  if ( is_trading() )
    Core::cancel_trade( this );
  clear_gotten_item();
  corpse->copyprops( *this );
  UPDATE_CHECKPOINT();

  // Change the character's color to grey
  color = 0;
  UPDATE_CHECKPOINT();

  // small lambdas to reduce the mess inside the loops
  auto _copy_item = [&]( Items::Item* _item ) {  // copy a item into the corpse
    Items::Item* copy = _item->clone();
    copy->invisible( true );
    copy->movable( false );
    corpse->add( copy );
  };
  auto _drop_item_to_world = [&]( Items::Item* _item ) {  // places the item onto the corpse coords
    _item->setposition( corpse->pos() );
    add_item_to_world( _item );
    register_with_supporting_multi( _item );
    move_item( _item, corpse->x(), corpse->y(), corpse->z(), nullptr );
  };

  // WARNING: never ever touch or be 10000% sure what you are doing!!!!
  for ( unsigned layer = Core::LAYER_EQUIP__LOWEST; layer <= Core::LAYER_EQUIP__HIGHEST; ++layer )
  {
    Items::Item* item = wornitems->GetItemOnLayer( layer );
    if ( item == nullptr )
      continue;
    if ( item->layer == Core::LAYER_BACKPACK )  // These needs to be the first!!!!
      continue;
    // never ever touch this order
    // first only copy the hair layers and only these!
    // then check for newbie and then I dont care
    if ( item->layer == Core::LAYER_BEARD || item->layer == Core::LAYER_HAIR ||
         item->layer == Core::LAYER_FACE )
    {
      // Copies hair items onto the corpse
      _copy_item( item );
      continue;
    }
    if ( item->newbie() || item->insured() || item->no_drop() )
      continue;
    else if ( item->layer != Core::LAYER_MOUNT && item->layer != Core::LAYER_ROBE_DRESS &&
              !item->movable() )  // dress layer needs to be unequipped for deathrobe
    {
      _copy_item( item );
      continue;
    }
    ///
    /// Unequip scripts aren't honored when moving a dead mobile's equipment
    /// onto a corpse if honor_unequip_script_on_death is disabled.
    ///
    UPDATE_CHECKPOINT();
    if ( Core::settingsManager.ssopt.honor_unequip_script_on_death )
    {
      if ( !item->check_unequiptest_scripts() )
        continue;
      if ( !item->check_unequip_script() )
        continue;
    }
    else
    {
      item->check_unequip_script();
    }
    UPDATE_CHECKPOINT();
    unequip( item );
    UPDATE_CHECKPOINT();

    u8 newSlot = 1;
    if ( !corpse->can_add_to_slot( newSlot ) || !item->slot_index( newSlot ) )
    {
      _drop_item_to_world( item );
    }
    else
    {
      corpse->add_at_random_location( item );
    }
    UPDATE_CHECKPOINT();
  }
  UPDATE_CHECKPOINT();

  // For some reason, the backpack shows up as a small child.. So change its layer.
  Core::UContainer* bp = backpack();
  if ( bp )
  {
    Core::UContainer::Contents tmp;
    UPDATE_CHECKPOINT();
    bp->extract( tmp );
    UPDATE_CHECKPOINT();
    // We set slot to 1 outside the loop. As it cycles through, this will continue
    // to increase. This will reduce the amount of checks to find next available
    // slots
    u8 packSlot = 1;
    // u8 corpseSlot = 1;
    while ( !tmp.empty() )
    {
      Items::Item* bp_item = ITEM_ELEM_PTR( tmp.back() );
      tmp.pop_back();
      bp_item->container = nullptr;
      bp_item->layer = 0;
      UPDATE_CHECKPOINT();
      if ( ( bp_item->newbie() || bp_item->no_drop() || bp_item->use_insurance() ) &&
           bp->can_add( *bp_item ) )
      {
        if ( !bp->can_add_to_slot( packSlot ) || !bp_item->slot_index( packSlot ) )
        {
          _drop_item_to_world( bp_item );
        }
        else
        {
          bp->add( bp_item );
        }
        UPDATE_CHECKPOINT();
      }
      else if ( corpse->can_add( *bp_item ) )
      {
        if ( !corpse->can_add_to_slot( packSlot ) || !bp_item->slot_index( packSlot ) )
        {
          _drop_item_to_world( bp_item );
        }
        else
        {
          corpse->add_at_random_location( bp_item );
        }
        UPDATE_CHECKPOINT();
      }
      else
      {
        UPDATE_CHECKPOINT();
        _drop_item_to_world( bp_item );
      }
      UPDATE_CHECKPOINT();
    }

    UPDATE_CHECKPOINT();

    for ( unsigned layer = Core::LAYER_EQUIP__LOWEST; layer <= Core::LAYER_EQUIP__HIGHEST; ++layer )
    {
      Items::Item* item = wornitems->GetItemOnLayer( layer );
      if ( item == nullptr )
        continue;
      if ( item->layer == Core::LAYER_BACKPACK )  // These needs to be the first!!!!
        continue;
      if ( item->layer == Core::LAYER_BEARD || item->layer == Core::LAYER_HAIR ||
           item->layer == Core::LAYER_FACE )
        continue;
      if ( item->layer != Core::LAYER_MOUNT && item->layer != Core::LAYER_ROBE_DRESS &&
           !item->movable() )
        continue;
      if ( ( item->newbie() || item->no_drop() || item->use_insurance() ) && bp->can_add( *item ) )
      {
        UPDATE_CHECKPOINT();
        if ( Core::settingsManager.ssopt.honor_unequip_script_on_death )
        {
          if ( !item->check_unequiptest_scripts() )
            continue;
          if ( !item->check_unequip_script() )
            continue;
        }
        else
        {
          item->check_unequip_script();
        }
        UPDATE_CHECKPOINT();
        unequip( item );
        item->layer = 0;
        UPDATE_CHECKPOINT();
        if ( !bp->can_add_to_slot( packSlot ) || !item->slot_index( packSlot ) )
        {
          _drop_item_to_world( item );
        }
        else
        {
          bp->add_at_random_location( item );
          update_item_to_inrange( item );
        }
        UPDATE_CHECKPOINT();
      }
    }
    UPDATE_CHECKPOINT();
  }


  UPDATE_CHECKPOINT();
  send_death_message( this, corpse );

  UPDATE_CHECKPOINT();
  corpse->restart_decay_timer();
  UPDATE_CHECKPOINT();
  add_item_to_world( corpse );
  UPDATE_CHECKPOINT();
  send_item_to_inrange( corpse );

  UPDATE_CHECKPOINT();

  clear_opponent_of();

  set_opponent( nullptr );

  UPDATE_CHECKPOINT();

  on_death( corpse );

  CLEAR_CHECKPOINT();
}

void Character::refresh_ar()
{
  //  find_armor(); <-- MuadDib commented out, put code inside here to cut down on iter.
  // Figure out what's in each zone
  //   FIXME? NZONES * NLAYERS (5 * 24 = 124) iterations.
  // okay, reverse, for each wornitem, for each coverage area, upgrade.
  // Turley: should be fixed now only iterators over armor's coverage zones instead of all zones
  for ( unsigned zone = 0; zone < Core::gamestate.armorzones.size(); ++zone )
    armor_[zone] = nullptr;
  // we need to reset each resist to 0, then add the base back using calc.
  resetEquipableProperties();

  for ( unsigned layer = Core::LAYER_EQUIP__LOWEST; layer <= Core::LAYER_EQUIP__HIGHEST; ++layer )
  {
    Items::Item* item = wornitems->GetItemOnLayer( layer );
    if ( item == nullptr )
      continue;
    // Let's check all items as base, and handle their element_resists.
    updateEquipableProperties( item );
    if ( item->isa( Core::UOBJ_CLASS::CLASS_ARMOR ) )
    {
      Items::UArmor* armor = static_cast<Items::UArmor*>( item );
      std::set<unsigned short> tmplzones = armor->tmplzones();
      std::set<unsigned short>::iterator itr;
      for ( itr = tmplzones.begin(); itr != tmplzones.end(); ++itr )
      {
        if ( ( armor_[*itr] == nullptr ) || ( armor->ar() > armor_[*itr]->ar() ) )
          armor_[*itr] = armor;
      }
    }
  }

  //  calculate_ar();  <-- MuadDib Commented out, mixed code within ported find_armor to reduce
  // iter.
  double new_ar = 0.0;
  for ( unsigned zone = 0; zone < Core::gamestate.armorzones.size(); ++zone )
  {
    Items::UArmor* armor = armor_[zone];
    if ( armor != nullptr )
    {
      new_ar += armor->ar() * Core::gamestate.armorzones[zone].chance;
    }
  }

  /* add AR due to shield : parry skill / 2 is percent of AR */
  // FIXME: Should we allow this to be adjustable via a prop? Hrmmmmm
  if ( shield != nullptr )
  {
    double add =
        0.5 * 0.01 * shield->ar() * attribute( Core::gamestate.pAttrParry->attrid ).effective();
    if ( add > 1.0 )
      new_ar += add;
    else
      new_ar += 1.0;
  }

  new_ar += ar_mod();

  short s_new_ar = static_cast<short>( new_ar );
  if ( s_new_ar >= 0 )
    ar_ = s_new_ar;
  else
    ar_ = 0;

  if ( client != nullptr )
  {  // CHECKME consider sending less frequently
    send_full_statmsg( client, this );
  }
}

void Character::updateEquipableProperties( Items::Item* item )
{
  // calc caps
  if ( item->has_defence_increase_cap() )
    defence_increase_cap( defence_increase_cap().addToValue( item->defence_increase_cap() ) );
  // calc resist caps
  if ( item->has_fire_resist_cap() )
    fire_resist_cap( fire_resist_cap().addToValue( item->fire_resist_cap() ) );
  if ( item->has_cold_resist_cap() )
    cold_resist_cap( cold_resist_cap().addToValue( item->cold_resist_cap() ) );
  if ( item->has_energy_resist_cap() )
    energy_resist_cap( energy_resist_cap().addToValue( item->energy_resist_cap() ) );
  if ( item->has_poison_resist_cap() )
    poison_resist_cap( poison_resist_cap().addToValue( item->poison_resist_cap() ) );
  if ( item->has_physical_resist_cap() )
    physical_resist_cap( physical_resist_cap().addToValue( item->physical_resist_cap() ) );
  // calc resists
  if ( item->has_fire_resist() )
    fire_resist( fire_resist().addToValue( item->fire_resist() ) );
  if ( item->has_cold_resist() )
    cold_resist( cold_resist().addToValue( item->cold_resist() ) );
  if ( item->has_energy_resist() )
    energy_resist( energy_resist().addToValue( item->energy_resist() ) );
  if ( item->has_poison_resist() )
    poison_resist( poison_resist().addToValue( item->poison_resist() ) );
  if ( item->has_physical_resist() )
    physical_resist( physical_resist().addToValue( item->physical_resist() ) );

  // calc damages
  if ( item->has_fire_damage() )
    fire_damage( fire_damage().addToValue( item->fire_damage() ) );
  if ( item->has_cold_damage() )
    cold_damage( cold_damage().addToValue( item->cold_damage() ) );
  if ( item->has_energy_damage() )
    energy_damage( energy_damage().addToValue( item->energy_damage() ) );
  if ( item->has_poison_damage() )
    poison_damage( poison_damage().addToValue( item->poison_damage() ) );
  if ( item->has_physical_damage() )
    physical_damage( physical_damage().addToValue( item->physical_damage() ) );

  // calc others
  if ( item->has_lower_reagent_cost() )
    lower_reagent_cost( lower_reagent_cost().addToValue( item->lower_reagent_cost() ) );
  if ( item->has_spell_damage_increase() )
    spell_damage_increase( spell_damage_increase().addToValue( item->spell_damage_increase() ) );
  if ( item->has_faster_casting() )
    faster_casting( faster_casting().addToValue( item->faster_casting() ) );
  if ( item->has_faster_cast_recovery() )
    faster_cast_recovery( faster_cast_recovery().addToValue( item->faster_cast_recovery() ) );
  if ( item->has_lower_mana_cost() )
    lower_mana_cost( lower_mana_cost().addToValue( item->lower_mana_cost() ) );
  if ( item->has_hit_chance() )
    hit_chance( hit_chance().addToValue( item->hit_chance() ) );
  if ( item->has_luck() )
    luck( luck().addToValue( item->luck() ) );
  if ( item->has_swing_speed_increase() )
    swing_speed_increase( swing_speed_increase().addToValue( item->swing_speed_increase() ) );

  // calc defence increase if lower than cap
  if ( item->has_defence_increase() )
    defence_increase( defence_increase().addToValue( item->defence_increase() ) );

  if ( client != nullptr )
  {  // CHECKME consider sending less frequently
    send_full_statmsg( client, this );
  }
}

void Character::resetEquipableProperties()
{
  // reset resists
  if ( has_fire_resist() )
    fire_resist( fire_resist().setAsValue( 0 ) );
  if ( has_cold_resist() )
    cold_resist( cold_resist().setAsValue( 0 ) );
  if ( has_energy_resist() )
    energy_resist( energy_resist().setAsValue( 0 ) );
  if ( has_poison_resist() )
    poison_resist( poison_resist().setAsValue( 0 ) );
  if ( has_physical_resist() )
    physical_resist( physical_resist().setAsValue( 0 ) );

  // reset caps
  if ( has_fire_resist_cap() )
    fire_resist_cap( fire_resist_cap().setAsValue( 0 ) );
  if ( has_cold_resist_cap() )
    cold_resist_cap( cold_resist_cap().setAsValue( 0 ) );
  if ( has_energy_resist_cap() )
    energy_resist_cap( energy_resist_cap().setAsValue( 0 ) );
  if ( has_poison_resist_cap() )
    poison_resist_cap( poison_resist_cap().setAsValue( 0 ) );
  if ( has_physical_resist_cap() )
    physical_resist_cap( physical_resist_cap().setAsValue( 0 ) );

  // reset damages
  if ( has_fire_damage() )
    fire_damage( fire_damage().setAsValue( 0 ) );
  if ( has_cold_damage() )
    cold_damage( cold_damage().setAsValue( 0 ) );
  if ( has_energy_damage() )
    energy_damage( energy_damage().setAsValue( 0 ) );
  if ( has_poison_damage() )
    poison_damage( poison_damage().setAsValue( 0 ) );
  if ( has_physical_damage() )
    physical_damage( physical_damage().setAsValue( 0 ) );

  // reset others
  if ( has_lower_reagent_cost() )
    lower_reagent_cost( lower_reagent_cost().setAsValue( 0 ) );
  if ( has_spell_damage_increase() )
    spell_damage_increase( spell_damage_increase().setAsValue( 0 ) );
  if ( has_faster_casting() )
    faster_casting( faster_casting().setAsValue( 0 ) );
  if ( has_faster_cast_recovery() )
    faster_cast_recovery( faster_cast_recovery().setAsValue( 0 ) );

  if ( has_defence_increase() )
    defence_increase( defence_increase().setAsValue( 0 ) );
  if ( has_defence_increase_cap() )
    defence_increase_cap( defence_increase_cap().setAsValue( 0 ) );
  if ( has_lower_mana_cost() )
    lower_mana_cost( lower_mana_cost().setAsValue( 0 ) );
  if ( has_hit_chance() )
    hit_chance( hit_chance().setAsValue( 0 ) );
  if ( has_luck() )
    luck( luck().setAsValue( 0 ) );
  if ( has_swing_speed_increase() )
    swing_speed_increase( swing_speed_increase().setAsValue( 0 ) );
}

void Character::showarmor() const
{
  if ( client != nullptr )
  {
    Core::send_sysmessage( client, "Your armor coverage:" );
    for ( unsigned i = 0; i < armor_.size(); ++i )
    {
      std::string text = Core::gamestate.armorzones[i].name + ": ";
      if ( armor_[i] == nullptr )
        text += "Nothing";
      else
        text += armor_[i]->name();
      Core::send_sysmessage( client, text );
    }
  }
}

/* check skill: test skill, advance, reset atrophy timers, blah blah..
   obviously, needs work, and more parameters.
   */

bool Character::check_skill( Core::USKILLID skillid, int difficulty, unsigned short pointvalue )
{
  INC_PROFILEVAR( skill_checks );
  static bool in_here = false;
  if ( !in_here && Core::gamestate.system_hooks.check_skill_hook )
  {
    in_here = true;
    bool res = Core::gamestate.system_hooks.check_skill_hook->call(
        new Module::ECharacterRefObjImp( this ), new Bscript::BLong( skillid ),
        new Bscript::BLong( difficulty ), new Bscript::BLong( pointvalue ) );
    in_here = false;
    return res;
  }
  else
  {
    return false;
  }
}

void Character::check_concealment_level()
{
  if ( concealed() > cmdlevel() )
    concealed( cmdlevel() );
}

// you can only be concealed from
// those of lower stature
bool Character::is_concealed_from_me( const Character* chr ) const
{
  return ( chr->concealed() > cmdlevel() );
}

bool Character::is_visible_to_me( const Character* chr ) const
{
  if ( chr == nullptr )
    return false;
  if ( chr == this )
    return true;  // I can always see myself (?)
  if ( is_concealed_from_me( chr ) )
    return false;

  if ( chr->realm() != this->realm() )
    return false;  // noone can see across different realms.
  if ( !chr->logged_in() )
    return false;
  if ( chr->hidden() && !cached_settings.get( PRIV_FLAGS::SEE_HIDDEN ) )
    return false;  // noone can see anyone hidden.
  if ( dead() )
    return true;  // If I'm dead, I can see anything
  if ( !chr->dead() || cached_settings.get( PRIV_FLAGS::SEE_GHOSTS ) )
    return true;  // Anyone can see the living
  if ( chr->warmode() )
    return true;  // Anyone can see someone in warmode
  return false;
};

// NOTE: chr is at new position, lastx/lasty have old position.
void PropagateMove( /*Client *client,*/ Character* chr )
{
  using namespace Network;
  if ( chr == nullptr )
    return;
  RemoveObjectPkt msgremove( chr->serial_ext );
  HealthBarStatusUpdate msgpoison( chr->serial_ext, HealthBarStatusUpdate::Color::GREEN,
                                   chr->poisoned() );
  HealthBarStatusUpdate msginvul( chr->serial_ext, HealthBarStatusUpdate::Color::YELLOW,
                                  chr->invul() );
  PktHelper::PacketOut<PktOut_78> msgcreate;
  MoveChrPkt msgmove( chr );
  build_owncreate( chr, msgcreate.Get() );

  Core::WorldIterator<Core::OnlinePlayerFilter>::InVisualRange(
      chr,
      [&]( Character* zonechr )
      {
        Client* client = zonechr->client;
        if ( zonechr == chr )
          return;
        if ( !zonechr->is_visible_to_me( chr ) )
          return;
        /* The two characters exist, and are in range of each other.
        Character 'chr''s lastx and lasty coordinates are valid.
        SO, if lastx/lasty are out of range of client->chr, we
        should send a 'create' type message.  If they are in range,
        we should just send a move.
        */
        if ( chr->move_reason == Character::MULTIMOVE )
        {
          if ( client->ClientType & Network::CLIENTTYPE_7090 )
          {
            if ( chr->poisoned() )  // if poisoned send 0x17 for newer clients
              msgpoison.Send( client );

            if ( chr->invul() )  // if invul send 0x17 for newer clients
              msginvul.Send( client );
            return;
          }
          else
          {
// NOTE: uncomment this line to make movement smoother (no stepping anims)
// but basically makes it very difficult to talk while the ship
// is moving.
#ifdef PERGON
            send_remove_character( client, chr, msgremove );
#else
// send_remove_character( client, chr );
#endif
            send_owncreate( client, chr, msgcreate.Get() );
            if ( chr->poisoned() )
              msgpoison.Send( client );
            if ( chr->invul() )
              msginvul.Send( client );
          }
        }
        else if ( Core::inrange( zonechr->x(), zonechr->y(), chr->lastx, chr->lasty ) )
        {
          msgmove.Send( client );
          if ( chr->poisoned() )
            msgpoison.Send( client );
          if ( chr->invul() )
            msginvul.Send( client );
        }
        else
        {
          send_owncreate( client, chr, msgcreate.Get() );
          if ( chr->poisoned() )
            msgpoison.Send( client );
          if ( chr->invul() )
            msginvul.Send( client );
        }
      } );

  // iter over all old in range players and send remove
  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      chr->lastx, chr->lasty, chr->realm(), RANGE_VISUAL,
      [&]( Character* zonechr )
      {
        Client* client = zonechr->client;
        if ( !zonechr->is_visible_to_me( chr ) )
          return;

        if ( Core::inrange( zonechr, chr ) )  // already handled
          return;
        // if we just walked out of range of this character, send its
        // client a remove object, or else a ghost character will remain.
        send_remove_character( client, chr, msgremove );
      } );
}

void Character::getpos_ifmove( Plib::UFACING i_facing, unsigned short* px, unsigned short* py )
{
  *px = x() + Core::move_delta[i_facing].xmove;
  *py = y() + Core::move_delta[i_facing].ymove;
}

void Character::swing_task_func( Character* chr )
{
  THREAD_CHECKPOINT( tasks, 800 );
  INFO_PRINT_TRACE( 20 ) << "swing_task_func(0x" << fmt::hexu( chr->serial ) << ")\n";
  chr->mob_flags_.set( MOB_FLAGS::READY_TO_SWING );
  chr->check_attack_after_move();
  THREAD_CHECKPOINT( tasks, 899 );
}

void Character::schedule_attack()
{
  INFO_PRINT_TRACE( 18 ) << "schedule_attack(0x" << fmt::hexu( this->serial ) << ")\n";
  // we'll get here with a swing_task already set, if
  // while in an adjacent cell to your opponent, you turn/move
  // while waiting for your timeout.
  if ( swing_task == nullptr )
  {
    unsigned int weapon_speed = weapon->speed();
    unsigned int weapon_delay = weapon->delay();
    Core::polclock_t clocks;

    if ( !weapon_delay )
    {
      INFO_PRINT_TRACE( 19 ) << "clocks[speed] = (" << Core::POLCLOCKS_PER_SEC << "*15000)/(("
                             << dexterity() << "+100)*" << weapon_speed << ")\n";

      clocks = Core::POLCLOCKS_PER_SEC * static_cast<Core::polclock_t>( 15000L );
      clocks /= ( dexterity() + 100 ) * weapon_speed;
    }
    else
    {
      int delay_sum = weapon_delay + delay_mod();
      if ( delay_sum < 0 )
        delay_sum = 0;

      INFO_PRINT_TRACE( 19 ) << "clocks[delay] = ((" << weapon_delay << "+" << delay_mod() << "="
                             << delay_sum << ")*" << Core::POLCLOCKS_PER_SEC << ")/1000\n";

      clocks = ( delay_sum * Core::POLCLOCKS_PER_SEC ) / 1000;
    }
    // Swing speed modifier can't be less than -1 otherwise it would give a negitive swing speed and
    // thats not possible.
    double speed_modifier = ( swing_speed_increase().sum() / 100.0 );
    if ( speed_modifier < -0.99 )
      speed_modifier = -0.99;
    clocks = static_cast<Core::polclock_t>( round( clocks / ( 1 + speed_modifier ) ) );

    if ( clocks < ( Core::POLCLOCKS_PER_SEC / 5 ) )
    {
      INFO_PRINT_TRACE( 20 ) << name() << " attack timer: " << clocks << "\n";
    }
    INFO_PRINT_TRACE( 19 ) << "clocks=" << clocks << "\n";

    new Core::OneShotTaskInst<Character*>( &swing_task, swing_timer_start_clock_ + clocks + 1,
                                           swing_task_func, this );
  }
}

void Character::reset_swing_timer()
{
  INFO_PRINT_TRACE( 15 ) << "reset_swing_timer(0x" << fmt::hexu( this->serial ) << ")\n";
  mob_flags_.remove( MOB_FLAGS::READY_TO_SWING );

  swing_timer_start_clock_ = Core::polclock();
  if ( swing_task )
    swing_task->cancel();

  if ( opponent_ || !opponent_of.empty() )
  {
    schedule_attack();
  }
}

bool Character::manual_set_swing_timer( Core::polclock_t clocks )
{
  INFO_PRINT_TRACE( 15 ) << "manual_set_swing_timer(0x" << fmt::hexu( this->serial )
                         << ") delay: " << clocks << "\n";
  mob_flags_.remove( MOB_FLAGS::READY_TO_SWING );

  swing_timer_start_clock_ = Core::polclock();
  if ( swing_task )
    swing_task->cancel();

  if ( opponent_ || !opponent_of.empty() )
  {
    new Core::OneShotTaskInst<Character*>( &swing_task, swing_timer_start_clock_ + clocks + 1,
                                           swing_task_func, this );
    return true;
  }
  else
    return false;
}

/* The highlighted character is:
     Your selected_opponent, if you have one.
     If not, then the first char that has you as their opponent.
     Or, noone.
     */
Character* Character::get_opponent() const
{
  if ( opponent_ != nullptr )
    return opponent_;
  else if ( !opponent_of.empty() )
    return *opponent_of.begin();
  else
    return nullptr;
}

bool Character::is_attackable( Character* who ) const
{
  passert( who != nullptr );
  if ( Core::settingsManager.combat_config.scripted_attack_checks )
  {
    INFO_PRINT_TRACE( 21 ) << "is_attackable(0x" << fmt::hexu( this->serial ) << ",0x"
                           << fmt::hexu( who->serial ) << "): will be handled by combat hook.\n";
    return true;
  }
  else
  {
    INFO_PRINT_TRACE( 21 ) << "is_attackable(0x" << fmt::hexu( this->serial ) << ",0x"
                           << fmt::hexu( who->serial ) << "):\n"
                           << "  who->dead:  " << who->dead() << "\n"
                           << "  wpn->inrange: " << weapon->in_range( this, who ) << "\n"
                           << "  hidden:     " << hidden() << "\n"
                           << "  who->hidden:  " << who->hidden() << "\n"
                           << "  concealed:  " << is_concealed_from_me( who ) << "\n";
    if ( who->dead() )
      return false;
    else if ( !weapon->in_range( this, who ) )
      return false;
    else if ( hidden() && !cached_settings.get( PRIV_FLAGS::HIDDEN_ATTACK ) )
      return false;
    else if ( who->hidden() && !cached_settings.get( PRIV_FLAGS::ATTACK_HIDDEN ) )
      return false;
    else if ( is_concealed_from_me( who ) )
      return false;
    else if ( !realm()->has_los( *this, *who ) )
      return false;
    else
      return true;
  }
}

Character* Character::get_attackable_opponent() const
{
  if ( opponent_ != nullptr )
  {
    INFO_PRINT_TRACE( 20 ) << "get_attackable_opponent(0x" << fmt::hexu( this->serial )
                           << "): checking opponent, 0x" << fmt::hexu( opponent_->serial ) << "\n";
    if ( is_attackable( opponent_ ) )
      return opponent_;
  }

  if ( !opponent_of.empty() )
  {
    for ( auto& who : opponent_of )
    {
      INFO_PRINT_TRACE( 20 ) << "get_attackable_opponent(0x" << fmt::hexu( this->serial )
                             << "): checking opponent_of, 0x" << fmt::hexu( who->serial ) << "\n";
      if ( is_attackable( who ) )
        return who;
    }
  }

  return nullptr;
}

void Character::send_highlight() const
{
  if ( client != nullptr && has_active_client() )
  {
    Character* opponent = get_opponent();

    Network::PktHelper::PacketOut<Network::PktOut_AA> msg;
    if ( opponent != nullptr )
      msg->Write<u32>( opponent->serial_ext );
    else
      msg->offset += 4;
    msg.Send( client );
  }
}

void Character::on_swing_failure( Character* /*attacker*/ )
{
  // do nothing
}

void Character::inform_disengaged( Character* /*disengaged*/ )
{
  // someone has just disengaged. If we don't have an explicit opponent,
  // pick one of those that has us targetted as the highlight character.
  if ( opponent_ == nullptr )
    send_highlight();
}

void Character::inform_engaged( Character* /*engaged*/ )
{
  // someone has targetted us.  If we don't have an explicit opponent,
  // pick one of those that has us targetted as the highlight character.
  if ( opponent_ == nullptr )
    send_highlight();
}

void Character::inform_criminal( Character* /*moved*/ )
{
  // virtual that does nothing at character level, but fires event for NPCs
}

void Character::inform_leftarea( Character* /*wholeft*/ )
{
  // virtual that does nothing at character level, but fires event for NPCs
}

void Character::inform_enteredarea( Character* /*whoentered*/ )
{
  // virtual that does nothing at character level, but fires event for NPCs
}

void Character::inform_moved( Character* /*moved*/ )
{
  // consider moving PropagateMove here!
}
void Character::inform_imoved( Character* /*chr*/ ) {}

void Character::set_opponent( Character* new_opponent, bool inform_old_opponent )
{
  INFO_PRINT_TRACE( 12 ) << "set_opponent(0x" << fmt::hexu( this->serial ) << ",0x"
                         << ( new_opponent != nullptr ? fmt::hex( new_opponent->serial ) : '0' )
                         << ")\n";
  if ( new_opponent != nullptr )
  {
    if ( new_opponent->dead() )
      return;

    if ( !warmode() && ( script_isa( Core::POLCLASS_NPC ) || has_active_client() ) )
      set_warmode( true );
  }

  if ( opponent_ != nullptr )
  {
    opponent_->opponent_of.erase( this );
    // Turley 05/26/09 no need to send disengaged event on shutdown
    if ( !Clib::exit_signalled )
    {
      if ( inform_old_opponent && opponent_ != nullptr )
        opponent_->inform_disengaged( this );
    }
  }

  opponent_ = new_opponent;


  // Turley 05/26/09 possible shutdown crashfix during cleanup
  // (inside schedule_attack() the rest is also senseless on shutdowncleanup)
  if ( !Clib::exit_signalled )
  {
    reset_swing_timer();

    if ( opponent_ != nullptr )
    {
      repsys_on_attack( opponent_ );
      if ( opponent_->get_opponent() == nullptr )
        opponent_->reset_swing_timer();

      opponent_->opponent_of.insert( this );

      opponent_->inform_engaged( this );

      opponent_->schedule_attack();
    }

    send_highlight();
  }
}

void Character::select_opponent( u32 opp_serial )
{
  // test for setting to same so swing timer doesn't reset
  // if you double-click the same guy over and over
  if ( opponent_ == nullptr || opponent_->serial != opp_serial )
  {
    Character* new_opponent = Core::find_character( opp_serial );
    if ( new_opponent != nullptr )
    {
      if ( realm() != new_opponent->realm() )
        return;
      set_opponent( new_opponent );
    }
  }
}

void Character::disable_regeneration_for( int seconds )
{
  time_t new_disable_time = Core::poltime() + seconds;
  if ( new_disable_time > disable_regeneration_until )
    disable_regeneration_until = new_disable_time;
}

bool Character::warmode() const
{
  return mob_flags_.get( MOB_FLAGS::WARMODE );
}

void Character::set_warmode( bool i_warmode )
{
  if ( Core::gamestate.system_hooks.warmode_change )
    Core::gamestate.system_hooks.warmode_change->call( new Module::ECharacterRefObjImp( this ),
                                                       new Bscript::BLong( i_warmode ) );

  if ( warmode() != i_warmode )
  {
    disable_regeneration_for( 2 );
  }

  mob_flags_.change( MOB_FLAGS::WARMODE, i_warmode );
  if ( i_warmode == false )
  {
    set_opponent( nullptr );
  }
  reset_swing_timer();

  if ( dead() )
  {
    if ( warmode() )  // if entered warmode, display me now
    {
      send_create_mobile_to_nearby_cansee( this );
    }
    else  // if leaving warmode, I go invisible.
    {
      send_remove_character_to_nearby_cantsee( this );
    }
  }
  else
  {
    Network::MoveChrPkt msgmove( this );
    Core::WorldIterator<Core::OnlinePlayerFilter>::InVisualRange( this,
                                                                  [&]( Character* chr )
                                                                  {
                                                                    if ( chr == this )
                                                                      return;
                                                                    msgmove.Send( chr->client );
                                                                  } );
  }
}

const AttributeValue& Character::weapon_attribute() const
{
  return attribute( weapon->attribute().attrid );
}

unsigned short Character::random_weapon_damage() const
{
  return weapon->get_random_damage();
}

unsigned short Character::min_weapon_damage() const
{
  return weapon->min_weapon_damage();
}

unsigned short Character::max_weapon_damage() const
{
  return weapon->max_weapon_damage();
}

void Character::damage_weapon()
{
  if ( !weapon->is_intrinsic() && !weapon->is_projectile() )
  {
    weapon->reduce_hp_from_hit();
  }
}

Items::UArmor* Character::choose_armor() const
{
  double f = Clib::random_double( Core::gamestate.armor_zone_chance_sum );
  for ( unsigned zone = 0; zone < Core::gamestate.armorzones.size(); ++zone )
  {
    f -= Core::gamestate.armorzones[zone].chance;
    if ( f <= 0.0 )
    {
      return armor_[zone];
    }
  }
  return nullptr;
}

Core::UACTION Character::weapon_anim() const
{
  if ( on_mount() )
    return weapon->mounted_anim();
  else
    return weapon->anim();
}

void Character::do_attack_effects( Character* target )
{
  if ( weapon->is_projectile() )
  {
    // 234 is hit, 238 is miss??
    play_sound_effect( this, weapon->projectile_sound() );
    play_moving_effect( this, target, weapon->projectile_anim(),
                        9,    // Speed (??)
                        0,    // Loop
                        0 );  // Explode
    if ( graphic >= UOBJ_HUMAN_MALE )
    {
      send_action_to_inrange( this, weapon_anim() );
    }
    else
    {
      send_action_to_inrange( this, Core::ACTION_LOOK_AROUND );
    }
  }
  else if ( graphic >= UOBJ_HUMAN_MALE )
  {
    send_action_to_inrange( this, weapon_anim() );
  }
  else
  {
    send_action_to_inrange( this, Core::ACTION_LOOK_AROUND );
  }
}

void Character::do_hit_success_effects()
{
  unsigned short sound = weapon->hit_sound();
  if ( sound )
    play_sound_effect( this, sound );
}

void Character::do_hit_failure_effects()
{
  unsigned short sound = weapon->miss_sound();
  if ( sound )
    play_sound_effect( this, sound );
}

u16 Character::get_damaged_sound() const
{
  if ( gender == Plib::GENDER_MALE )
    return SOUND_EFFECT_MALE_DEFENSE;
  return SOUND_EFFECT_FEMALE_DEFENSE;
}

void Character::do_imhit_effects()
{
  if ( Core::settingsManager.combat_config.core_hit_sounds )
  {
    play_sound_effect( this, get_damaged_sound() );
  }
  if ( objtype_ >= UOBJ_HUMAN_MALE )
    send_action_to_inrange( this, Core::ACTION_GOT_HIT );
}


void Character::attack( Character* opponent )
{
  INC_PROFILEVAR( combat_operations );

  if ( Core::gamestate.system_hooks.attack_hook )
  {
    if ( Core::gamestate.system_hooks.attack_hook->call(
             new Module::ECharacterRefObjImp( this ),
             new Module::ECharacterRefObjImp( opponent ) ) )
      return;
  }

  if ( Core::settingsManager.watch.combat )
    INFO_PRINT << name() << " attacks " << opponent->name() << "\n";

  if ( weapon->is_projectile() )
  {
    Core::UContainer* bp = backpack();
    int check_consume_hook = -1;
    if ( Core::gamestate.system_hooks.consume_ammunition_hook )
    {
      check_consume_hook = Core::gamestate.system_hooks.consume_ammunition_hook->call_long(
          new Module::ECharacterRefObjImp( this ), new Module::EItemRefObjImp( weapon ) );
      if ( check_consume_hook == 0 )
      {
        return;
      }
    }

    if ( ( check_consume_hook == -1 ) &&
         ( ( bp == nullptr ) || ( weapon->consume_projectile( bp ) == false ) ) )
    {
      // 04/2007 - MuadDib
      // Range through wornitems to find containers and check
      // here also if backpack fails. Use the mainpack first this way.
      bool projectile_check = false;
      for ( unsigned layer = Core::LAYER_EQUIP__LOWEST; layer <= Core::LAYER_EQUIP__HIGHEST;
            layer++ )
      {
        Items::Item* item = wornitems->GetItemOnLayer( layer );
        if ( item )
        {
          if ( item != nullptr && item->script_isa( Core::POLCLASS_CONTAINER ) )
          {
            if ( layer != Core::LAYER_HAIR && layer != Core::LAYER_FACE &&
                 layer != Core::LAYER_BEARD && layer != Core::LAYER_BACKPACK &&
                 layer != Core::LAYER_MOUNT )
            {
              Core::UContainer* cont = static_cast<Core::UContainer*>( item );

              if ( weapon->consume_projectile( cont ) == true )
              {
                projectile_check = true;
                break;
              }
            }
          }
        }
      }
      // I'm out of projectiles.
      if ( projectile_check == false )
        return;
    }
  }

  repsys_on_attack( opponent );
  repsys_on_damage( opponent );

  do_attack_effects( opponent );

  if ( Core::gamestate.system_hooks.combat_advancement_hook )
  {
    Core::gamestate.system_hooks.combat_advancement_hook->call(
        new Module::ECharacterRefObjImp( this ), new Module::EItemRefObjImp( weapon ),
        new Module::ECharacterRefObjImp( opponent ) );
  }

  double hit_chance = ( weapon_attribute().effective() + 50.0 ) /
                      ( 2.0 * ( opponent->weapon_attribute().effective() + 50.0 ) );
  hit_chance += hitchance_mod() * 0.001f;
  hit_chance -= opponent->evasionchance_mod() * 0.001f;
  if ( Core::settingsManager.watch.combat )
    INFO_PRINT << "Chance to hit: " << hit_chance << ": ";
  if ( Clib::random_double( 1.0 ) < hit_chance )
  {
    if ( Core::settingsManager.watch.combat )
      INFO_PRINT << "Hit!\n";
    do_hit_success_effects();

    double damage = random_weapon_damage();
    damage_weapon();

    if ( Core::settingsManager.watch.combat )
      INFO_PRINT << "Base damage: " << damage << "\n";

    double damage_multiplier = attribute( Core::gamestate.pAttrTactics->attrid ).effective() + 50;
    damage_multiplier += strength() * 0.20f;
    damage_multiplier *= 0.01f;

    damage *= damage_multiplier;

    if ( Core::settingsManager.watch.combat )
      INFO_PRINT << "Damage multiplier due to tactics/STR: " << damage_multiplier
                 << " Result: " << damage << "\n";

    if ( opponent->shield != nullptr )
    {
      if ( Core::gamestate.system_hooks.parry_advancement_hook )
      {
        Core::gamestate.system_hooks.parry_advancement_hook->call(
            new Module::ECharacterRefObjImp( this ), new Module::EItemRefObjImp( weapon ),
            new Module::ECharacterRefObjImp( opponent ),
            new Module::EItemRefObjImp( opponent->shield ) );
      }

      double parry_chance =
          opponent->attribute( Core::gamestate.pAttrParry->attrid ).effective() / 200.0;
      if ( Core::settingsManager.watch.combat )
        INFO_PRINT << "Parry Chance: " << parry_chance << ": ";
      if ( Clib::random_double( 1.0 ) < parry_chance )
      {
        if ( Core::settingsManager.watch.combat )
          INFO_PRINT << opponent->shield->ar() << " hits deflected\n";
        if ( Core::settingsManager.combat_config.display_parry_success_messages &&
             opponent->client )
          Core::send_sysmessage( opponent->client, "You successfully parried the attack!" );

        damage -= opponent->shield->ar();
        if ( damage < 0 )
          damage = 0;
      }
      else
      {
        if ( Core::settingsManager.watch.combat )
          INFO_PRINT << "failed.\n";
      }
    }
    if ( weapon->hit_script().empty() )
    {
      opponent->apply_damage( damage, this, true,
                              Core::settingsManager.combat_config.send_damage_packet );
    }
    else
    {
      run_hit_script( opponent, damage );
    }
  }
  else
  {
    if ( Core::settingsManager.watch.combat )
      INFO_PRINT << "Miss!\n";
    opponent->on_swing_failure( this );
    do_hit_failure_effects();
    if ( Core::gamestate.system_hooks.hitmiss_hook )
    {
      Core::gamestate.system_hooks.hitmiss_hook->call(
          new Module::ECharacterRefObjImp( this ), new Module::ECharacterRefObjImp( opponent ) );
    }
  }
}

void Character::check_attack_after_move()
{
  FUNCTION_CHECKPOINT( check_attack_after_move, 1 );
  Character* opponent = get_attackable_opponent();
  FUNCTION_CHECKPOINT( check_attack_after_move, 2 );
  INFO_PRINT_TRACE( 20 ) << "check_attack_after_move(0x" << fmt::hexu( this->serial )
                         << "): opponent is 0x"
                         << ( opponent != nullptr ? fmt::hex( opponent->serial ) : '0' ) << "\n";
  if ( opponent != nullptr &&  // and I have an opponent
       !dead() &&              // If I'm not dead
       ( Core::settingsManager.combat_config.attack_while_frozen ||
         ( !paralyzed() && !frozen() ) ) )
  {
    FUNCTION_CHECKPOINT( check_attack_after_move, 3 );
    if ( mob_flags_.get( MOB_FLAGS::READY_TO_SWING ) )  // and I can swing now,
    {                                                   // do so.
      FUNCTION_CHECKPOINT( check_attack_after_move, 4 );
      if ( Core::settingsManager.combat_config.send_swing_packet && client != nullptr )
        send_fight_occuring( client, opponent );

      // we don't want attack() to recursively cause new attacks
      mob_flags_.remove( MOB_FLAGS::READY_TO_SWING );
      attack( opponent );
      FUNCTION_CHECKPOINT( check_attack_after_move, 5 );
      reset_swing_timer();
      FUNCTION_CHECKPOINT( check_attack_after_move, 6 );
    }
    else
    {
      FUNCTION_CHECKPOINT( check_attack_after_move, 7 );
      INFO_PRINT_TRACE( 20 ) << "not ready to swing\n";
      schedule_attack();
      FUNCTION_CHECKPOINT( check_attack_after_move, 8 );
    }
  }
  FUNCTION_CHECKPOINT( check_attack_after_move, 0 );
}


void Character::check_light_region_change()
{
  auto light_unil = lightoverride_until();
  if ( light_unil < Core::read_gameclock() && light_unil != ~0u )
  {
    lightoverride_until( 0 );
    lightoverride( -1 );
  }
  if ( client->gd->weather_region && client->gd->weather_region->lightoverride != -1 &&
       !has_lightoverride() )
    return;

  int newlightlevel;
  if ( has_lightoverride() )
    newlightlevel = lightoverride();
  else
  {
    // dave 12-22 check for no regions
    Core::LightRegion* light_region = Core::gamestate.lightdef->getregion( x(), y(), realm() );
    if ( light_region != nullptr )
      newlightlevel = light_region->lightlevel;
    else
      newlightlevel = Core::settingsManager.ssopt.default_light_level;
  }

  if ( newlightlevel != client->gd->lightlevel )
  {
    Core::send_light( client, newlightlevel );
    client->gd->lightlevel = newlightlevel;
  }
}

void Character::check_justice_region_change()
{
  Core::JusticeRegion* cur_justice_region = client->gd->justice_region;
  Core::JusticeRegion* new_justice_region =
      Core::gamestate.justicedef->getregion( x(), y(), client->chr->realm() );

  if ( cur_justice_region != new_justice_region )
  {
    if ( cur_justice_region != nullptr )
      cur_justice_region->RunLeaveScript( client->chr );
    if ( new_justice_region != nullptr )
      new_justice_region->RunEnterScript( client->chr );

    // print 'leaving' message
    bool printmsgs;
    if ( cur_justice_region != nullptr && new_justice_region != nullptr &&
         cur_justice_region->entertext() == new_justice_region->entertext() &&
         cur_justice_region->leavetext() == new_justice_region->leavetext() )
    {
      printmsgs = false;
    }
    else
    {
      printmsgs = true;
    }

    if ( printmsgs && cur_justice_region )
    {
      const std::string& leavetext = cur_justice_region->leavetext();
      if ( !leavetext.empty() )
      {
        Core::send_sysmessage( client, leavetext );
      }
    }

    client->gd->justice_region = new_justice_region;

    if ( new_justice_region && new_justice_region->RunNoCombatCheck( client ) == true )
    {
      Character* opp2 = get_opponent();
      if ( ( opp2 != nullptr && opp2->client ) )
      {
        opp2->opponent_of.erase( client->chr );
        opp2->set_opponent( nullptr, true );
        opp2->schedule_attack();
        opp2->opponent_ = nullptr;
        opp2->clear_opponent_of();
        set_opponent( nullptr, true );
        if ( swing_task != nullptr )
          swing_task->cancel();
      }
    }


    // print 'entering' message
    // handle nocombat while we have entered.
    if ( printmsgs && new_justice_region )
    {
      const std::string& entertext = new_justice_region->entertext();
      if ( !entertext.empty() )
      {
        Core::send_sysmessage( client, entertext );
      }
    }
  }
}

void Character::check_music_region_change()
{
  Core::MusicRegion* cur_music_region = client->gd->music_region;
  Core::MusicRegion* new_music_region = Core::gamestate.musicdef->getregion( x(), y(), realm() );

  // may want to consider changing every n minutes, too, even if region didn't change
  if ( cur_music_region != new_music_region )
  {
    client->gd->music_region = new_music_region;
    if ( new_music_region != nullptr )
    {
      Core::send_midi( client, new_music_region->getmidi() );
    }
    else
    {
      Core::send_midi( client, 0 );
    }
  }
}

void Character::check_weather_region_change( bool force )  // dave changed 5/26/03 - use force
                                                           // boolean if current weather region
                                                           // changed type/intensity
{
  Core::WeatherRegion* cur_weather_region = client->gd->weather_region;
  Core::WeatherRegion* new_weather_region =
      Core::gamestate.weatherdef->getregion( x(), y(), realm() );

  // eric 5/31/03: I don't think this is right.  it's possible to go from somewhere that has no
  // weather region,
  // and to walk to somewhere that doesn't have a weather region.
  //
  if ( force || ( cur_weather_region != new_weather_region ) )
  {
    if ( new_weather_region != nullptr && new_weather_region->lightoverride != -1 &&
         !has_lightoverride() )
    {
      Core::send_light( client, new_weather_region->lightoverride );
      client->gd->lightlevel = new_weather_region->lightoverride;
    }

    // eric removed this 5/31/03, it's calling itself recursively:
    // move_character_to -> tellmove -> check_region_changes -> check_weather_region_change (here,
    // doh)
    // if you need to send the client something special, just do it.
    // move_character_to(this,x,y,z,0); //dave added 5/26/03: client doesn't refresh properly
    // without a teleport :| and send_goxyz causes weather effects to stop if character is
    // walking/running too

    if ( new_weather_region )
    {
      Core::send_weather( client, new_weather_region->weathertype, new_weather_region->severity,
                          new_weather_region->aux );
    }
    else
    {
      Core::send_weather( client, 0xff, 0, 0 );  // turn off
    }
    client->gd->weather_region = new_weather_region;
  }
}

void Character::check_region_changes()
{
  if ( client != nullptr )
  {
    check_weather_region_change();

    check_light_region_change();

    check_justice_region_change();

    check_music_region_change();
  }
}

void Character::position_changed()
{
  wornitems->setposition( pos() );
}

void Character::unhide()
{
  if ( Core::gamestate.system_hooks.un_hide )
  {
    if ( !Core::gamestate.system_hooks.un_hide->call( make_mobileref( this ) ) )
      return;
  }

  hidden( false );
  if ( is_visible() )
  {
    if ( client != nullptr )
      send_owncreate( client, this );

    Core::WorldIterator<Core::OnlinePlayerFilter>::InVisualRange(
        this,
        [&]( Character* chr )
        {
          if ( chr == this )
            return;
          if ( !chr->is_visible_to_me( this ) )
            return;
          send_owncreate( chr->client, this );
        } );

    realm()->notify_unhid( *this );
  }
}

void Character::set_stealthsteps( unsigned short newval )
{
  stealthsteps_ = newval;
}

bool Character::doors_block() const
{
  return !( graphic == UOBJ_GAMEMASTER || graphic == UOBJ_HUMAN_MALE_GHOST ||
            graphic == UOBJ_HUMAN_FEMALE_GHOST || graphic == UOBJ_ELF_MALE_GHOST ||
            graphic == UOBJ_ELF_FEMALE_GHOST || graphic == UOBJ_GARGOYLE_MALE_GHOST ||
            graphic == UOBJ_GARGOYLE_FEMALE_GHOST ||
            cached_settings.get( PRIV_FLAGS::IGNORE_DOORS ) );
}

/*
    This isn't quite right, for hidden characters.
    What normally happens:
    1) Client sends "use skill hiding"
    2) Server sends "77 move" with the hidden flag set.
    3) client sends move-request
    4) server sends move-approve
    5) server sense "78 create" message with hidden flag clear, at new posn.

    We're sending the "78 create" _before_ the move-approve.
    */

bool Character::can_face( Plib::UFACING /*i_facing*/ )
{
  if ( can_freemove() )
    return true;

  if ( frozen() || paralyzed() )
  {
    if ( client != nullptr )
    {
      if ( frozen() )
        private_say_above( this, this, "I am frozen and cannot move." );
      else if ( paralyzed() )
        private_say_above( this, this, "I am paralyzed and cannot move." );
    }
    return false;
  }

  if ( Core::settingsManager.ssopt.movement_uses_stamina &&
       vital( Core::gamestate.pVitalStamina->vitalid ).current_ones() == 0 && !dead() )
  {
    private_say_above( this, this, "You are too fatigued to move." );
    return false;
  }

  return true;
}


bool Character::face( Plib::UFACING i_facing, int flags )
{
  if ( ( flags & 1 ) == 0 )
  {
    if ( !can_face( i_facing ) )
      return false;
  }

  if ( i_facing != facing )
  {
    setfacing( static_cast<u8>( i_facing ) );

    if ( Core::settingsManager.combat_config.reset_swing_onturn &&
         !cached_settings.get( PRIV_FLAGS::FIRE_WHILE_MOVING ) && weapon->is_projectile() )
      reset_swing_timer();
    if ( hidden() && ( Core::settingsManager.ssopt.hidden_turns_count ) )
    {
      if ( stealthsteps_ == 0 )
        unhide();
      else
        stealthsteps_--;
    }
  }

  return true;
}


bool Character::CustomHousingMove( unsigned char i_dir )
{
  passert( facing < 8 );

  Multi::UMulti* multi = Core::system_find_multi( client->gd->custom_house_serial );
  if ( multi != nullptr )
  {
    Multi::UHouse* house = multi->as_house();
    if ( house != nullptr )
    {
      Plib::UFACING i_facing = static_cast<Plib::UFACING>( i_dir & PKTIN_02_FACING_MASK );
      if ( i_facing != facing )
      {
        setfacing( static_cast<u8>( i_facing ) );
        set_dirty();
        dir = i_dir;
        return true;
      }
      else
      {
        s8 newz = house->z() +
                  Multi::CustomHouseDesign::custom_house_z_xlate_table[house->editing_floor_num];
        u16 newx = x() + Core::move_delta[facing].xmove;
        u16 newy = y() + Core::move_delta[facing].ymove;
        const Multi::MultiDef& def = house->multidef();
        if ( newx > ( house->x() + def.minrx ) && newx <= ( house->x() + def.maxrx ) &&
             newy > ( house->y() + def.minry ) && newy <= ( house->y() + def.maxry ) )
        {
          setposition( Core::Pos4d( static_cast<u16>( newx ), static_cast<u16>( newy ),
                                    static_cast<s8>( newz ), realm() ) );
          MoveCharacterWorldPosition( lastx, lasty, x(), y(), this, nullptr );

          position_changed();
          set_dirty();
          return true;
        }
      }
    }
  }
  return false;
}

//************************************
// Method:    move
// FullName:  Character::move
// Access:    public
// Returns:   bool
// Qualifier:
// Parameter: unsigned char i_dir
//************************************
bool Character::move( unsigned char i_dir )
{
  lastx = x();
  lasty = y();
  lastz = z();

  // if currently building a house chr can move free inside the multi
  if ( is_house_editing() )
    return CustomHousingMove( i_dir );

  u8 oldFacing = facing;

  Plib::UFACING i_facing = static_cast<Plib::UFACING>( i_dir & PKTIN_02_FACING_MASK );
  if ( !face( i_facing ) )
    return false;

  // If we're a player, and we used our "move" command to turn,
  //  we want to skip the meat of this function
  if ( ( script_isa( Core::POLCLASS_NPC ) ) || ( facing == oldFacing ) )
  {
    if ( facing & 1 )  // check if diagonal movement is allowed -- Nando (2009-02-26)
    {
      short new_z;
      u8 tmp_facing = ( facing + 1 ) & 0x7;
      unsigned short tmp_newx = x() + Core::move_delta[tmp_facing].xmove;
      unsigned short tmp_newy = y() + Core::move_delta[tmp_facing].ymove;

      // needs to save because if only one direction is blocked, it shouldn't block ;)
      bool walk1 =
          realm()->walkheight( this, tmp_newx, tmp_newy, z(), &new_z, nullptr, nullptr, nullptr );

      tmp_facing = ( facing - 1 ) & 0x7;
      tmp_newx = x() + Core::move_delta[tmp_facing].xmove;
      tmp_newy = y() + Core::move_delta[tmp_facing].ymove;

      if ( !walk1 && !realm()->walkheight( this, tmp_newx, tmp_newy, z(), &new_z, nullptr, nullptr,
                                           nullptr ) )
        return false;
    }

    unsigned short newx = x() + Core::move_delta[facing].xmove;
    unsigned short newy = y() + Core::move_delta[facing].ymove;

    // FIXME consider consolidating with similar code in UOEMOD.CPP
    short newz;
    Multi::UMulti* supporting_multi;
    Items::Item* walkon_item;

    short current_boost = gradual_boost;
    if ( !realm()->walkheight( this, newx, newy, z(), &newz, &supporting_multi, &walkon_item,
                               &current_boost ) )
      return false;

    remote_containers_.clear();

    if ( !CheckPushthrough() )
      return false;

    if ( !can_freemove() && Core::settingsManager.ssopt.movement_uses_stamina && !dead() )
    {
      int carry_perc = weight() * 100 / carrying_capacity();
      unsigned short tmv = movecost(
          this, carry_perc, ( i_dir & PKTIN_02_DIR_RUNNING_BIT ) ? true : false, on_mount() );
      VitalValue& stamina = vital( Core::gamestate.pVitalStamina->vitalid );
      if ( !consume( Core::gamestate.pVitalStamina, stamina, tmv, VitalDepletedReason::MOVEMENT ) )
      {
        private_say_above( this, this, "You are too fatigued to move." );
        return false;
      }
    }

    // Maybe have a flag for this in servspecopt?
    if ( !cached_settings.get( PRIV_FLAGS::FIRE_WHILE_MOVING ) && weapon->is_projectile() )
      reset_swing_timer();

    setposition( Core::Pos4d( pos() )
                     .x( static_cast<u16>( newx ) )
                     .y( static_cast<u16>( newy ) )
                     .z( static_cast<s8>( newz ) ) );

    if ( on_mount() && !script_isa( Core::POLCLASS_NPC ) )
    {
      mountedsteps_++;
    }
    // FIXME: Need to add Walkon checks for multi right here if type is house.
    if ( supporting_multi != nullptr )
    {
      supporting_multi->register_object( this );
      Multi::UHouse* this_house = supporting_multi->as_house();
      if ( this->registered_house == 0 )
      {
        this->registered_house = supporting_multi->serial;

        if ( this_house != nullptr )
          this_house->walk_on( this );
      }
    }
    else
    {
      if ( registered_house > 0 )
      {
        Multi::UMulti* multi = Core::system_find_multi( registered_house );
        if ( multi != nullptr )
        {
          multi->unregister_object( (UObject*)this );
        }
        registered_house = 0;
      }
    }

    gradual_boost = current_boost;
    MoveCharacterWorldPosition( lastx, lasty, x(), y(), this, nullptr );

    position_changed();
    if ( walkon_item != nullptr )
    {
      walkon_item->walk_on( this );
    }

    if ( hidden() )
    {
      if ( ( ( i_dir & PKTIN_02_DIR_RUNNING_BIT ) &&
             !cached_settings.get( PRIV_FLAGS::RUN_WHILE_STEALTH ) ) ||
           ( stealthsteps_ == 0 ) )
        unhide();
      else if ( stealthsteps_ )
        --stealthsteps_;
    }

    if ( Core::gamestate.system_hooks.ouch_hook )
    {
      if ( ( lastz - z() ) > 21 )
        Core::gamestate.system_hooks.ouch_hook->call(
            make_mobileref( this ), new Bscript::BLong( lastx ), new Bscript::BLong( lasty ),
            new Bscript::BLong( lastz ) );
    }
  }

  set_dirty();
  dir = i_dir;

  return true;


  // this would be a great place for tellmove(), except that
  // we want to send the move acknowledge to the client before
  // sending the move/create messages to the neighboring clients.

  // Why? Maybe to give the best response time to the client.

  // may want to rethink this, since it's starting to complicate
  // things.
}

void Character::realm_changed()
{
  // Commented out the explicit backpack handling, should be handled
  // automagically by wormitems realm handling.  There is a slim
  // possibility that backpacks might be assigned to a character but
  // not be a worn item?  If this is the case, that will be broken.
  //  backpack()->realm = realm;
  //  backpack()->for_each_item(setrealm, (void*)realm);
  wornitems->for_each_item( Core::setrealm, (void*)realm() );
  if ( has_gotten_item() )
    gotten_item()->setposition( Core::Pos4d( pos().xyz(), realm() ) );
  if ( trading_cont.get() )
    trading_cont->setposition( Core::Pos4d( pos().xyz(), realm() ) );

  if ( has_active_client() )
  {
    // these are important to keep here in this order
    Core::send_realm_change( client, realm() );
    Core::send_map_difs( client );
    if ( Core::settingsManager.ssopt.core_sends_season )
      Core::send_season_info( client );
    Core::send_short_statmsg( client, this );
    Core::send_feature_enable( client );
  }
}

bool Character::CheckPushthrough()
{
  if ( !can_freemove() && Core::gamestate.system_hooks.pushthrough_hook )
  {
    unsigned short newx = x() + Core::move_delta[facing].xmove;
    unsigned short newy = y() + Core::move_delta[facing].ymove;
    auto mobs = std::unique_ptr<Bscript::ObjArray>();

    Core::WorldIterator<Core::MobileFilter>::InRange(
        newx, newy, realm(), 0,
        [&]( Mobile::Character* _chr )
        {
          if ( _chr->z() >= z() - 10 && _chr->z() <= z() + 10 && !_chr->dead() &&
               ( is_visible_to_me( _chr ) ||
                 _chr->hidden() ) )  // add hidden mobs even if they're not visible to me
          {
            if ( !mobs )
              mobs.reset( new Bscript::ObjArray );
            mobs->addElement( make_mobileref( _chr ) );
          }
        } );

    if ( mobs )
    {
      return Core::gamestate.system_hooks.pushthrough_hook->call( make_mobileref( this ),
                                                                  mobs.release() );
    }
    return true;
  }
  return true;
}

void Character::tellmove()
{
  check_region_changes();
  PropagateMove( this );

  // notify npcs and items (maybe the PropagateMove should also go there eventually? - Nando
  // 2018-06-16)
  realm()->notify_moved( *this );

  check_attack_after_move();

  if ( opponent_ != nullptr )
    opponent_->check_attack_after_move();

  // attacking can change the opponent_of array drastically.
  std::set<Character*> tmp( opponent_of );
  for ( auto& chr : tmp )
  {
    chr->check_attack_after_move();
  }

  move_reason = OTHER;
}

void Character::add_remote_container( Items::Item* item )
{
  remote_containers_.push_back( Core::ItemRef( item ) );
}

Items::Item* Character::search_remote_containers( u32 find_serial, bool* isRemoteContainer ) const
{
  if ( isRemoteContainer )
    *isRemoteContainer = false;
  for ( const auto& elem : remote_containers_ )
  {
    Items::Item* item = elem.get();
    if ( item->orphan() )
      continue;  // it'll be cleaned up when they move
    if ( item->serial == find_serial )
    {
      if ( isRemoteContainer )
        *isRemoteContainer = true;
      return item;
    }
    if ( item->isa( Core::UOBJ_CLASS::CLASS_CONTAINER ) )
    {
      item = ( (Core::UContainer*)item )->find( find_serial );
      if ( item )
      {
        if ( isRemoteContainer )
          *isRemoteContainer = false;
        return item;
      }
    }
  }
  return nullptr;
}

bool Character::mightsee( const Items::Item* item ) const
{
  while ( item->container != nullptr )
    item = item->container;

  for ( const auto& elem : remote_containers_ )
  {
    Items::Item* additional_item = elem.get();
    if ( additional_item == item )
      return true;
  }


  return ( ( item->realm() == realm() ) && ( abs( x() - item->x() ) <= RANGE_VISUAL ) &&
           ( abs( y() - item->y() ) <= RANGE_VISUAL ) );
}

bool Character::squelched() const
{
  Core::gameclock_t squelched = squelched_until();
  if ( squelched == 0 )
    return false;
  else if ( squelched == ~0u )
    return true;

  if ( Core::read_gameclock() < squelched )
  {
    return true;
  }
  else
  {
    const_cast<Character*>( this )->squelched_until( 0 );
    return false;
  }
}

bool Character::deafened() const
{
  Core::gameclock_t deafened = deafened_until();
  if ( deafened == 0 )
    return false;
  else if ( deafened == ~0u )
    return true;

  if ( Core::read_gameclock() < deafened )
  {
    return true;
  }
  else
  {
    const_cast<Character*>( this )->deafened_until( 0 );
    return false;
  }
}

bool Character::invul() const
{
  return cached_settings.get( PRIV_FLAGS::INVUL );
}

u16 Character::strength() const
{
  return static_cast<u16>( attribute( Core::gamestate.pAttrStrength->attrid ).effective() );
}
u16 Character::dexterity() const
{
  return static_cast<u16>( attribute( Core::gamestate.pAttrDexterity->attrid ).effective() );
}
u16 Character::intelligence() const
{
  return static_cast<u16>( attribute( Core::gamestate.pAttrIntelligence->attrid ).effective() );
}

bool Character::target_cursor_busy() const
{
  if ( tcursor2 != nullptr )
    return true;
  if ( client && client->gd && client->gd->target_cursor_uoemod != nullptr )
    return true;
  return false;
}

// get_legal_item removed, wasn't being used. - MuadDib

void Character::cancel_menu()
{
  menu.clear();
  if ( on_menu_selection != nullptr )
    on_menu_selection( client, nullptr, nullptr );
  on_menu_selection = nullptr;
}

bool Character::is_trading() const
{
  return ( trading_with.get() != nullptr );
}

bool Character::trade_accepted() const
{
  return mob_flags_.get( MOB_FLAGS::TRADE_ACCEPTED );
}

void Character::trade_accepted( bool newvalue )
{
  mob_flags_.change( MOB_FLAGS::TRADE_ACCEPTED, newvalue );
}

void Character::create_trade_container()
{
  if ( trading_cont.get() == nullptr )  // FIXME hardcoded
  {
    Items::Item* cont = Items::Item::create( Core::settingsManager.extobj.secure_trade_container );
    cont->setposition( Core::Pos4d( pos().xyz(), realm() ) );
    trading_cont.set( static_cast<Core::UContainer*>( cont ) );
  }
}

Core::UContainer* Character::trade_container()
{
  return trading_cont.get();
}

// SkillValue removed for no use - MuadDib

const char* Character::target_tag() const
{
  return "mobile";
}

void Character::set_caps_to_default()
{
  for ( unsigned ai = 0; ai < Core::gamestate.numAttributes; ++ai )
  {
    Attribute* pAttr = Core::gamestate.attributes[ai];
    AttributeValue& av = attribute( ai );

    av = attribute( ai );
    av.cap( pAttr->default_cap );
  }
}

u16 Character::last_textcolor() const
{
  return _last_textcolor;
}

void Character::last_textcolor( u16 new_color )
{
  _last_textcolor = new_color;
}

unsigned int Character::guildid() const
{
  auto g = guild();
  return ( g != nullptr ) ? g->guildid() : 0;
}

/**
 * Adds a new buff or overwrites an existing one for the character
 * Sends packets to the client accordingly
 * @author Bodom
 */
void Character::addBuff( u16 icon, u16 duration, u32 cl_name, u32 cl_descr,
                         const std::string& arguments )
{
  // Icon is already present, must send a remove packet first or client will not update
  delBuff( icon );

  Core::gameclock_t end = Core::read_gameclock() + duration;
  buffs_[icon] = { end, cl_name, cl_descr, arguments };

  if ( client != nullptr )
    send_buff_message( this, icon, true, duration, cl_name, cl_descr, arguments );
}

/**
 * Removes a buff for the character
 * Sends packets to the client accordingly
 * @author Bodom
 * @return True when the buff has been found and removed, False when the buff was not present
 */
bool Character::delBuff( u16 icon )
{
  auto b = buffs_.find( icon );

  if ( b == buffs_.end() )
    return false;

  buffs_.erase( b );
  if ( client != nullptr )
    send_buff_message( this, icon, false );
  return true;
}

/**
 * Removes al buffs for the character
 * Sends packets to the client accordingly
 * @author Bodom
 */
void Character::clearBuffs()
{
  for ( auto it = buffs_.begin(); it != buffs_.end(); ++it )
    delBuff( it->first );
}

/**
 * Resends all buffs (with updated duration), usually called at (re)login
 * @author Bodom
 */
void Character::send_buffs()
{
  if ( client == nullptr )
    return;

  for ( auto it = buffs_.begin(); it != buffs_.end(); ++it )
  {
    int duration = it->second.end - Core::read_gameclock();
    if ( duration < 0 )
      duration = 0;
    else if ( duration > 0xFFFF )
      duration = 0xFFFF;

    send_buff_message( this, it->first, true, static_cast<u16>( duration ), it->second.cl_name,
                       it->second.cl_descr, it->second.arguments );
  }
}

size_t Character::estimatedSize() const
{
  size_t size = base::estimatedSize() + uclang.capacity() + privs.estimatedSize() +
                settings.estimatedSize() + sizeof( Core::AccountRef ) /*acct*/
                + sizeof( Network::Client* )                          /*client*/
                + sizeof( u32 )                                       /*registered_house*/
                + sizeof( unsigned char )                             /*cmdlevel_*/
                + sizeof( u8 )                                        /*dir*/
                + sizeof( u16 )                                       /*lastx*/
                + sizeof( u16 )                                       /*lasty*/
                + sizeof( s8 )                                        /*lastz*/
                + sizeof( MOVEREASON )                                /*move_reason*/
                + sizeof( Plib::MOVEMODE )                            /*movemode*/
                + sizeof( time_t )                                    /*disable_regeneration_until*/
                + sizeof( u16 )                                       /*truecolor*/
                + sizeof( u32 )                                       /*trueobjtype*/
                + sizeof( Plib::UGENDER )                             /*gender*/
                + sizeof( Plib::URACE )                               /*race*/
                + sizeof( short )                                     /*gradual_boost*/
                + sizeof( u32 )                                       /*last_corpse*/
                + sizeof( GOTTEN_ITEM_TYPE )                          /*gotten_item_source*/
                + sizeof( Core::TargetCursor* )                       /*tcursor2*/
                + sizeof( weak_ptr<Core::Menu> )                      /*menu*/
                + sizeof( u16 )                                       /*_last_textcolor*/
                + sizeof( ref_ptr<Core::WornItemsContainer> )         /*wornitems_ref*/
                + sizeof( unsigned short )                            /*ar_*/
                + sizeof( Items::UWeapon* )                           /*weapon*/
                + sizeof( Items::UArmor* )                            /*shield*/
                + sizeof( unsigned char )                             /*concealed_*/
                + sizeof( unsigned short )                            /*stealthsteps_*/
                + sizeof( unsigned int )                              /*mountedsteps_*/
                + privs.estimatedSize() + settings.estimatedSize() +
                sizeof( Core::UOExecutor* )                  /*script_ex*/
                + sizeof( Character* )                       /*opponent_*/
                + sizeof( Core::polclock_t )                 /*swing_timer_start_clock_*/
                + sizeof( Core::OneShotTask* )               /*swing_task*/
                + sizeof( Core::OneShotTask* )               /*spell_task*/
                + sizeof( Core::gameclock_t )                /*created_at*/
                + sizeof( Core::polclock_t )                 /*criminal_until_*/
                + sizeof( Core::OneShotTask* )               /*repsys_task_*/
                + sizeof( Core::OneShotTask* )               /*party_decline_timeout_*/
                + sizeof( Core::AttributeFlags<PRIV_FLAGS> ) /*cached_settings*/
                + sizeof( Core::AttributeFlags<MOB_FLAGS> )  /*mob_flags_*/
      ;

  size += 3 * sizeof( AttributeValue* ) + attributes.capacity() * sizeof( AttributeValue );
  size += 3 * sizeof( VitalValue* ) + vitals.capacity() * sizeof( VitalValue );
  size += 3 * sizeof( Items::UArmor** ) + armor_.capacity() * sizeof( Items::UArmor* );
  size += 3 * sizeof( Core::ItemRef* ) + remote_containers_.capacity() * sizeof( Core::ItemRef );

  size += 3 * sizeof( void* ) + opponent_of.size() * ( sizeof( Character* ) + 3 * sizeof( void* ) );

  size += aggressor_to_.size() * ( sizeof( Core::CharacterRef ) + sizeof( Core::polclock_t ) +
                                   ( sizeof( void* ) * 3 + 1 ) / 2 );
  size += lawfully_damaged_.size() * ( sizeof( Core::CharacterRef ) + sizeof( Core::polclock_t ) +
                                       ( sizeof( void* ) * 3 + 1 ) / 2 );

  size +=
      3 * sizeof( void* ) + to_be_reportable_.size() * ( sizeof( USERIAL ) + 3 * sizeof( void* ) );
  size +=
      3 * sizeof( void* ) + reportable_.size() * ( sizeof( reportable_t ) + 3 * sizeof( void* ) );

  size +=
      3 * sizeof( void* ) + buffs_.size() * ( sizeof( u16 ) + sizeof( Buff ) + sizeof( void* ) );

  return size;
}

void Character::on_delete_from_account()
{
  if ( realm() )
    realm()->remove_mobile( *this, Realms::WorldChangeReason::PlayerDeleted );
}

bool Character::get_method_hook( const char* methodname, Bscript::Executor* ex,
                                 Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.character_method_script.get(), methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}


AttributeValue::AttributeValue()
    : _base( 0 ), _temp( 0 ), _intrinsic( 0 ), _lockstate( 0 ), _cap( 0 )
{
}
int AttributeValue::effective() const
{
  int v = _base;
  v += _temp;
  v += _intrinsic;
  return ( v > 0 ) ? ( v / 10 ) : 0;
}
int AttributeValue::effective_tenths() const
{
  int v = _base;
  v += _temp;
  v += _intrinsic;
  return ( v > 0 ) ? v : 0;
}
int AttributeValue::base() const
{
  return _base;
}
void AttributeValue::base( unsigned short base )
{
  passert( base <= ATTRIBUTE_MAX_BASE );
  _base = base;
}
int AttributeValue::temp_mod() const
{
  return _temp;
}
void AttributeValue::temp_mod( short temp )
{
  _temp = temp;
}
int AttributeValue::intrinsic_mod() const
{
  return _intrinsic;
}
void AttributeValue::intrinsic_mod( short val )
{
  _intrinsic = val;
}
unsigned char AttributeValue::lock() const
{
  return _lockstate;
}
void AttributeValue::lock( unsigned char lockstate )
{
  _lockstate = lockstate;
}
unsigned short AttributeValue::cap() const
{
  return _cap;
}
void AttributeValue::cap( unsigned short cap )
{
  _cap = cap;
}

VitalValue::VitalValue() : _current( 0 ), _maximum( 0 ), _regenrate( 0 ) {}
int VitalValue::current() const
{
  return _current;
}
int VitalValue::current_ones() const
{
  return _current / 100;
}
int VitalValue::current_thousands() const
{
  // use division to prevent overflow
  return ( _current / 100 ) * 1000 / ( _maximum / 100 );
}
int VitalValue::maximum() const
{
  return _maximum;
}
int VitalValue::maximum_ones() const
{
  return _maximum / 100;
}
bool VitalValue::is_at_maximum() const
{
  return ( _current >= _maximum );
}
int VitalValue::regenrate() const
{
  return _regenrate;
}
void VitalValue::current( int cur )
{
  _current = cur;
  if ( _current > _maximum )
    _current = _maximum;
}
void VitalValue::current_ones( int ones )
{
  current( ones * 100 );
}
void VitalValue::maximum( int val )
{
  _maximum = val;
  if ( _current > _maximum )
    current( _maximum );
}
void VitalValue::regenrate( int rate )
{
  _regenrate = rate;
}
bool VitalValue::consume( unsigned int hamt )
{
  if ( _current > hamt )
  {
    _current -= hamt;
    return true;
  }
  _current = 0;
  return false;
}
void VitalValue::produce( unsigned int hamt )
{
  unsigned newcur = _current + hamt;
  if ( newcur > _maximum || newcur < _current )
  {
    _current = _maximum;
  }
  else
  {
    _current = newcur;
  }
}

}  // namespace Mobile
}  // namespace Pol
