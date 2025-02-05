/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/09/15 MuadDib:   Multi registration/unregistration support added.
 */


#include "core.h"

#include <stddef.h>

#include "../bscript/config.h"
#include "../clib/Debugging/ExceptionParser.h"
#include "../clib/clib.h"
#include "../clib/compilerspecifics.h"
#include "../clib/mdump.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../plib/systemstate.h"
#include "fnsearch.h"
#include "globals/settings.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "mobile/charactr.h"
#include "multi/house.h"
#include "multi/multi.h"
#include "network/cgdata.h"
#include "network/client.h"
#include "proplist.h"
#include "realms/realm.h"
#include "ufunc.h"
#include "uworld.h"

namespace Pol
{
namespace Core
{

void apply_polcfg( bool initial )
{
  auto& config = Plib::systemstate.config;
  if ( initial )
  {
    if ( config.account_save > 0 )
    {
      gamestate.write_account_task->set_secs( config.account_save );
      gamestate.write_account_task->start();
    }
  }
  Bscript::escript_config.max_call_depth = config.max_call_depth;
  Clib::passert_dump_stack = config.passert_dump_stack;

  if ( config.passert_failure_action == "abort" )
  {
    Clib::passert_shutdown = false;
    Clib::passert_nosave = false;
    Clib::passert_abort = true;
  }
  else if ( config.passert_failure_action == "continue" )
  {
    Clib::passert_shutdown = false;
    Clib::passert_nosave = false;
    Clib::passert_abort = false;
  }
  else if ( config.passert_failure_action == "shutdown" )
  {
    Clib::passert_shutdown = true;
    Clib::passert_nosave = false;
    Clib::passert_abort = false;
  }
  else if ( config.passert_failure_action == "shutdown-nosave" )
  {
    Clib::passert_shutdown = true;
    Clib::passert_nosave = true;
    Clib::passert_abort = false;
  }
  else
  {
    Clib::passert_shutdown = false;
    Clib::passert_abort = true;
    POLLOG_ERRORLN(
        "Unknown pol.cfg AssertionFailureAction value: {} (expected abort, continue, shutdown, or "
        "shutdown-nosave)",
        config.passert_failure_action );
  }

  Clib::LogfileTimestampEveryLine = config.logfile_timestamp_everyline;
  if ( !config.enable_debug_log )
    DISABLE_DEBUGLOG();

#ifdef _WIN32
  Clib::MiniDumper::SetMiniDumpType( config.minidump_type );
#endif

  Clib::ExceptionParser::configureProgramAbortReportingSystem(
      config.report_active, config.report_server, config.report_url, config.report_admin_email );


  /// The profiler needs to gather some data before the pol.cfg file gets loaded, so when it
  /// turns out to be disabled, or when it was enabled before, but is being disabled now,
  /// run "garbage collection" to free the allocated resources
  if ( !config.profile_cprops )
    Core::CPropProfiler::instance().clear();

  settingsManager.ssopt.features.updateFromPolCfg( config.character_slots );
}


void SetSysTrayPopupText( const char* text );
#ifdef _WIN32
static Priority tipPriority = ToolTipPriorityNone;
#endif
void CoreSetSysTrayToolTip( const std::string& text, Priority priority )
{
#ifdef _WIN32
  if ( priority >= tipPriority )
  {
    tipPriority = priority;
    SetSysTrayPopupText( text.c_str() );
  }
#else
  (void)text;
  (void)priority;
#endif
}

bool move_character_to( Mobile::Character* chr, Pos4d newpos, int flags )
{
  // FIXME consider consolidating with similar code in CHARACTER.CPP
  short newz;
  Multi::UMulti* supporting_multi = nullptr;
  Items::Item* walkon_item = nullptr;
  short new_boost = 0;

  if ( flags & MOVEITEM_FORCELOCATION )
  {
    newpos.realm()->walkheight( newpos.xy(), newpos.z(), &newz, &supporting_multi, &walkon_item,
                                true, chr->movemode, &new_boost );
  }
  else
  {
    if ( !newpos.realm()->walkheight( chr, newpos.xy(), newpos.z(), &newz, &supporting_multi,
                                      &walkon_item, &new_boost ) )
    {
      return false;
    }
    newpos.z( Clib::clamp_convert<s8>( newz ) );
  }
  chr->set_dirty();
  Pos4d oldpos = chr->pos();

  if ( ( chr->realm() != nullptr ) && ( chr->realm() != newpos.realm() ) )
  {
    // Notify NPCs in the old realm that the player left the realm.
    oldpos.realm()->notify_left( *chr );

    send_remove_character_to_nearby( chr );
    if ( chr->client != nullptr )
      remove_objects_inrange( chr->client );
    chr->setposition( newpos );
    chr->realm_changed();
    chr->lastpos = oldpos;
  }
  else
  {
    chr->lastpos = oldpos;
    chr->setposition( newpos );
  }
  MoveCharacterWorldPosition( oldpos, chr );

  chr->gradual_boost = new_boost;
  chr->position_changed();
  if ( supporting_multi != nullptr )
  {
    supporting_multi->register_object( chr );
    if ( chr->registered_multi == 0 )
    {
      chr->registered_multi = supporting_multi->serial;
      supporting_multi->walk_on( chr );
    }
  }
  else
  {
    if ( chr->registered_multi > 0 )
    {
      Multi::UMulti* multi = system_find_multi( chr->registered_multi );
      if ( multi != nullptr )
      {
        multi->unregister_object( chr );
      }
      chr->registered_multi = 0;
    }
  }

  if ( chr->has_active_client() )
  {
    passert_assume( chr->client !=
                    nullptr );  // tells compiler to assume this is true during static code analysis

    if ( oldpos.realm() != chr->realm() )
      send_new_subserver( chr->client );
    send_owncreate( chr->client, chr );
    send_goxyz( chr->client, chr );

    // send_goxyz seems to stop the weather.  This will force a refresh, if the client cooperates.
    chr->client->gd->weather_region = nullptr;
  }
  if ( chr->isa( UOBJ_CLASS::CLASS_NPC ) ||
       chr->client )  // dave 3/26/3 dont' tell moves of offline PCs
  {
    chr->tellmove();
  }
  if ( chr->has_active_client() )
  {
    send_objects_newly_inrange( chr->client );
    chr->check_light_region_change();
  }
  if ( walkon_item != nullptr )
  {
    walkon_item->walk_on( chr );
  }
  // TODO pos why?
  /*
    chr->lastx = chr->x();
    chr->lasty = chr->y();
    chr->lastz = chr->z();
  */
  return true;
}

/* For us to care, the item must:
   1) be directly under the current position
   2) have a "walk on" script
   */

Items::Item* find_walkon_item( const ItemsVector& ivec, short z )
{
  for ( const auto& item : ivec )
  {
    if ( z == item->pos3d().z() || z == item->pos3d().z() + 1 )
    {
      if ( !item->itemdesc().walk_on_script.empty() )
      {
        return item;
      }
    }
  }
  return nullptr;
}
}  // namespace Core
}  // namespace Pol
