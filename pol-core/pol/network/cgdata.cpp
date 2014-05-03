/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../../clib/unicode.h"

#include "../module/osmod.h"
#include "../module/unimod.h"
#include "../module/uomod.h"
#include "../uoexec.h"

#include "cgdata.h"
#include "../fnsearch.h"
#include "../multi/house.h"
#include "../containr.h"

namespace Pol {
  namespace Network {

	ClientGameData::ClientGameData() :
	  vendor( NULL ),
	  gumpmods(),
	  textentry_uoemod( NULL ),
	  target_cursor_uoemod( NULL ),
	  menu_selection_uoemod( NULL ),
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
		Module::UOExecutorModule* uoemod = *gumpmods.begin();
		uoemod->uoexec.os_module->revive();
		uoemod->gump_chr = NULL;
		gumpmods.erase( gumpmods.begin() );
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
			vector<u8> newvec;
			house->WorkingCompressed.swap( newvec );

			vector<u8> newvec2;
			house->CurrentCompressed.swap( newvec2 );
			house->editing = false;
		  }
		}
		custom_house_serial = 0;
	  }

	}

	void ClientGameData::add_gumpmod( Module::UOExecutorModule* uoemod )
	{
	  gumpmods.insert( uoemod );
	}

	Module::UOExecutorModule* ClientGameData::find_gumpmod( u32 pid )
	{
	  for ( GumpMods::iterator itr = gumpmods.begin(); itr != gumpmods.end(); ++itr )
	  {
		Module::UOExecutorModule* uoemod = ( *itr );
		if ( uoemod->uoexec.os_module->pid() == pid )
		  return uoemod;
	  }
	  return NULL;
	}

	void ClientGameData::remove_gumpmod( Module::UOExecutorModule* uoemod )
	{
	  gumpmods.erase( uoemod );
	}
  }
}