#include "memoryusage.h"

#include "../../clib/boostutils.h"
#include "../../clib/fileutil.h"
#include "../../clib/logfacility.h"
#include "../../plib/systemstate.h"
#include "multidefs.h"
#include "network.h"
#include "object_storage.h"
#include "script_internals.h"
#include "settings.h"
#include "state.h"
#include "ucfg.h"
#include "uvars.h"
#include <fmt/format.h>
#include <iterator>

#include "pol_global_config.h"


namespace Pol::Core
{
void MemoryUsage::log()
{
  // std::string footprint is ~ string.capacity()
  // std::vector footprint is ~ 3 * sizeof(T*) + vector.capacity() * sizeof( T );
  // std::set footprint is ~ 3 * sizeof( void* ) + set.size() * ( sizeof(T)+3 * sizeof( void* ) );
  // std::map footprint is ~ ( sizeof(K)+sizeof( V ) + ( sizeof(void*) * 3 + 1 ) / 2 ) * map.size();

  size_t systemstate_size = Plib::systemstate.estimatedSize();
  size_t multibuffer_size = Multi::multidef_buffer.estimateSize();
  auto network_size = networkManager.estimateSize();
  auto object_sizes = objStorageManager.estimateSize();
  auto script_sizes = scriptScheduler.estimateSize( false );
  size_t settings_size = settingsManager.estimateSize();
  size_t state_size = stateManager.estimateSize();
  auto config_sizes = configurationbuffer.estimateSize();
  auto gamestate_size = gamestate.estimateSize();
  auto cprop_profiler_size = CPropProfiler::instance().estimateSize();

  std::vector<std::pair<std::string, size_t>> logs;
  logs.emplace_back( "ProcessSize", Clib::getCurrentMemoryUsage() );
  logs.emplace_back( "CPProfilerSize", cprop_profiler_size );
  logs.emplace_back( "GameStateSize", gamestate_size.misc );
  logs.emplace_back( "RealmSize", gamestate_size.realm_size );
  logs.emplace_back( "SystemStateSize", systemstate_size );
  logs.emplace_back( "MultiBufferSize", multibuffer_size );
  logs.emplace_back( "SettingsSize", settings_size );
  logs.emplace_back( "StateSize", state_size );
  logs.emplace_back( "ScriptCount", script_sizes.script_count );
  logs.emplace_back( "ScriptSize", script_sizes.script_size );
  logs.emplace_back( "ScriptStoreCount", script_sizes.scriptstorage_count );
  logs.emplace_back( "ScriptStoreSize", script_sizes.scriptstorage_size );
  logs.emplace_back( "ConfigCount", config_sizes.cfg_count );
  logs.emplace_back( "ConfigSize", config_sizes.cfg_size );
  logs.emplace_back( "DataStoreCount", config_sizes.datastore_count );
  logs.emplace_back( "DataStoreSize", config_sizes.datastore_size );
  logs.emplace_back( "ConfigBufferSize", config_sizes.misc );
  logs.emplace_back( "AccountCount", gamestate_size.account_count );
  logs.emplace_back( "AccountSize", gamestate_size.account_size );
  logs.emplace_back( "ClientCount", network_size.client_count );
  logs.emplace_back( "ClientSize", network_size.client_size );
  logs.emplace_back( "NetworkSize", network_size.misc );
  logs.emplace_back( "ObjectStorage", object_sizes.misc );

  logs.emplace_back( "ObjItemCount", object_sizes.obj_item_count );
  logs.emplace_back( "ObjItemSize", object_sizes.obj_item_size );
  logs.emplace_back( "ObjContCount", object_sizes.obj_cont_count );
  logs.emplace_back( "ObjContSize", object_sizes.obj_cont_size );
  logs.emplace_back( "ObjCharCount", object_sizes.obj_char_count );
  logs.emplace_back( "ObjCharSize", object_sizes.obj_char_size );
  logs.emplace_back( "ObjNpcCount", object_sizes.obj_npc_count );
  logs.emplace_back( "ObjNpcSize", object_sizes.obj_npc_size );
  logs.emplace_back( "ObjWeaponCount", object_sizes.obj_weapon_count );
  logs.emplace_back( "ObjWeaponSize", object_sizes.obj_weapon_size );
  logs.emplace_back( "ObjArmorCount", object_sizes.obj_armor_count );
  logs.emplace_back( "ObjArmorSize", object_sizes.obj_armor_size );
  logs.emplace_back( "ObjMultiCount", object_sizes.obj_multi_count );
  logs.emplace_back( "ObjMultiSize", object_sizes.obj_multi_size );
  logs.emplace_back( "BObjectAllocatorSize", Bscript::bobject_alloc.memsize );
  logs.emplace_back( "UninitAllocatorSize", Bscript::uninit_alloc.memsize );
  logs.emplace_back( "BLongAllocatorSize", Bscript::blong_alloc.memsize );
  logs.emplace_back( "BDoubleAllocatorSize", Bscript::double_alloc.memsize );
#ifdef ENABLE_FLYWEIGHT_REPORT
  auto flydata = boost_utils::Query::getCountAndSize();
  int i = 0;
  for ( const auto& data : flydata )
  {
    auto str = std::to_string( i );
    logs.push_back( std::make_pair( "FlyWeightBucket" + str + "Count", data.first ) );
    logs.push_back( std::make_pair( "FlyWeightBucket" + str + "Size", data.second ) );
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
    fmt::format_to( std::back_inserter( line ), " ;{}", entry.second );
  FLEXLOGLN( log, line );

  CLOSE_FLEXLOG( log );
}
}  // namespace Pol::Core
