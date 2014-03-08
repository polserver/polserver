/*
History
=======
2005/05/24 Shinigami: added delete_account
2005/05/25 Shinigami: added duplicate_account
2007/06/17 Shinigami: added config.world_data_path
2009/09/03 MuadDib:   Relocation of account related cpp/h

Notes
=======

*/

#include "../../clib/stl_inc.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "../../clib/cfgfile.h"
#include "../../clib/cfgelem.h"
#include "../../clib/fileutil.h"
#include "../../clib/passert.h"
#include "../../clib/timer.h"
#include "../../clib/logfacility.h"

#include "account.h"
#include "accounts.h"
#include "../polcfg.h"
#include "../polsig.h"
#include "../schedule.h"
#include "../uvars.h"
namespace Pol {
  namespace Accounts {
	struct stat accounts_txt_stat;
	bool accounts_txt_dirty = false;
	void read_account_data()
	{
	  unsigned int naccounts = 0;
	  static int num_until_dot = 1000;
	  Tools::Timer<> timer;

	  string accountsfile = Core::config.world_data_path + "accounts.txt";

	  INFO_PRINT << "  " << accountsfile << ":";
	  stat( accountsfile.c_str(), &accounts_txt_stat );

	  {
		Clib::ConfigFile cf( accountsfile, "Account" );
		Clib::ConfigElem elem;

		while ( cf.read( elem ) )
		{
		  if ( --num_until_dot == 0 )
		  {
            INFO_PRINT << ".";
			num_until_dot = 1000;
		  }
		  Core::accounts.push_back( Core::AccountRef( new Account( elem ) ) );
		  naccounts++;
		}
	  }

	  if ( accounts_txt_dirty )
	  {
		write_account_data();
	  }

      INFO_PRINT << " " << naccounts << " elements in " << timer.ellapsed( ) << " ms.\n";
	}

	void write_account_data()
	{
	  string accountstxtfile = Core::config.world_data_path + "accounts.txt";
	  string accountsbakfile = Core::config.world_data_path + "accounts.bak";
	  string accountsndtfile = Core::config.world_data_path + "accounts.ndt";
	  const char *accountstxtfile_c = accountstxtfile.c_str();
	  const char *accountsbakfile_c = accountsbakfile.c_str();
	  const char *accountsndtfile_c = accountsndtfile.c_str();

	  unlink( accountsbakfile_c );
	  unlink( accountsndtfile_c );

      try
      {
        ofstream ofs( accountsndtfile_c, std::ios::trunc | ios::out );
        Clib::ThreadedOFStreamWriter sw( &ofs );
        for ( const auto &account : Core::accounts )
        {
          Account* acct = account.get();
          acct->writeto( sw );
        }
      }
      catch ( ... )
      {
        POLLOG_ERROR << "failed to store accounts!\n";
        Clib::force_backtrace();
      }

	  rename( accountstxtfile_c, accountsbakfile_c );
	  rename( accountsndtfile_c, accountstxtfile_c );

	  struct stat newst;
	  stat( accountstxtfile_c, &newst );
	  memcpy( &accounts_txt_stat, &newst, sizeof accounts_txt_stat );
	  accounts_txt_dirty = false;
	}

	Account* create_new_account( const string& acctname, const string& password, bool enabled )
	{
	  passert( !find_account( acctname.c_str() ) );

	  Clib::ConfigElem elem;
	  elem.add_prop( "name", acctname.c_str() );
	  elem.add_prop( "password", password.c_str() );

	  elem.add_prop( "enabled", ( (unsigned int)( enabled ? 1 : 0 ) ) );
	  auto acct = new Account( elem );
	  Core::accounts.push_back( Core::AccountRef( acct ) );
	  if ( Core::config.account_save == -1 )
		write_account_data();
	  else
		accounts_txt_dirty = true;
	  return acct;
	}

	Account* duplicate_account( const string& oldacctname, const string& newacctname )
	{
	  passert( !find_account( newacctname.c_str() ) );

	  Account* oldacct = find_account( oldacctname.c_str() );
	  if ( oldacct != NULL )
	  {
		Clib::ConfigElem elem;
		oldacct->writeto( elem );
		elem.remove_string( "name" );
		elem.add_prop( "name", newacctname.c_str() );

		auto acct = new Account( elem );
		Core::accounts.push_back( Core::AccountRef( acct ) );
		if ( Core::config.account_save == -1 )
		  write_account_data();
		else
		  accounts_txt_dirty = true;
		return acct;
	  }
	  return NULL;
	}

	Account* find_account( const char* acctname )
	{
	  for ( auto &account : Core::accounts )
	  {
		if ( stricmp( account->name(), acctname ) == 0 )
		{
		  return account.get();
		}
	  }
	  return NULL;
	}

	int delete_account( const char* acctname )
	{
	  for ( auto itr = Core::accounts.begin( ), end = Core::accounts.end( ); itr != end; ++itr )
	  {
		Account* account = ( *itr ).get();
		if ( stricmp( account->name(), acctname ) == 0 )
		{
		  if ( account->numchars() == 0 )
		  {
			Core::accounts.erase( itr );
			if ( Core::config.account_save == -1 )
			  write_account_data();
			else
			  accounts_txt_dirty = true;
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
	  string name = elem.remove_string( "NAME" );
	  Account* existing = find_account( name.c_str() );
	  if ( existing != NULL )
	  {
		existing->readfrom( elem );
	  }
	  else
	  {
		elem.add_prop( "NAME", name.c_str() );
		Core::accounts.push_back( Core::AccountRef( new Account( elem ) ) );
	  }
	}

	void reload_account_data( void )
	{
	  THREAD_CHECKPOINT( tasks, 500 );
	  try
	  {
		string accountsfile = Core::config.world_data_path + "accounts.txt";

		struct stat newst;
		stat( accountsfile.c_str(), &newst );
		if ( ( newst.st_mtime != accounts_txt_stat.st_mtime ) &&
			 ( newst.st_mtime < time( NULL ) - 10 ) )
		{
		  INFO_PRINT << "Reloading accounts.txt...";
		  memcpy( &accounts_txt_stat, &newst, sizeof accounts_txt_stat );

		  {
			Clib::ConfigFile cf( accountsfile, "Account" );
			Clib::ConfigElem elem;
			while ( cf.read( elem ) )
			{
			  reread_account( elem );
			}
            INFO_PRINT << "Done!\n";
		  }
		  if ( accounts_txt_dirty )
		  {
			write_account_data();
		  }
		}
	  }
	  catch ( ... )
	  {
        INFO_PRINT << "Error reading accounts.txt!\n";
	  }
	  THREAD_CHECKPOINT( tasks, 599 );
	}

	void write_account_data_task( void )
	{
	  if ( accounts_txt_dirty )
		write_account_data();
	}
	
  }
}