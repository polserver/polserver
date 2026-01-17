/** @file
 *
 * @par History
 * - 2009/09/20 MuadDib:   docast no longer unhides. Let scripts handle this.
 * - 2010/01/15 Turley:    (Tomi) SpeakPowerWords font and color params
 */


#include "spells.h"

#include <iterator>
#include <stdlib.h>
#include <time.h>

#include "../bscript/eprog.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/refptr.h"
#include "../clib/stlutil.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include "containr.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "layers.h"
#include "mkscrobj.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/pktin.h"
#include "polclass.h"
#include "polsig.h"
#include "regions/miscrgn.h"
#include "scrstore.h"
#include "skillid.h"
#include "spelbook.h"
#include "syshook.h"
#include "ufunc.h"
#include "umanip.h"
#include "vital.h"


namespace Pol::Core
{
static bool nocast_here( Mobile::Character* chr )
{
  NoCastRegion* rgn = gamestate.nocastdef->getregion( chr->pos() );
  if ( rgn == nullptr )
  {
    return false;
  }
  else
  {
    return rgn->nocast();
  }
}

bool knows_spell( Mobile::Character* chr, u16 spellid )
{
  // copied code from Character::spellbook to support multiple spellbooks in the pack
  Items::Item* item = chr->wornitem( LAYER_HAND1 );
  if ( item != nullptr && item->script_isa( POLCLASS_SPELLBOOK ) )
  {
    Spellbook* book = static_cast<Spellbook*>( item );
    if ( book->has_spellid( spellid ) )
      return true;
  }

  UContainer* cont = chr->backpack();
  if ( cont != nullptr )
  {
    for ( UContainer::const_iterator itr = cont->begin(), end = cont->end(); itr != end; ++itr )
    {
      const Items::Item* _item = *itr;

      if ( _item != nullptr && _item->script_isa( POLCLASS_SPELLBOOK ) )
      {
        const Spellbook* book = static_cast<const Spellbook*>( _item );
        if ( book->has_spellid( spellid ) )
          return true;
      }
    }
  }

  return false;
}

bool hands_are_free( Mobile::Character* chr )
{
  Items::Item* item;

  item = chr->wornitem( LAYER_HAND1 );
  if ( item != nullptr )
  {
    const Items::ItemDesc& id = item->itemdesc();
    if ( id.blocks_casting_if_in_hand )
      return false;
  }

  item = chr->wornitem( LAYER_HAND2 );
  if ( item != nullptr )
  {
    const Items::ItemDesc& id = item->itemdesc();
    if ( id.blocks_casting_if_in_hand )
      return false;
  }

  return true;
}

USpellParams::USpellParams() : manacost( 0 ), difficulty( 0 ), pointvalue( 0 ), delay( 0 ) {}

USpellParams::USpellParams( Clib::ConfigElem& elem )
    : manacost( elem.remove_ushort( "MANA" ) ),
      difficulty( elem.remove_ushort( "DIFFICULTY" ) ),
      pointvalue( elem.remove_ushort( "POINTVALUE" ) ),
      delay( elem.remove_ushort( "DELAY" ) )
{
}


SpellCircle::SpellCircle( Clib::ConfigElem& elem ) : params( elem ) {}

USpell::USpell( Clib::ConfigElem& elem, Plib::Package* pkg )
    : pkg_( pkg ),
      spellid_( elem.remove_ushort( "SPELLID" ) ),
      name_( elem.remove_string( "NAME" ) ),
      power_words_( elem.remove_string( "POWERWORDS" ) ),
      scriptdef_( elem.remove_string( "SCRIPT", "" ), pkg, "scripts/" )
{
  unsigned short action;
  if ( elem.remove_prop( "ANIMATION", &action ) )
  {
    if ( UACTION_IS_VALID( action ) )
    {
      action_ = static_cast<UACTION>( action );
    }
    else
    {
      elem.throw_error( "Animation is out of range" );
    }
  }
  else
  {
    action_ = ACTION_CAST_SPELL1;
  }

  unsigned short circle;
  if ( elem.remove_prop( "CIRCLE", &circle ) )
  {
    if ( circle < 1 || circle > gamestate.spellcircles.size() ||
         gamestate.spellcircles[circle - 1] == nullptr )
    {
      ERROR_PRINTLN( "Error reading spell {}: Circle {} is not defined.", name_, circle );
      throw std::runtime_error( "Config file error" );
    }

    params_ = gamestate.spellcircles[circle - 1]->params;
  }
  else
  {
    params_ = USpellParams( elem );
  }

  std::string reagent_name;
  while ( elem.remove_prop( "Reagent", &reagent_name ) )
  {
    unsigned int reagent = Items::get_objtype_from_string( reagent_name );

    reglist_.push_back( reagent );
  }
}

size_t USpell::estimateSize() const
{
  size_t size = sizeof( Plib::Package* )                                         /*pkg_*/
                + sizeof( unsigned short )                                       /*spellid_*/
                + name_.capacity() + power_words_.capacity() + sizeof( UACTION ) /*action_*/
                + Clib::memsize( reglist_ ) + sizeof( USpellParams )             /*params_*/
                + scriptdef_.estimatedSize();
  return size;
}

void USpell::cast( Mobile::Character* chr )
{
  if ( nocast_here( chr ) )
  {
    if ( chr->client != nullptr )
      send_sysmessage( chr->client, "Spells cannot be cast here." );
    return;
  }

  if ( !scriptdef_.empty() )
  {
    ref_ptr<Bscript::EScriptProgram> prog =
        find_script2( scriptdef_, true, Plib::systemstate.config.cache_interactive_scripts );

    if ( prog.get() != nullptr )
    {
      if ( chr->start_spell_script( prog.get(), this ) )
        return;
    }
  }

  if ( chr->client != nullptr )
    send_sysmessage( chr->client, "That spell doesn't seem to work." );
}

bool USpell::consume_reagents( Mobile::Character* chr )
{
  UContainer* bp = chr->backpack();
  if ( bp == nullptr )
    return false;

  for ( RegList::iterator itr = reglist_.begin(), end = reglist_.end(); itr != end; ++itr )
  {
    Items::Item* item = bp->find_objtype_noninuse( *itr );
    if ( item == nullptr )
      return false;
    subtract_amount_from_item( item, 1 );
  }

  return true;
}

bool USpell::check_mana( Mobile::Character* chr )
{
  return ( chr->vital( gamestate.pVitalMana->vitalid ).current_ones() >= manacost() );
}

void USpell::consume_mana( Mobile::Character* chr )
{
  chr->consume( gamestate.pVitalMana, chr->vital( gamestate.pVitalMana->vitalid ), manacost() * 100,
                Mobile::Character::VitalDepletedReason::SCRIPT );
}

void USpell::speak_power_words( Mobile::Character* chr, unsigned short font, unsigned short color )
{
  if ( chr->client != nullptr && chr->hidden() )
  {
    private_say_above( chr, chr, power_words_.c_str(), font, color );
  }
  else if ( !chr->hidden() )
  {
    say_above( chr, power_words_.c_str(), font, color );
  }
}

SpellTask::SpellTask( OneShotTask** handle, polclock_t run_when_clock, Mobile::Character* caster,
                      USpell* spell, bool /*dummy*/ )
    : OneShotTask( handle, run_when_clock ), caster_( caster ), spell_( spell )
{
}

void SpellTask::on_run()
{
  THREAD_CHECKPOINT( tasks, 900 );
  Mobile::Character* caster = caster_.get();
  if ( !caster->orphan() )
  {
    THREAD_CHECKPOINT( tasks, 911 );
    spell_->cast( caster );
    THREAD_CHECKPOINT( tasks, 912 );
  }
  THREAD_CHECKPOINT( tasks, 999 );
}

void do_cast( Network::Client* client, u16 spellid )
{
  if ( gamestate.system_hooks.on_cast_hook )
  {
    if ( gamestate.system_hooks.on_cast_hook->call( make_mobileref( client->chr ),
                                                    new Bscript::BLong( spellid ) ) )
      return;
  }
  // CHECKME should this look at spellnum, instead? static_cast behavior undefined if out of range.
  if ( spellid >= gamestate.spells.size() )
    return;

  USpell* spell = gamestate.spells[spellid];
  if ( spell == nullptr )
  {
    ERROR_PRINTLN( "Spell {} is not implemented.", spellid );
    send_sysmessage( client, "That spell does not function." );
    return;
  }

  // Let scripts handle this.
  // if (client->chr->hidden())
  //   client->chr->unhide();

  if ( client->chr->frozen() )
  {
    private_say_above( client->chr, client->chr, "I am frozen and cannot cast spells" );
    return;
  }

  if ( client->chr->paralyzed() )
  {
    private_say_above( client->chr, client->chr, "I am paralyzed and cannot cast spells" );
    return;
  }

  if ( client->chr->skill_ex_active() )
  {
    send_sysmessage( client, "You are already doing something else." );
    return;
  }

  if ( client->chr->casting_spell() )
  {
    send_sysmessage( client, "You are already casting a spell." );
    return;
  }

  if ( nocast_here( client->chr ) )
  {
    send_sysmessage( client, "Spells cannot be cast here." );
    return;
  }

  if ( Plib::systemstate.config.require_spellbooks )
  {
    if ( !knows_spell( client->chr, spellid ) )
    {
      send_sysmessage( client, "You don't know that spell." );
      return;
    }
  }

#if 1
  client->chr->schedule_spell( spell );
#else
  spell->cast( client );
  client->restart();
#endif
}

void handle_cast_spell( Network::Client* client, PKTIN_12* msg )
{
  u16 spellnum = static_cast<u16>( strtoul( (char*)msg->data, nullptr, 10 ) );

  do_cast( client, spellnum );
}


void handle_open_spellbook( Network::Client* client, PKTIN_12* /*msg*/ )
{
  if ( gamestate.system_hooks.open_spellbook_hook != nullptr )
  {
    if ( gamestate.system_hooks.open_spellbook_hook->call( make_mobileref( client->chr ) ) )
      return;
  }

  if ( client->chr->dead() )
  {
    send_sysmessage( client, "I am dead and cannot do that." );
    return;
  }


  Items::Item* spellbook = client->chr->wornitem( LAYER_HAND1 );
  if ( spellbook == nullptr )
  {
    UContainer* backpack = client->chr->backpack();
    if ( backpack != nullptr )
    {
      spellbook = backpack->find_toplevel_polclass( POLCLASS_SPELLBOOK );

      //
      // Client crashes if the pack isn't open and you don't tell him
      // about the spellbook
      //
      if ( spellbook != nullptr )
        send_put_in_container( client, spellbook );
    }
  }

  if ( spellbook != nullptr )
  {
    spellbook->double_click( client );
  }
}

void register_spell( USpell* spell, unsigned short spellid )
{
  if ( spellid >= gamestate.spells.size() )
  {
    gamestate.spells.resize( spellid + 1, nullptr );
  }

  if ( gamestate.spells[spellid] )
  {
    USpell* origspell = gamestate.spells[spellid];
    std::string tmp =
        fmt::format( "Spell ID {} ({}) multiply defined", spellid, origspell->name() );
    if ( origspell->pkg_ != nullptr )
    {
      fmt::format_to( std::back_inserter( tmp ),
                      "\n  Spell originally defined in package '{}' ({})", origspell->pkg_->name(),
                      origspell->pkg_->dir() );
    }
    else
    {
      tmp += "\n  Spell originally defined in main";
    }
    if ( spell->pkg_ != nullptr )
    {
      fmt::format_to( std::back_inserter( tmp ), "\n  Spell redefined in package '{}' ({})",
                      spell->pkg_->name(), spell->pkg_->dir() );
    }
    else
    {
      tmp += "\n  Spell redefined in main";
    }
    ERROR_PRINTLN( tmp );
    throw std::runtime_error( "Spell ID multiply defined" );
  }

  gamestate.spells[spellid] = spell;
}


void load_circle_data()
{
  if ( !Clib::FileExists( "config/circles.cfg" ) )
  {
    if ( Plib::systemstate.config.loglevel > 1 )
      INFO_PRINTLN( "File config/circles not found, skipping." );
    return;
  }

  Clib::ConfigFile cf( "config/circles.cfg", "Circle" );
  Clib::ConfigElem elem;

  while ( cf.read( elem ) )
  {
    int index = strtoul( elem.rest(), nullptr, 0 ) - 1;
    if ( index < 0 || index >= 100 )
    {
      ERROR_PRINTLN( "Error in CIRCLES.CFG: Circle must fall between 1 and 100" );
      throw std::runtime_error( "Config file error" );
    }

    gamestate.spellcircles.resize( index + 1, nullptr );

    if ( gamestate.spellcircles[index] != nullptr )
    {
      ERROR_PRINTLN( "Error in CIRCLES.CFG: Circle {} is multiply defined.", index + 1 );
      throw std::runtime_error( "Config file error" );
    }

    gamestate.spellcircles[index] = new SpellCircle( elem );
  }
}

void load_spells_cfg( const char* path, Plib::Package* pkg )
{
  Clib::ConfigFile cf( path, "Spell" );
  Clib::ConfigElem elem;

  while ( cf.read( elem ) )
  {
    std::unique_ptr<USpell> spell( new USpell( elem, pkg ) );

    unsigned short spellid = spell->spell_id();

    register_spell( spell.release(), spellid );
  }
}

void load_spell_data()
{
  load_circle_data();

  if ( Clib::FileExists( "config/spells.cfg" ) )
    load_spells_cfg( "config/spells.cfg", nullptr );
  else if ( Plib::systemstate.config.loglevel > 1 )
    INFO_PRINTLN( "File config/spells.cfg not found, skipping" );

  for ( Plib::Packages::iterator itr = Plib::systemstate.packages.begin();
        itr != Plib::systemstate.packages.end(); ++itr )
  {
    Plib::Package* pkg = ( *itr );
    std::string filename = Plib::GetPackageCfgPath( pkg, "spells.cfg" );
    if ( Clib::FileExists( filename.c_str() ) )
    {
      load_spells_cfg( filename.c_str(), pkg );
    }
  }
}

void clean_spells()
{
  std::vector<SpellCircle*>::iterator c_iter = gamestate.spellcircles.begin();
  for ( ; c_iter != gamestate.spellcircles.end(); ++c_iter )
  {
    delete *c_iter;
    *c_iter = nullptr;
  }
  gamestate.spellcircles.clear();
  std::vector<USpell*>::iterator s_iter = gamestate.spells.begin();
  for ( ; s_iter != gamestate.spells.end(); ++s_iter )
  {
    delete *s_iter;
    *s_iter = nullptr;
  }
  gamestate.spells.clear();
}
}  // namespace Pol::Core
