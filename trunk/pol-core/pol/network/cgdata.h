/*
History
=======


Notes
=======

*/

#ifndef CGDATA_H
#define CGDATA_H

#include "../../clib/refptr.h"

#include "../containr.h"
#include "../npc.h"
#include "../reftypes.h"
namespace Pol {
  namespace Module {
	class UnicodeExecutorModule;
  }
  namespace Core {
	class JusticeRegion;
	class LightRegion;
	class MusicRegion;
	class WeatherRegion;
  }
  namespace Network {

	class ClientGameData
	{
	public:
	  ClientGameData();
	  ~ClientGameData();
	  void clear();


	  ref_ptr<Core::UContainer> vendor_bought;
	  ref_ptr<Core::UContainer> vendor_for_sale;
	  Core::NPCRef vendor;

	  typedef std::set<Module::UOExecutorModule*> GumpMods;
	  GumpMods gumpmods;

	  void add_gumpmod( Module::UOExecutorModule* );
	  Module::UOExecutorModule* find_gumpmod( u32 pid );
	  void remove_gumpmod( Module::UOExecutorModule* );

	  Module::UOExecutorModule* textentry_uoemod;


	  /*
	   *  If a script is requesting a target cursor from this
	   *  character, target_cursor_ex is that script.
	   *  Same for menu selection.
	   */
	  Module::UOExecutorModule* target_cursor_uoemod;
	  Module::UOExecutorModule* menu_selection_uoemod;
	  Module::UOExecutorModule* prompt_uoemod;
	  Module::UOExecutorModule* resurrect_uoemod;
	  Module::UOExecutorModule* selcolor_uoemod;

	  Module::UnicodeExecutorModule* prompt_uniemod; // UNIemod, not UOemod!!!

	  Core::JusticeRegion* justice_region;

	  int lightlevel;
	  // LightRegion* light_region;
	  Core::MusicRegion* music_region;
	  Core::WeatherRegion* weather_region;
	  u32 custom_house_serial;
	};
  }
}
#endif
