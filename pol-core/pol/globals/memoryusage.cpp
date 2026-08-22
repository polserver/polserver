#include "pol/globals/memoryusage.h"

#include "bscript/bdouble.h"
#include "bscript/blong.h"
#include "bscript/buninit.h"
#include "clib/boostutils.h"
#include "clib/clib.h"
#include "clib/fileutil.h"
#include "clib/logfacility.h"
#include "plib/systemstate.h"

#include "pol/globals/multidefs.h"
#include "pol/globals/network.h"
#include "pol/globals/object_storage.h"
#include "pol/globals/script_internals.h"
#include "pol/globals/settings.h"
#include "pol/globals/state.h"
#include "pol/globals/ucfg.h"
#include "pol/globals/uvars.h"

#include <ctime>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

#include <pol_global_config.h>


namespace Pol::Core
{
namespace
{
struct SectionName
{
  const char* name;
  MemoryUsage::Section flag;
};

constexpr SectionName section_names[] = { { "process", MemoryUsage::SECTION_PROCESS },
                                          { "objects", MemoryUsage::SECTION_OBJECTS },
                                          { "scripts", MemoryUsage::SECTION_SCRIPTS },
                                          { "gamestate", MemoryUsage::SECTION_GAMESTATE },
                                          { "realms", MemoryUsage::SECTION_REALMS },
                                          { "network", MemoryUsage::SECTION_NETWORK },
                                          { "config", MemoryUsage::SECTION_CONFIG },
                                          { "systemstate", MemoryUsage::SECTION_SYSTEMSTATE },
                                          { "multis", MemoryUsage::SECTION_MULTIS },
                                          { "settings", MemoryUsage::SECTION_SETTINGS },
                                          { "state", MemoryUsage::SECTION_STATE },
                                          { "cprops", MemoryUsage::SECTION_CPROPS },
                                          { "allocators", MemoryUsage::SECTION_ALLOCATORS } };
}  // namespace

bool MemoryUsage::sectionByName( const std::string& name, unsigned* flag )
{
  for ( const auto& entry : section_names )
  {
    if ( stricmp( name.c_str(), entry.name ) == 0 )
    {
      *flag = entry.flag;
      return true;
    }
  }
  return false;
}

std::string MemoryUsage::sectionNames()
{
  std::string names;
  for ( const auto& entry : section_names )
  {
    if ( !names.empty() )
      names += ", ";
    names += entry.name;
  }
  return names;
}

std::string MemoryUsage::reportPath( const std::string& prefix )
{
  Clib::make_dir( "log/memory" );
  auto now = Clib::localtime( std::time( nullptr ) );
  return fmt::format( "log/memory/{}-{:%Y%m%d-%H%M%S}.log", prefix, now );
}

std::string MemoryUsage::sanitizeForFilename( const std::string& name )
{
  std::string out = name;
  if ( out.size() > 4 && stricmp( out.c_str() + out.size() - 4, ".ecl" ) == 0 )
    out.resize( out.size() - 4 );
  for ( auto& ch : out )
  {
    if ( ch == '/' || ch == '\\' || ch == ':' || ch == ' ' )
      ch = '_';
  }
  return out;
}

void MemoryUsage::log( unsigned sections )
{
  const bool want_gamestate = ( sections & SECTION_GAMESTATE ) != 0;
  const bool want_realms = ( sections & SECTION_REALMS ) != 0;

  size_t process_size = ( sections & SECTION_PROCESS ) ? Clib::getCurrentMemoryUsage() : 0;
  size_t systemstate_size =
      ( sections & SECTION_SYSTEMSTATE ) ? Plib::systemstate.estimatedSize() : 0;
  size_t multibuffer_size =
      ( sections & SECTION_MULTIS ) ? Multi::multidef_buffer.estimateSize() : 0;
  size_t settings_size = ( sections & SECTION_SETTINGS ) ? settingsManager.estimateSize() : 0;
  size_t state_size = ( sections & SECTION_STATE ) ? stateManager.estimateSize() : 0;
  size_t cprop_profiler_size =
      ( sections & SECTION_CPROPS ) ? CPropProfiler::instance().estimateSize() : 0;

  NetworkManager::Memory network_size{};
  if ( sections & SECTION_NETWORK )
    network_size = networkManager.estimateSize();

  ObjectStorageManager::MemoryUsage object_sizes{};
  if ( sections & SECTION_OBJECTS )
    object_sizes = objStorageManager.estimateSize();

  ScriptScheduler::Memory script_sizes{};
  if ( sections & SECTION_SCRIPTS )
    script_sizes = scriptScheduler.estimateSize( false );

  ConfigurationBuffer::Memory config_sizes{};
  if ( sections & SECTION_CONFIG )
    config_sizes = configurationbuffer.estimateSize();

  // One call covers both sections; the realm walk is the expensive half.
  GameState::Memory gamestate_size{};
  if ( want_gamestate || want_realms )
    gamestate_size = gamestate.estimateSize( want_realms );

  // Unmeasured columns are written empty rather than dropped: the header is written once, when
  // the file is created, so the column set has to stay fixed for the life of the file. An empty
  // field also reads differently from a measured zero.
  std::vector<std::pair<std::string, std::optional<size_t>>> logs;
  auto add = [&]( std::string label, unsigned section, size_t value )
  {
    logs.emplace_back( std::move( label ),
                       ( sections & section ) ? std::optional<size_t>( value ) : std::nullopt );
  };
  add( "ProcessSize", SECTION_PROCESS, process_size );
  add( "CPProfilerSize", SECTION_CPROPS, cprop_profiler_size );
  add( "GameStateSize", SECTION_GAMESTATE, gamestate_size.misc );
  add( "RealmSize", SECTION_REALMS, gamestate_size.realm_size );
  add( "SystemStateSize", SECTION_SYSTEMSTATE, systemstate_size );
  add( "MultiBufferSize", SECTION_MULTIS, multibuffer_size );
  add( "SettingsSize", SECTION_SETTINGS, settings_size );
  add( "StateSize", SECTION_STATE, state_size );
  add( "ScriptCount", SECTION_SCRIPTS, script_sizes.script_count );
  add( "ScriptSize", SECTION_SCRIPTS, script_sizes.script_size );
  add( "ScriptStoreCount", SECTION_SCRIPTS, script_sizes.scriptstorage_count );
  add( "ScriptStoreSize", SECTION_SCRIPTS, script_sizes.scriptstorage_size );
  add( "ConfigCount", SECTION_CONFIG, config_sizes.cfg_count );
  add( "ConfigSize", SECTION_CONFIG, config_sizes.cfg_size );
  add( "DataStoreCount", SECTION_CONFIG, config_sizes.datastore_count );
  add( "DataStoreSize", SECTION_CONFIG, config_sizes.datastore_size );
  add( "ConfigBufferSize", SECTION_CONFIG, config_sizes.misc );
  add( "AccountCount", SECTION_GAMESTATE, gamestate_size.account_count );
  add( "AccountSize", SECTION_GAMESTATE, gamestate_size.account_size );
  add( "ClientCount", SECTION_NETWORK, network_size.client_count );
  add( "ClientSize", SECTION_NETWORK, network_size.client_size );
  add( "NetworkSize", SECTION_NETWORK, network_size.misc );
  add( "ObjectStorage", SECTION_OBJECTS, object_sizes.misc );

  add( "ObjItemCount", SECTION_OBJECTS, object_sizes.obj_item_count );
  add( "ObjItemSize", SECTION_OBJECTS, object_sizes.obj_item_size );
  add( "ObjContCount", SECTION_OBJECTS, object_sizes.obj_cont_count );
  add( "ObjContSize", SECTION_OBJECTS, object_sizes.obj_cont_size );
  add( "ObjCharCount", SECTION_OBJECTS, object_sizes.obj_char_count );
  add( "ObjCharSize", SECTION_OBJECTS, object_sizes.obj_char_size );
  add( "ObjNpcCount", SECTION_OBJECTS, object_sizes.obj_npc_count );
  add( "ObjNpcSize", SECTION_OBJECTS, object_sizes.obj_npc_size );
  add( "ObjWeaponCount", SECTION_OBJECTS, object_sizes.obj_weapon_count );
  add( "ObjWeaponSize", SECTION_OBJECTS, object_sizes.obj_weapon_size );
  add( "ObjArmorCount", SECTION_OBJECTS, object_sizes.obj_armor_count );
  add( "ObjArmorSize", SECTION_OBJECTS, object_sizes.obj_armor_size );
  add( "ObjMultiCount", SECTION_OBJECTS, object_sizes.obj_multi_count );
  add( "ObjMultiSize", SECTION_OBJECTS, object_sizes.obj_multi_size );
  add( "BObjectAllocatorSize", SECTION_ALLOCATORS, Bscript::bobject_alloc.memsize );
  add( "UninitAllocatorSize", SECTION_ALLOCATORS, Bscript::uninit_alloc.memsize );
  add( "BLongAllocatorSize", SECTION_ALLOCATORS, Bscript::blong_alloc.memsize );
  add( "BDoubleAllocatorSize", SECTION_ALLOCATORS, Bscript::double_alloc.memsize );
#ifdef ENABLE_FLYWEIGHT_REPORT
  // Always queried, so the column count stays the same whatever was asked for.
  auto flydata = boost_utils::Query::getCountAndSize();
  int i = 0;
  for ( const auto& data : flydata )
  {
    auto str = std::to_string( i );
    add( "FlyWeightBucket" + str + "Count", SECTION_ALLOCATORS, data.first );
    add( "FlyWeightBucket" + str + "Size", SECTION_ALLOCATORS, data.second );
    ++i;
  }
#endif
  bool needs_header = !Clib::FileExists( "log/memoryusage.log" );
  auto log = OPEN_FLEXLOG( "log/memoryusage.log", false );
  if ( needs_header )
  {
    std::string header = "Time";
    for ( const auto& entry : logs )
      fmt::format_to( std::back_inserter( header ), " ;{}", entry.first );
    FLEXLOGLN( log, header );
  }

  std::string line = GET_LOG_FILESTAMP;
  for ( const auto& entry : logs )
  {
    if ( entry.second.has_value() )
      fmt::format_to( std::back_inserter( line ), " ;{}", *entry.second );
    else
      line += " ;";  // not measured this time round
  }
  FLEXLOGLN( log, line );

  CLOSE_FLEXLOG( log );
}
}  // namespace Pol::Core
