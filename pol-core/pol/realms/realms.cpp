/** @file
 *
 * @par History
 * - 2005/03/01 Shinigami: added MAX_NUMER_REALMS check to prevent core crash (see
 * MSGBF_SUB18_ENABLE_MAP_DIFFS)
 * - 2008/12/17 MuadDub:   Added check when loading Realms for no realms existing via int counter.
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: shadowrealm_count definition fixed
 */


#include "realms.h"

#include <filesystem>
#include <stddef.h>
#include <system_error>

#include "clib/logfacility.h"
#include "clib/passert.h"
#include "clib/strutil.h"
#include "clib/timer.h"
#include "plib/systemstate.h"

#include "globals/uvars.h"
#include "realms/realm.h"

namespace Pol
{
namespace Core
{
namespace fs = std::filesystem;
bool load_realms()
{
  Realms::Realm* temprealm;
  int realm_counter = 0;
  std::error_code ec;
  for ( const auto& dir_entry :
        fs::directory_iterator( Plib::systemstate.config.realm_data_path, ec ) )
  {
    if ( !dir_entry.is_directory() )
      continue;
    if ( auto fn = dir_entry.path().filename().u8string(); !fn.empty() && *fn.begin() == '.' )
      continue;
    const auto realm_name = dir_entry.path().stem().u8string();

    passert_r( gamestate.Realms.size() < MAX_NUMER_REALMS,
               "You can't use more than " + Clib::tostring( MAX_NUMER_REALMS ) + " realms" );

    POLLOG_INFO << "Loading Realm " << realm_name << ".\n";
    Tools::Timer<> timer;
    temprealm = new Realms::Realm( realm_name, dir_entry.path().u8string() );
    POLLOG_INFO << "Completed in " << timer.ellapsed() << " ms.\n";
    gamestate.Realms.push_back( temprealm );
    ++realm_counter;

    // To-Fix - Nasty kludge assuming 'britannia' is the default realm
    // May want to make this configurable in later core releases.
    if ( realm_name == "britannia" )
      gamestate.main_realm = temprealm;
  }
  //  main_realm = new DummyRealm();
  gamestate.baserealm_count = realm_counter;
  gamestate.shadowrealm_count = 0;
  if ( realm_counter > 0 )
    return true;
  else
    return false;
}

Realms::Realm* find_realm( const std::string& name )
{
  for ( auto& realm : gamestate.Realms )
  {
    if ( realm->name() == name )
      return realm;
  }
  return nullptr;
}

bool defined_realm( const std::string& name )
{
  for ( const auto& realm : gamestate.Realms )
  {
    if ( realm->name() == name )
      return true;
  }
  return false;
}

void add_realm( const std::string& name, Realms::Realm* base )
{
  Realms::Realm* r = new Realms::Realm( name, base );
  r->shadowid = ++gamestate.shadowrealm_count;
  gamestate.shadowrealms_by_id[r->shadowid] = r;
  gamestate.Realms.push_back( r );
}

void remove_realm( const std::string& name )
{
  std::vector<Realms::Realm*>::iterator itr;
  for ( itr = gamestate.Realms.begin(); itr != gamestate.Realms.end(); ++itr )
  {
    if ( ( *itr )->name() == name )
    {
      gamestate.storage.on_delete_realm( *itr );
      gamestate.shadowrealms_by_id[( *itr )->shadowid] = nullptr;  // used inside the decaythread
      delete *itr;
      gamestate.Realms.erase( itr );
      break;
    }
  }
}
}  // namespace Core
}  // namespace Pol
