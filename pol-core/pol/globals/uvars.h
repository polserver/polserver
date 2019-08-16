/** @file
 *
 * @par History
 */


#ifndef __UVARS_H
#define __UVARS_H

#include <array>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../../clib/maputil.h"
#include "../../clib/rawtypes.h"
#include "../../clib/refptr.h"
#include "../../clib/threadhelp.h"
#include "../../plib/clidata.h"
#include "../action.h"
#include "../cmdlevel.h"
#include "../layers.h"
#include "../menu.h"
#include "../reftypes.h"
#include "../region.h"
#include "../schedule.h"
#include "../storage.h"
#include "../syshook.h"
#include "../target.h"
#include "../tasks.h"
#include "../textcmd.h"
#include "../uoskills.h"


namespace Pol
{
namespace Realms
{
class Realm;
}
namespace Items
{
class Equipment;
class Item;
class ItemDesc;
class UWeapon;
}  // namespace Items
namespace Mobile
{
class Attribute;
}
namespace Network
{
class Client;
}
namespace Multi
{
struct BoatShape;
}
namespace Core
{
class ConsoleCommand;
class Guild;
class JusticeRegion;
class LightRegion;
class ListenPoint;
class MusicRegion;
class NoCastRegion;
class NpcTemplate;
class NpcTemplateElem;
class Party;
class PropertyList;
class ResourceDef;
class SpellCircle;
class StartingLocation;
class UOExecutor;
class USpell;
class Vital;
class WeatherDef;

typedef std::vector<Core::CmdLevel> CmdLevels;

typedef std::vector<AccountRef> AccountsVector;
class ItemsVector : public std::vector<Items::Item*>
{
};

typedef std::vector<StartingLocation*> StartingLocations;
typedef RegionGroup<JusticeRegion> JusticeDef;
typedef RegionGroup<NoCastRegion> NoCastDef;
typedef RegionGroup<LightRegion> LightDef;
typedef RegionGroup<MusicRegion> MusicDef;

typedef std::map<std::string, NpcTemplateElem, Clib::ci_cmp_pred> NpcTemplatesElems;
typedef std::map<std::string, NpcTemplate*> NpcTemplates;

typedef ref_ptr<Party> PartyRef;
typedef std::vector<PartyRef> Parties;
typedef ref_ptr<Guild> GuildRef;
typedef std::map<unsigned int, GuildRef> Guilds;

typedef std::map<std::string, Mobile::Attribute*, Clib::ci_cmp_pred> AttributesByName;

typedef std::vector<ArmorZone> ArmorZones;
typedef std::map<std::string, Vital*, Clib::ci_cmp_pred> VitalsByName;
typedef std::map<unsigned int, unsigned int> OldObjtypeConversions;
typedef std::map<std::string, u32, Clib::ci_cmp_pred> ObjtypeByNameMap;
typedef std::map<std::string, ResourceDef*> ResourceDefs;
typedef std::pair<std::string /* name */, u8 /* layer */> NameAndLayer;
typedef std::map<NameAndLayer, Items::Equipment*> IntrinsicEquipments;
typedef std::map<u16 /* graphic */, Multi::BoatShape*> BoatShapes;
typedef std::map<UOExecutor*, ListenPoint*> ListenPoints;
typedef std::priority_queue<ScheduledTask*, std::vector<ScheduledTask*>, SchComparer> TaskQueue;
typedef std::set<std::string> PropSet;

typedef void ( *TextCmdFunc )( Network::Client* );
typedef std::map<std::string, TextCmdFunc, Clib::ci_cmp_pred> TextCmds;
typedef void ( *ParamTextCmdFunc )( Network::Client*, const std::string& );
typedef std::map<std::string, ParamTextCmdFunc, wordicmp> ParamTextCmds;


class GameState
{
public:
  GameState();
  ~GameState();
  GameState( const GameState& ) = delete;
  GameState& operator=( const GameState& ) = delete;

  void deinitialize();
  struct Memory;

  Memory estimateSize() const;

