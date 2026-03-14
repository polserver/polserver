/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include "cgdata.h"

#include <cstddef>

#include "../../clib/stlutil.h"
#include "../containr.h"
#include "../fnsearch.h"
#include "../menu.h"
#include "../mobile/npc.h"
#include "../module/unimod.h"
#include "../module/uomod.h"
#include "../multi/customhouses.h"
#include "../multi/house.h"
#include "../multi/multi.h"
#include "../plib/systemstate.h"
#include "../target.h"
#include "../uoexec.h"
#include "pktin.h"


namespace Pol::Network
{
ClientGameData::ClientGameData()
    : vendor( nullptr ),
      gumpmods(),
      textentry_uoemod( nullptr ),
      target_cursor_uoemod( nullptr ),
      menu_selection_uoemod( nullptr ),
      popup_menu_selection_uoemod( nullptr ),
      prompt_uoemod( nullptr ),
      resurrect_uoemod( nullptr ),
      selcolor_uoemod( nullptr ),
      prompt_uniemod( nullptr ),
      justice_region( nullptr ),
      // light_region(nullptr),
      music_region( nullptr ),
      weather_region( nullptr ),
      tcursor2( nullptr ),
      menu( nullptr ),
      on_menu_selection( nullptr ),
      on_popup_menu_selection( nullptr ),
      lightlevel( 0 ),
      custom_house_serial( 0 ),
      custom_house_chrserial( 0 ),
      script_defined_update_range( false ),
      update_range( Core::settingsManager.ssopt.default_visual_range ),
      original_client_update_range( 0 )
{
}

ClientGameData::~ClientGameData()
{
  clear();
}

void ClientGameData::clear()
{
  if ( textentry_uoemod )
  {
    textentry_uoemod->uoexec().revive();
    textentry_uoemod->textentry_chr = nullptr;
    textentry_uoemod = nullptr;
  }

  if ( menu_selection_uoemod )
  {
    menu_selection_uoemod->uoexec().revive();
    menu_selection_uoemod->menu_selection_chr = nullptr;
    menu_selection_uoemod = nullptr;
  }

  if ( popup_menu_selection_uoemod )
  {
    popup_menu_selection_uoemod->uoexec().revive();
    popup_menu_selection_uoemod->popup_menu_selection_chr = nullptr;
    popup_menu_selection_uoemod->popup_menu_selection_above = nullptr;
    popup_menu_selection_uoemod = nullptr;
  }

  if ( prompt_uoemod )
  {
    prompt_uoemod->uoexec().revive();
    prompt_uoemod->prompt_chr = nullptr;
    prompt_uoemod = nullptr;
  }

  if ( resurrect_uoemod )
  {
    resurrect_uoemod->uoexec().revive();
    resurrect_uoemod->resurrect_chr = nullptr;
    resurrect_uoemod = nullptr;
  }

  if ( selcolor_uoemod )
  {
    selcolor_uoemod->uoexec().revive();
    selcolor_uoemod->selcolor_chr = nullptr;
    selcolor_uoemod = nullptr;
  }

  if ( target_cursor_uoemod )
  {
    target_cursor_uoemod->uoexec().revive();
    target_cursor_uoemod->target_cursor_chr = nullptr;
    target_cursor_uoemod = nullptr;
  }

  if ( prompt_uniemod )
  {
    prompt_uniemod->uoexec().revive();
    prompt_uniemod->prompt_chr = nullptr;
    prompt_uniemod = nullptr;
  }
  if ( custom_house_serial )
  {
    Multi::UMulti* multi = Core::system_find_multi( custom_house_serial );
    if ( multi )
    {
      Multi::UHouse* house = multi->as_house();
      if ( house )
      {
        Mobile::Character* chr = Core::find_character( custom_house_chrserial );
        house->CustomHousesQuit( chr, false /*drop_changes*/, false /*send_pkts*/ );
      }
    }
    custom_house_serial = 0;
  }
  custom_house_chrserial = 0;
}

/// Registers a gumpid for the given module
void ClientGameData::add_gumpmod( Module::UOExecutorModule* uoemod, u32 gumpid, bool event_based )
{
  gumpmods[gumpid] = { uoemod, event_based };
}

/// Given a gumpid, finds the module that registered it, returns nullptr if not found
std::pair<Module::UOExecutorModule*, bool> ClientGameData::find_gumpmod( u32 gumpid )
{
  auto it = gumpmods.find( gumpid );
  if ( it == gumpmods.end() )
    return { nullptr, false };
  return it->second;
}

/// Removes all the registered gumpids for a given module
void ClientGameData::remove_gumpmods( Module::UOExecutorModule* uoemod )
{
  for ( auto it = gumpmods.cbegin(); it != gumpmods.cend(); /* no inc here */ )
    if ( it->second.first == uoemod )
      it = gumpmods.erase( it );
    else
      ++it;
}
void ClientGameData::remove_gumpmod( Module::UOExecutorModule* uoemod, u32 gumpid )
{
  auto it = gumpmods.find( gumpid );
  if ( it == gumpmods.end() || it->second.first != uoemod )
    return;
  gumpmods.erase( it );
}

size_t ClientGameData::estimatedSize() const
{
  size_t size = sizeof( ClientGameData );
  size += Clib::memsize( gumpmods );
  return size;
}
}  // namespace Pol::Network
