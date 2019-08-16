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
#include <format/format.h>

#include "pol_global_config.h"

namespace Pol
{
namespace Core
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
  logs.push_back( std::make_pair( "ProcessSize", Clib::getCurrentMemoryUsage() ) );
  logs.push_back( std::make_pair( "CPProfilerSize", cprop_profiler_size ) );
  logs.push_back( std::make_pair( "GameStateSize", gamestate_size.misc ) );
  logs.push_back( std::make_pair( "RealmSize", gamestate_size.realm_size ) );
  logs.push_back( std::make_pair( "SystemStateSize", systemstate_size ) );
  logs.push_back( std::make_pair( "MultiBufferSize", multibuffer_size ) );
  logs.push_back( std::make_pair( "SettingsSize", settings_size ) );
  logs.push_back( std::make_pair( "StateSize", state_size ) );
  logs.push_back( std::make_pair( "ScriptCount", script_sizes.script_count ) );
  logs.push_back( std::make_pair( "ScriptSize", script_sizes.script_size ) );
  logs.push_back( std::make_pair( "ScriptStoreCount", script_sizes.scriptstorage_count ) );
  logs.push_back( std::make_pair( "ScriptStoreSize", script_sizes.scriptstorage_size ) );
  logs.push_back( std::make_pair( "ConfigCount", config_sizes.cfg_count ) );
  logs.push_back( std::make_pair( "ConfigSize", config_sizes.cfg_size ) );
  logs.push_back( std::make_pair( "DataStoreCount", config_sizes.datastore_count ) );
  logs.push_back( std::make_pair( "DataStoreSize", config_sizes.datastore_size ) );
  logs.push_back( std::make_pair( "ConfigBufferSize", config_sizes.misc ) );
  logs.push_back( std::make_pair( "AccountCount", gamestate_size.account_count ) );
  logs.push_back( std::make_pair( "AccountSize", gamestate_size.account_size ) );
  logs.push_back( std::make_pair( "ClientCount", network_size.client_count ) );
  logs.push_back( std::make_pair( "ClientSize", network_size.client_size ) );
  logs.push_back( std::make_pair( "NetworkSize", network_size.misc ) );
  logs.push_back( std::make_pair( "ObjectStorage", object_sizes.misc ) );

  logs.push_back( std::make_pair( "ObjItemCount", object_sizes.obj_item_count ) );
  logs.push_back( std::make_pair( "ObjItemSize", object_sizes.obj_item_size ) );
  logs.push_back( std::make_pair( "ObjContCount", object_sizes.obj_cont_count ) );
  logs.push_back( std::make_pair( "ObjContSize", object_sizes.obj_cont_size ) );
  logs.push_back( std::make_pair( "ObjCharCount", object_sizes.obj_char_count ) );
  logs.push_back( std::make_pair( "ObjCharSize", object_sizes.obj_char_size ) );
  logs.push_back( std::make_pair( "ObjNpcCount", object_sizes.obj_npc_count ) );
  logs.push_back( std::make_pair( "ObjNpcSize", object_sizes.obj_npc_size ) );
  logs.push_back( std::make_pair( "ObjWeaponCount", object_sizes.obj_weapon_count ) );
  logs.push_back( std::make_pair( "ObjWeaponSize", object_sizes.obj_weapon_size ) );
  logs.push_back( std::make_pair( "ObjArmorCount", object_sizes.obj_armor_count ) );
  logs.push_back( std::make_pair( "ObjArmorSize", object_sizes.obj_armor_size ) );
  logs.push_back( std::make_pair( "ObjMultiCount", object_sizes.obj_multi_count ) );
  logs.push_back( std::make_pair( "ObjMultiSize", object_sizes.obj_multi_size ) );
  logs.push_back( std::make_pair( "BObjectAllocatorSize", Bscript::bobject_alloc.memsize ) );
  logs.push_back( std::make_pair( "UninitAllocatorSize", Bscript::uninit_alloc.memsize ) );
  logs.push_back( std::make_pair( "BLongAllocatorSize", Bscript::blong_alloc.memsize ) );
  logs.push_back( std::make_pair( "BDoubleAllocatorSize", Bscript::double_alloc.memsize ) );
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
    fmt::Writer header;
    header << "Time";
    for ( const auto& entry : logs )
      header << " ;" << entry.first;
    FLEXLOG( log ) << header.str() << "\n";
  }


  fmt::Writer line;
  line << GET_LOG_FILESTAMP;
  for ( const auto& entry : logs )
    line << " ;" << entry.second;
  FLEXLOG( log ) << line.str() << "\n";

  CLOSE_FLEXLOG( log );
}
}  // namespace Core
}  // namespace Pol
