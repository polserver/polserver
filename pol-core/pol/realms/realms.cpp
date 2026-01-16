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


namespace Pol::Core
{
namespace fs = std::filesystem;
bool load_realms()
{
  Realms::Realm* temprealm;
  std::error_code ec;
  for ( const auto& dir_entry :
        fs::directory_iterator( Plib::systemstate.config.realm_data_path, ec ) )
  {
    if ( !dir_entry.is_directory() )
      continue;
    if ( auto fn = dir_entry.path().filename().string(); !fn.empty() && *fn.begin() == '.' )
      continue;
    const auto realm_name = dir_entry.path().stem().string();

    passert_r( gamestate.Realms.size() < MAX_NUMER_REALMS,
               "You can't use more than " + Clib::tostring( MAX_NUMER_REALMS ) + " realms" );

    POLLOG_INFOLN( "Loading Realm {}.", realm_name );
    Tools::Timer<> timer;
    temprealm = new Realms::Realm( realm_name, dir_entry.path().string() );
    POLLOG_INFOLN( "Completed in {} ms.", timer.ellapsed() );
    gamestate.Realms.push_back( temprealm );

    // To-Fix - Nasty kludge assuming 'britannia' is the default realm
    // May want to make this configurable in later core releases.
    if ( realm_name == "britannia" )
      gamestate.main_realm = temprealm;
  }
  return !gamestate.Realms.empty();
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

void add_realm( const std::string& name, Realms::Realm* base, bool has_decay )
{
  Realms::Realm* r = new Realms::Realm( name, base );
  r->has_decay = has_decay;
  gamestate.Realms.push_back( r );
  if ( r->has_decay )
  {
    gamestate.decay.after_realms_size_changed();
  }
}

void remove_realm( const std::string& name )
{
  std::vector<Realms::Realm*>::iterator itr;
  for ( itr = gamestate.Realms.begin(); itr != gamestate.Realms.end(); ++itr )
  {
    if ( ( *itr )->name() == name )
    {
      gamestate.storage.on_delete_realm( *itr );
      gamestate.decay.on_delete_realm( *itr );
      delete *itr;
      gamestate.Realms.erase( itr );
      gamestate.decay.after_realms_size_changed();
      break;
    }
  }
}
}  // namespace Pol::Core
