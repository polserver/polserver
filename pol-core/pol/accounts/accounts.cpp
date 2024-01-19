/** @file
 *
 * @par History
 * - 2005/05/24 Shinigami: added delete_account
 * - 2005/05/25 Shinigami: added duplicate_account
 * - 2007/06/17 Shinigami: added config.world_data_path
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 */


#include "accounts.h"

#include <iosfwd>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/clib.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/streamsaver.h"
#include "../../clib/timer.h"
#include "../../plib/systemstate.h"
#include "../globals/state.h"
#include "../globals/uvars.h"
#include "../polsig.h"
#include "account.h"

namespace Pol
{
namespace Accounts
{
void read_account_data()
{
  unsigned int naccounts = 0;
  static int num_until_dot = 1000;
  Tools::Timer<> timer;

  std::string accountsfile = Plib::systemstate.config.world_data_path + "accounts.txt";

  INFO_PRINT( "  {}:", accountsfile );
  stat( accountsfile.c_str(), &Plib::systemstate.accounts_txt_stat );

  {
    Clib::ConfigFile cf( accountsfile, "Account" );
    Clib::ConfigElem elem;

    while ( cf.read( elem ) )
    {
      if ( --num_until_dot == 0 )
      {
        INFO_PRINT( "." );
        num_until_dot = 1000;
      }
      Core::gamestate.accounts.push_back( Core::AccountRef( new Account( elem ) ) );
      naccounts++;
    }
  }

  if ( Plib::systemstate.accounts_txt_dirty )
  {
    write_account_data();
  }

  INFO_PRINTLN( " {} elements in {} ms.", naccounts, timer.ellapsed() );
}

void write_account_data()
{
  std::string accountstxtfile = Plib::systemstate.config.world_data_path + "accounts.txt";
  std::string accountsbakfile = Plib::systemstate.config.world_data_path + "accounts.bak";
  std::string accountsndtfile = Plib::systemstate.config.world_data_path + "accounts.ndt";
  const char* accountstxtfile_c = accountstxtfile.c_str();
  const char* accountsbakfile_c = accountsbakfile.c_str();
  const char* accountsndtfile_c = accountsndtfile.c_str();

  unlink( accountsbakfile_c );
  unlink( accountsndtfile_c );

  bool result( true );
  try
  {
    std::ofstream ofs( accountsndtfile_c, std::ios::trunc | std::ios::out );
    Clib::StreamWriter sw( &ofs );
    for ( const auto& account : Core::gamestate.accounts )
    {
      Account* acct = account.get();
      acct->writeto( sw );
    }
  }
  catch ( ... )
  {
    POLLOG_ERRORLN( "failed to store accounts!" );
    result = false;
    Clib::force_backtrace();
  }
  if ( !result )
    return;
  rename( accountstxtfile_c, accountsbakfile_c );
  rename( accountsndtfile_c, accountstxtfile_c );

  struct stat newst;
  stat( accountstxtfile_c, &newst );
  memcpy( &Plib::systemstate.accounts_txt_stat, &newst,
          sizeof Plib::systemstate.accounts_txt_stat );
  Plib::systemstate.accounts_txt_dirty = false;
}

Account* create_new_account( const std::string& acctname, const std::string& password,
                             bool enabled )
{
  passert( !find_account( acctname.c_str() ) );

  Clib::ConfigElem elem;
  elem.add_prop( "name", acctname );
  elem.add_prop( "password", password );

  elem.add_prop( "enabled", ( (unsigned int)( enabled ? 1 : 0 ) ) );
  auto acct = new Account( elem );
  Core::gamestate.accounts.push_back( Core::AccountRef( acct ) );
  if ( Plib::systemstate.config.account_save == -1 )
    write_account_data();
  else
    Plib::systemstate.accounts_txt_dirty = true;
  return acct;
}

Account* duplicate_account( const std::string& oldacctname, const std::string& newacctname )
{
  passert( !find_account( newacctname.c_str() ) );

  Account* oldacct = find_account( oldacctname.c_str() );
  if ( oldacct != nullptr )
  {
    Clib::ConfigElem elem;
    oldacct->writeto( elem );
    elem.remove_string( "name" );
    elem.add_prop( "name", newacctname );

    auto acct = new Account( elem );
    Core::gamestate.accounts.push_back( Core::AccountRef( acct ) );
    if ( Plib::systemstate.config.account_save == -1 )
      write_account_data();
    else
      Plib::systemstate.accounts_txt_dirty = true;
    return acct;
  }
  return nullptr;
}

Account* find_account( const char* acctname )
{
  for ( auto& account : Core::gamestate.accounts )
  {
    if ( stricmp( account->name(), acctname ) == 0 )
    {
      return account.get();
    }
  }
  return nullptr;
}

int delete_account( const char* acctname )
{
  for ( auto itr = Core::gamestate.accounts.begin(), end = Core::gamestate.accounts.end();
        itr != end; ++itr )
  {
    Account* account = ( *itr ).get();
    if ( stricmp( account->name(), acctname ) == 0 )
    {
      if ( account->numchars() == 0 )
      {
        Core::gamestate.accounts.erase( itr );
        if ( Plib::systemstate.config.account_save == -1 )
          write_account_data();
        else
          Plib::systemstate.accounts_txt_dirty = true;
        return 1;
      }
      else
        return -1;
    }
  }
  return -2;
}

void reread_account( Clib::ConfigElem& elem )
{
  std::string name = elem.remove_string( "NAME" );
  Account* existing = find_account( name.c_str() );
  if ( existing != nullptr )
  {
    existing->readfrom( elem );
  }
  else
  {
    elem.add_prop( "NAME", name );
    Core::gamestate.accounts.push_back( Core::AccountRef( new Account( elem ) ) );
  }
}

void reload_account_data( void )
{
  THREAD_CHECKPOINT( tasks, 500 );
  try
  {
    std::string accountsfile = Plib::systemstate.config.world_data_path + "accounts.txt";

    struct stat newst;
    stat( accountsfile.c_str(), &newst );
    if ( ( newst.st_mtime != Plib::systemstate.accounts_txt_stat.st_mtime ) &&
         ( newst.st_mtime < time( nullptr ) - 10 ) )
    {
      INFO_PRINT( "Reloading accounts.txt..." );
      memcpy( &Plib::systemstate.accounts_txt_stat, &newst,
              sizeof Plib::systemstate.accounts_txt_stat );

      {
        Clib::ConfigFile cf( accountsfile, "Account" );
        Clib::ConfigElem elem;
        while ( cf.read( elem ) )
        {
          reread_account( elem );
        }
        INFO_PRINTLN( "Done!" );
      }
      if ( Plib::systemstate.accounts_txt_dirty )
      {
        write_account_data();
      }
    }
  }
  catch ( ... )
  {
    INFO_PRINTLN( "Error reading accounts.txt!" );
  }
  THREAD_CHECKPOINT( tasks, 599 );
}

void write_account_data_task( void )
{
  if ( Plib::systemstate.accounts_txt_dirty )
    write_account_data();
}
}  // namespace Accounts
}  // namespace Pol
