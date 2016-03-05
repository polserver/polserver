/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */



#include "cgdata.h"
#include "../fnsearch.h"

#include "../containr.h"
#include "../multi/house.h"


#include "../mobile/npc.h"

#include "../module/osmod.h"
#include "../module/unimod.h"
#include "../module/uomod.h"
#include "../uoexec.h"
#include "../unicode.h"

#include <vector>

namespace Pol
{
namespace Network
{

ClientGameData::ClientGameData() :
  vendor( NULL ),
  gumpmods(),
  textentry_uoemod( NULL ),
  target_cursor_uoemod( NULL ),
  menu_selection_uoemod( NULL ),
  popup_menu_selection_uoemod( NULL ),
  prompt_uoemod( NULL ),
  resurrect_uoemod( NULL ),
  selcolor_uoemod( NULL ),
  prompt_uniemod( NULL ),
  justice_region( NULL ),
  lightlevel( 0 ),
  // light_region(NULL),
  music_region( NULL ),
  weather_region( NULL ),
  custom_house_serial( 0 )
{}

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
    uoemod->uoexec.os_module->revive();
    uoemod->gump_chr = NULL;
    gumpmods.erase( it );
  }

  if ( textentry_uoemod != NULL )
  {
    textentry_uoemod->uoexec.os_module->revive();
    textentry_uoemod->textentry_chr = NULL;
    textentry_uoemod = NULL;
  }

  if ( menu_selection_uoemod != NULL )
  {
    menu_selection_uoemod->uoexec.os_module->revive();
    menu_selection_uoemod->menu_selection_chr = NULL;
    menu_selection_uoemod = NULL;
  }

  if ( popup_menu_selection_uoemod != NULL )
  {
    popup_menu_selection_uoemod->uoexec.os_module->revive();
    popup_menu_selection_uoemod->popup_menu_selection_chr = NULL;
    popup_menu_selection_uoemod->popup_menu_selection_above = NULL;
    popup_menu_selection_uoemod = NULL;
  }

  if ( prompt_uoemod != NULL )
  {
    prompt_uoemod->uoexec.os_module->revive();
    prompt_uoemod->prompt_chr = NULL;
    prompt_uoemod = NULL;
  }

  if ( resurrect_uoemod != NULL )
  {
    resurrect_uoemod->uoexec.os_module->revive();
    resurrect_uoemod->resurrect_chr = NULL;
    resurrect_uoemod = NULL;
  }

  if ( selcolor_uoemod != NULL )
  {
    selcolor_uoemod->uoexec.os_module->revive();
    selcolor_uoemod->selcolor_chr = NULL;
    selcolor_uoemod = NULL;
  }

  if ( target_cursor_uoemod != NULL )
  {
    target_cursor_uoemod->uoexec.os_module->revive();
    target_cursor_uoemod->target_cursor_chr = NULL;
    target_cursor_uoemod = NULL;
  }

  if ( prompt_uniemod != NULL )
  {
    prompt_uniemod->os_module->revive();
    prompt_uniemod->prompt_chr = NULL;
    prompt_uniemod = NULL;
  }
  if ( custom_house_serial != 0 )
  {
    Multi::UMulti* multi = Core::system_find_multi( custom_house_serial );
    if ( multi != NULL )
    {
      Multi::UHouse* house = multi->as_house();
      if ( house != NULL )
      {
        house->CurrentDesign.FillComponents( house );
        house->WorkingDesign.FillComponents( house, false ); // keep in sync
        house->revision++;
        std::vector<u8> newvec;
        house->WorkingCompressed.swap( newvec );

        std::vector<u8> newvec2;
        house->CurrentCompressed.swap( newvec2 );
        house->editing = false;
      }
    }
    custom_house_serial = 0;
  }

}

/// Registers a gumpid for the given module
void ClientGameData::add_gumpmod( Module::UOExecutorModule* uoemod, u32 gumpid )
{
  gumpmods[gumpid] = uoemod;
}

/// Given a gumpid, finds the module that registered it, returns NULL if not found
Module::UOExecutorModule* ClientGameData::find_gumpmod( u32 gumpid )
{
  GumpMods::iterator it = gumpmods.find( gumpid );
  if( it == gumpmods.end() )
    return NULL;
  return it->second;
}

/// Removes all the registered gumpids for a given module
void ClientGameData::remove_gumpmods( Module::UOExecutorModule* uoemod )
{
  for( GumpMods::const_iterator it = gumpmods.cbegin(); it != gumpmods.cend(); /* no inc here */ )
    if( it->second == uoemod )
      gumpmods.erase( it++ );
    else
      ++it;
}

size_t ClientGameData::estimatedSize() const
{
  size_t size = sizeof( ClientGameData );
  size += 3 * sizeof(void*)+gumpmods.size( ) * ( sizeof( Module::UOExecutorModule*) + 3 * sizeof( void*) );
  return size;
}
}
}