  CmdLevels cmdlevels;

  NpcTemplates npc_templates;
  NpcTemplatesElems npc_template_elems;

  std::unique_ptr<Core::PropertyList> global_properties;

  AccountsVector accounts;
  StartingLocations startlocations;
  Items::UWeapon* wrestling_weapon;

  JusticeDef* justicedef;
  NoCastDef* nocastdef;
  LightDef* lightdef;
  WeatherDef* weatherdef;
  MusicDef* musicdef;

  std::vector<Menu> menus;

  Storage storage;

  Parties parties;
  Guilds guilds;
  unsigned int nextguildid;

  Realms::Realm* main_realm;
  std::vector<Realms::Realm*> Realms;
  std::map<int, Realms::Realm*> shadowrealms_by_id;
  unsigned int baserealm_count;
  unsigned int shadowrealm_count;

  std::unique_ptr<PeriodicTask> update_rpm_task;
  std::unique_ptr<PeriodicTask> regen_stats_task;
  std::unique_ptr<PeriodicTask> regen_resources_task;
  std::unique_ptr<PeriodicTask> reload_accounts_task;
  std::unique_ptr<PeriodicTask> write_account_task;
  std::unique_ptr<PeriodicTask> update_sysload_task;
  std::unique_ptr<PeriodicTask> reload_pol_cfg_task;

  std::vector<Mobile::Attribute*> attributes;
  unsigned numAttributes;
  AttributesByName attributes_byname;
  const Mobile::Attribute* pAttrStrength;
  const Mobile::Attribute* pAttrIntelligence;
  const Mobile::Attribute* pAttrDexterity;
  const Mobile::Attribute* pAttrParry;
  const Mobile::Attribute* pAttrTactics;

  std::array<std::array<u32, 2>, 8> spell_scroll_objtype_limits;
  std::vector<USpell*> spells;
  std::vector<SpellCircle*> spellcircles;

  std::vector<std::unique_ptr<ExportScript>> export_scripts;
  SystemHooks system_hooks;

  std::vector<std::string> tipfilenames;

  ArmorZones armorzones;
  double armor_zone_chance_sum;

  std::vector<Vital*> vitals;
  unsigned numVitals;
  const Vital* pVitalLife;
  const Vital* pVitalStamina;
  const Vital* pVitalMana;
  VitalsByName vitals_byname;

  std::map<u32, Items::ItemDesc*> desctable;
  OldObjtypeConversions old_objtype_conversions;
  std::vector<Items::ItemDesc*> dynamic_item_descriptors;
  ObjtypeByNameMap objtype_byname;
  std::unique_ptr<Items::ItemDesc> empty_itemdesc;
  std::unique_ptr<Items::ItemDesc> temp_itemdesc;

  ResourceDefs resourcedefs;

  IntrinsicEquipments intrinsic_equipments;

  BoatShapes boatshapes;

  UACTION mount_action_xlate[ACTION__HIGHEST + 1];
  std::map<std::string, MobileTranslate> animation_translates;

  std::vector<ConsoleCommand> console_commands;

  std::array<Plib::LandTile, Plib::LANDTILE_COUNT> landtiles;
  bool landtiles_loaded;

  ListenPoints listen_points;

  Plib::Package* wwwroot_pkg;
  std::map<std::string, std::string> mime_types;

  TaskQueue task_queue;

  PropSet Global_Ignore_CProps;

  Cursors target_cursors;

  TextCmds textcmds;
  ParamTextCmds paramtextcmds;

  std::vector<UOSkill> uo_skills;

  struct Memory
  {
    size_t account_size;
    size_t account_count;
    size_t realm_size;
    size_t misc;
  };
  threadhelp::TaskThreadPool task_thread_pool;

private:
  void cleanup_vars();
  void cleanup_scripts();
  void clear_listen_points();
  void unload_intrinsic_weapons();
  void unload_intrinsic_templates();
  void unload_npc_templates();
};
extern GameState gamestate;
}  // namespace Core
}  // namespace Pol
#endif
