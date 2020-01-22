/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include "cgdata.h"

#include <cstddef>

#include "../containr.h"
#include "../fnsearch.h"
#include "../mobile/npc.h"
#include "../module/unimod.h"
#include "../module/uomod.h"
#include "../multi/customhouses.h"
#include "../multi/house.h"
#include "../multi/multi.h"
#include "../uoexec.h"

namespace Pol
{
namespace Network
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
      lightlevel( 0 ),
      // light_region(nullptr),
      music_region( nullptr ),
      weather_region( nullptr ),
      custom_house_serial( 0 ),
      custom_house_chrserial( 0 )
{
}

ClientGameData::~ClientGameData()
{
  clear();
}

void ClientGameData::clear()
{
  while ( !gumpmods.empty() )
  {
    GumpMods::iterator it = gumpmods.begin();
    Module::UOExecutorModule* uoemod = it->second;
    uoemod->uoexec().revive();
    uoemod->gump_chr = nullptr;
    gumpmods.erase( it );
  }

  if ( textentry_uoemod != nullptr )
  {
    textentry_uoemod->uoexec().revive();
    textentry_uoemod->textentry_chr = nullptr;
    textentry_uoemod = nullptr;
  }

  if ( menu_selection_uoemod != nullptr )
  {
    menu_selection_uoemod->uoexec().revive();
    menu_selection_uoemod->menu_selection_chr = nullptr;
    menu_selection_uoemod = nullptr;
  }

  if ( popup_menu_selection_uoemod != nullptr )
  {
    popup_menu_selection_uoemod->uoexec().revive();
    popup_menu_selection_uoemod->popup_menu_selection_chr = nullptr;
    popup_menu_selection_uoemod->popup_menu_selection_above = nullptr;
    popup_menu_selection_uoemod = nullptr;
  }

  if ( prompt_uoemod != nullptr )
  {
    prompt_uoemod->uoexec().revive();
    prompt_uoemod->prompt_chr = nullptr;
    prompt_uoemod = nullptr;
  }

  if ( resurrect_uoemod != nullptr )
  {
    resurrect_uoemod->uoexec().revive();
    resurrect_uoemod->resurrect_chr = nullptr;
    resurrect_uoemod = nullptr;
  }

  if ( selcolor_uoemod != nullptr )
  {
    selcolor_uoemod->uoexec().revive();
    selcolor_uoemod->selcolor_chr = nullptr;
    selcolor_uoemod = nullptr;
  }

  if ( target_cursor_uoemod != nullptr )
  {
    target_cursor_uoemod->uoexec().revive();
    target_cursor_uoemod->target_cursor_chr = nullptr;
    target_cursor_uoemod = nullptr;
  }

  if ( prompt_uniemod != nullptr )
  {
    prompt_uniemod->uoexec().revive();
    prompt_uniemod->prompt_chr = nullptr;
    prompt_uniemod = nullptr;
  }
  if ( custom_house_serial != 0 )
  {
    Multi::UMulti* multi = Core::system_find_multi( custom_house_serial );
    if ( multi != nullptr )
    {
      Multi::UHouse* house = multi->as_house();
      if ( house != nullptr )
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
void ClientGameData::add_gumpmod( Module::UOExecutorModule* uoemod, u32 gumpid )
{
  gumpmods[gumpid] = uoemod;
}

/// Given a gumpid, finds the module that registered it, returns nullptr if not found
Module::UOExecutorModule* ClientGameData::find_gumpmod( u32 gumpid )
{
  GumpMods::iterator it = gumpmods.find( gumpid );
  if ( it == gumpmods.end() )
    return nullptr;
  return it->second;
}

/// Removes all the registered gumpids for a given module
void ClientGameData::remove_gumpmods( Module::UOExecutorModule* uoemod )
{
  for ( GumpMods::const_iterator it = gumpmods.cbegin(); it != gumpmods.cend(); /* no inc here */ )
    if ( it->second == uoemod )
      gumpmods.erase( it++ );
    else
      ++it;
}

size_t ClientGameData::estimatedSize() const
{
  size_t size = sizeof( ClientGameData );
  size += 3 * sizeof( void* ) +
          gumpmods.size() * ( sizeof( Module::UOExecutorModule* ) + 3 * sizeof( void* ) );
  return size;
}
}  // namespace Network
}  // namespace Pol
