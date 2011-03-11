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
#include <time.h>

#include "../../clib/cfgfile.h"
#include "../../clib/cfgelem.h"
#include "../../clib/fileutil.h"
#include "../../clib/passert.h"
#include "../../clib/wallclock.h"

#include "account.h"
#include "accounts.h"
#include "../polcfg.h"
#include "../polsig.h"
#include "../schedule.h"
#include "../uvars.h"

struct stat accounts_txt_stat;
bool accounts_txt_dirty = false;
void read_account_data()
{
	unsigned int naccounts = 0;
    static int num_until_dot = 1000;
	wallclock_t start = wallclock();
	
	string accountsfile = config.world_data_path + "accounts.txt";
	
	cout << "  " << accountsfile << ":";
    stat( accountsfile.c_str(), &accounts_txt_stat );

	{
		ConfigFile cf( accountsfile, "Account" );
		ConfigElem elem;

		while (cf.read( elem ))
		{
            if (--num_until_dot == 0)
            {
                cout << ".";
                num_until_dot = 1000;
            }
	        accounts.push_back( AccountRef(new Account( elem )) );
			naccounts++;
		}
	}
	
	if(accounts_txt_dirty)
	{
		write_account_data();
	}
	wallclock_t end = wallclock();
    int ms = wallclock_diff_ms( start, end );

    cout << " " << naccounts << " elements in " << ms << " ms." << std::endl;
}

void write_account_data()
{
	string accountstxtfile = config.world_data_path + "accounts.txt";
	string accountsbakfile = config.world_data_path + "accounts.bak";
	string accountsndtfile = config.world_data_path + "accounts.ndt";

	RemoveFile( accountsbakfile );
    RemoveFile( accountsndtfile );
    
    {
        ofstream ofs( accountsndtfile.c_str(), std::ios::trunc|ios::out );
        for( Accounts::iterator itr = accounts.begin(); itr != accounts.end(); ++itr )
        {
            Account* acct = (*itr).get();
            acct->writeto( ofs );
        }
    }
    
    rename( accountstxtfile.c_str(), accountsbakfile.c_str() );
    rename( accountsndtfile.c_str(), accountstxtfile.c_str() );

	struct stat newst;
    stat( accountstxtfile.c_str(), &newst );
    memcpy( &accounts_txt_stat, &newst, sizeof accounts_txt_stat );
	accounts_txt_dirty = false;
}

Account* create_new_account( const string& acctname, const string& password, bool enabled )
{
    passert( !find_account( acctname.c_str() ) );

    ConfigElem elem;
    elem.add_prop( "name", acctname.c_str() );	
	elem.add_prop( "password", password.c_str() );
		
    elem.add_prop( "enabled", ((unsigned int)(enabled?1:0)) );
    Account* acct = new Account( elem );
    accounts.push_back( AccountRef(acct) );
	if (config.account_save == -1)
		write_account_data();
	else
		accounts_txt_dirty = true;
    return acct;
}

Account* duplicate_account( const string& oldacctname, const string& newacctname )
{
    passert( !find_account( newacctname.c_str() ) );
    
    Account* oldacct = find_account( oldacctname.c_str() );
    if (oldacct != NULL)
    {
        ConfigElem elem;
        oldacct->writeto( elem );
        elem.remove_string( "name" );
        elem.add_prop( "name", newacctname.c_str() );	
        
        Account* acct = new Account( elem );
        accounts.push_back( AccountRef(acct) );
		if (config.account_save == -1)
			write_account_data();
		else
			accounts_txt_dirty = true;
        return acct;
    }
    return NULL;
}

Account* find_account( const char* acctname )
{
  	for( unsigned idx = 0; idx < accounts.size(); idx++ )
	{
		if (stricmp( accounts[idx]->name(), acctname ) == 0)
		{
			return accounts[idx].get();
		}
	}
    return NULL;
}

int delete_account( const char* acctname )
{
    for( Accounts::iterator itr = accounts.begin(), end = accounts.end(); itr != end; ++itr )
    {
        Account* account = (*itr).get();
        if (stricmp( account->name(), acctname ) == 0)
        {
            if (account->numchars() == 0)
            {
                accounts.erase(itr);
				if (config.account_save == -1)
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

void reread_account( ConfigElem& elem )
{
    string name = elem.remove_string( "NAME" );
    Account* existing = find_account( name.c_str() );
    if (existing != NULL)
    {
        existing->readfrom( elem );
    }
    else
    {
        elem.add_prop( "NAME", name.c_str() );
        accounts.push_back( AccountRef(new Account( elem )) );
    }
}

void reload_account_data(void)
{
    THREAD_CHECKPOINT( tasks, 500 );
    try
    {
		string accountsfile = config.world_data_path + "accounts.txt";

        struct stat newst;
        stat( accountsfile.c_str(), &newst );
        if ((newst.st_mtime != accounts_txt_stat.st_mtime) &&
            (newst.st_mtime < time(NULL)-10))
        {
            cout << "Reloading accounts.txt...";
            memcpy( &accounts_txt_stat, &newst, sizeof accounts_txt_stat );
            
			{
				ConfigFile cf( accountsfile, "Account" );
				ConfigElem elem;
				while (cf.read( elem ))
				{
					reread_account( elem );
				}
				cout << "Done!" << endl;
			}
			if(accounts_txt_dirty)
			{
				write_account_data();
			}
        }
    }
    catch( ... )
    {
        cout << "Error reading accounts.txt!" << endl;
    }
    THREAD_CHECKPOINT( tasks, 599 );
}
PeriodicTask reload_accounts_task( reload_account_data, 30, "LOADACCT" );

void write_account_data_task(void)
{
	if(accounts_txt_dirty)
		write_account_data();
}
PeriodicTask write_account_task( write_account_data_task, 60, "WRITEACCT" );
