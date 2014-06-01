/*
History
=======

2005/03/01 Shinigami: added MAX_NUMER_REALMS check to prevent core crash (see MSGBF_SUB18_ENABLE_MAP_DIFFS)
2008/12/17 MuadDub:   Added check when loading Realms for no realms existing via int counter.
2009/08/25 Shinigami: STLport-5.2.1 fix: shadowrealm_count definition fixed

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "../clib/dirlist.h"
#include "../clib/passert.h"
#include "../clib/strutil.h"
#include "../plib/realm.h"
#include "uofile.h"
#include "los.h"
#include "polcfg.h"
#include "realms.h"
#include "storage.h"
#include "../clib/timer.h"
#include "../plib/mapserver.h"
#include "../clib/logfacility.h"

namespace Pol {
  namespace Core {
	Plib::Realm* main_realm = NULL;
	vector<Plib::Realm*>* Realms = new vector<Plib::Realm*>();
	std::map<int, Plib::Realm*> shadowrealms_by_id;
	unsigned int baserealm_count = 0;
	unsigned int shadowrealm_count = 0;

	bool load_realms()
	{
	  //string realm_dir = "realm/";
	  Plib::Realm* temprealm;
	  int realm_counter = 0;
	  for ( Clib::DirList dl( config.realm_data_path.c_str() ); !dl.at_end(); dl.next() )
	  {
		string realm_name = dl.name();
		if ( realm_name[0] == '.' )
		  continue;

		passert_r( Realms->size() < MAX_NUMER_REALMS,
				   "You can't use more than " + Clib::decint( MAX_NUMER_REALMS ) + " realms" );

        POLLOG_INFO << "Loading Realm " << realm_name << ".\n";
		Tools::Timer<> timer;
		temprealm = new Plib::Realm( realm_name, config.realm_data_path + realm_name );
        POLLOG_INFO << "Completed in " << timer.ellapsed( ) << " ms.\n";
		Realms->push_back( temprealm );
		++realm_counter;

		//To-Fix - Nasty kludge assuming 'britannia' is the default realm
		//May want to make this configurable in later core releases.
		if ( realm_name == string( "britannia" ) )
		  main_realm = temprealm;
	  }
	  //	main_realm = new DummyRealm();
	  baserealm_count = realm_counter;
	  shadowrealm_count = 0;
	  if ( realm_counter > 0 )
		return true;
	  else
		return false;
	}

	Plib::Realm* find_realm( const string& name )
	{
	  for ( auto &realm : *Realms )
	  {
		if ( realm->name() == name )
		  return realm;
	  }
	  return NULL;
	}

	bool defined_realm( const string& name )
	{
	  for ( const auto &realm : *Realms )
	  {
		if ( realm->name() == name )
		  return true;
	  }
	  return false;
	}

	void add_realm( const string& name, Plib::Realm* base )
	{
	  Plib::Realm* r = new Plib::Realm( name, base );
	  r->shadowid = ++shadowrealm_count;
	  shadowrealms_by_id[r->shadowid] = r;
	  Realms->push_back( r );
	}

	void remove_realm( const string& name )
	{
	  vector<Plib::Realm*>::iterator itr;
	  for ( itr = Realms->begin(); itr != Realms->end(); ++itr )
	  {
		if ( ( *itr )->name() == name )
		{
		  storage.on_delete_realm( *itr );
		  shadowrealms_by_id[( *itr )->shadowid] = NULL; // used inside the decaythread
		  delete *itr;
		  Realms->erase( itr );
		  break;
		}
	  }
	}
  }
}

