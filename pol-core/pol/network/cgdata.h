/** @file
 *
 * @par History
 */


#ifndef CGDATA_H
#define CGDATA_H

#include <cstddef>
#include <map>

#include "../../clib/rawtypes.h"
#include "../../clib/refptr.h"
#include "../../clib/weakptr.h"
#include "../reftypes.h"

namespace Pol
{
namespace Module
{
class UOExecutorModule;
class UnicodeExecutorModule;
}  // namespace Module
namespace Core
{
class JusticeRegion;
class MusicRegion;
class UContainer;
class WeatherRegion;
class TargetCursor;
class Menu;
class MenuItem;
struct PKTIN_7D;
}  // namespace Core
namespace Network
{
class Client;

class ClientGameData
{
public:
  ClientGameData();
  ~ClientGameData();
  void clear();
  size_t estimatedSize() const;


  ref_ptr<Core::UContainer> vendor_bought;
  ref_ptr<Core::UContainer> vendor_for_sale;
  Core::NpcRef vendor;

  typedef std::map<u32, Module::UOExecutorModule*> GumpMods;
  GumpMods gumpmods;

  void add_gumpmod( Module::UOExecutorModule*, u32 gumpid );
  Module::UOExecutorModule* find_gumpmod( u32 gumpid );
  void remove_gumpmods( Module::UOExecutorModule* );

  Module::UOExecutorModule* textentry_uoemod;


  /*
   *  If a script is requesting a target cursor from this
   *  character, target_cursor_ex is that script.
   *  Same for menu selection.
   */
  Module::UOExecutorModule* target_cursor_uoemod;
  Module::UOExecutorModule* menu_selection_uoemod;
  Module::UOExecutorModule* popup_menu_selection_uoemod;
  Module::UOExecutorModule* prompt_uoemod;
  Module::UOExecutorModule* resurrect_uoemod;
  Module::UOExecutorModule* selcolor_uoemod;

  Module::UnicodeExecutorModule* prompt_uniemod;  // UNIemod, not UOemod!!!

  Core::JusticeRegion* justice_region;

  // LightRegion* light_region;
  Core::MusicRegion* music_region;
  Core::WeatherRegion* weather_region;
  Core::TargetCursor* tcursor2;
  weak_ptr<Core::Menu> menu;
  void ( *on_menu_selection )( Network::Client* client, Core::MenuItem* mi, Core::PKTIN_7D* msg );
  void ( *on_popup_menu_selection )( Network::Client* client, u32 serial, u16 id );
  int lightlevel;
  u32 custom_house_serial;
  u32 custom_house_chrserial;
  bool script_defined_update_range;
  u8 update_range;
};
}  // namespace Network
}  // namespace Pol
#endif
