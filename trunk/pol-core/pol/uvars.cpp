/*
History
=======
2005/01/23 Shinigami: cleanup_vars - Tokuno MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)
2008/02/09 Shinigami: removed hard coded MAX_CHARS from cleanup_vars()
2009/01/18 Nando:     Realms was not being freed when exiting POL
                      vitals too. Multidefs too. Oh well...
                      Moved objecthash.ClearCharacterAccountReferences() out of the realms' loop
                      Added clean_multidefs()
2009/09/03 MuadDib:   Relocation of account related cpp/h
                      Changes for multi related source file relocation

Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif


#include "../clib/cfgelem.h"
#include "../clib/clib.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"
#include "../clib/cfgfile.h"
#include "../clib/logfacility.h"

#include "accounts/account.h"
#include "mobile/attribute.h"
#include "multi/boat.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "cmdlevel.h"
#include "gprops.h"
#include "guardrgn.h"
#include "item/item.h"
#include "miscrgn.h"
#include "musicrgn.h"
#include "multi/multi.h"
#include "objecthash.h"
#include "network/packethooks.h"
#include "polcfg.h"
#include "realms.h"
#include "servdesc.h"
#include "spells.h"
#include "startloc.h"
#include "storage.h"
#include "ufunc.h"
#include "uoskills.h"
#include "uvars.h"
#include "uworld.h"
#include "watch.h"

#include "vital.h"
#include "multi/multidef.h"

namespace Pol {
  namespace Core {
    AccountsVector accounts;
	Clients clients;
	//Clients pending_clients;
	Servers servers;
	StartingLocations startlocations;

	Items::UWeapon* wrestling_weapon;
	bool pol_startup;

	Watch watch;
	PropertyList global_properties;

	void clean_resources();

	void cleanup_vars()
	{
      for (auto &client : clients)
      {
        Network::Client::Delete( client );
      }
	  clients.clear();

	  //dave added 9/27/03: accounts and player characters have a mutual reference that prevents them getting cleaned up
	  //  properly. So clear the references now.
      for ( auto &account : accounts )
      {
        for ( int i = 0; i < config.character_slots; i++ )
          account->clear_character( i );
      }

	  for ( auto &realm : *Realms)
	  {
		unsigned wgridx = realm->width() / WGRID_SIZE;
		unsigned wgridy = realm->height() / WGRID_SIZE;

		// Tokuno-Fix
		if ( wgridx * WGRID_SIZE < realm->width() )
		  wgridx++;
		if ( wgridy * WGRID_SIZE < realm->height() )
		  wgridy++;

		for ( unsigned wx = 0; wx < wgridx; ++wx )
		{
          for ( unsigned wy = 0; wy < wgridy; ++wy )
          {
            for ( auto &item : realm->zone[wx][wy].items )
            {
              world_delete( item );
            }
            realm->zone[wx][wy].items.clear();
          }
		}

		for ( unsigned wx = 0; wx < wgridx; ++wx )
		{
          for ( unsigned wy = 0; wy < wgridy; ++wy )
          {
            for ( auto &chr : realm->zone[wx][wy].characters )
            {
              chr->acct.clear(); //dave added 9/27/03, see above comment re: mutual references
              world_delete( chr );
            }
            realm->zone[wx][wy].characters.clear();
            for ( auto &chr : realm->zone[wx][wy].npcs )
            {
              chr->acct.clear(); //dave added 9/27/03, see above comment re: mutual references
              world_delete( chr );
            }
            realm->zone[wx][wy].npcs.clear();
          }
		}

		for ( unsigned wx = 0; wx < wgridx; ++wx )
		{
		  for ( unsigned wy = 0; wy < wgridy; ++wy )
		  {
            for ( auto &multi : realm->zone[wx][wy].multis )
			{
			  world_delete( multi );
			}
            realm->zone[wx][wy].multis.clear();
		  }
		}
	  }

	  // dave renamed this 9/27/03, so we only have to traverse the objhash once, to clear out account references and delete.
	  // and Nando placed it outside the Realms' loop in 2009-01-18. 
	  objecthash.ClearCharacterAccountReferences();

	  accounts.clear();
	  Clib::delete_all( servers );
	  Clib::delete_all( startlocations );

	  storage.clear();

	  //RegionGroup cleanup _before_ Realm cleanup
	  delete justicedef;
	  delete lightdef;
	  delete nocastdef;
	  delete weatherdef;
	  delete musicdef;
	  clean_resources();

	  Clib::delete_all( *Realms ); // Was leaking... btw, should Realms really be
	  delete Realms;         // a pointer to a vector<Realm*>? What's the need? (Nando -- 2009-18-01)
	  Realms = NULL;

	  //delete_all(vitals);
	  clean_vitals();
	  Multi::clean_multidefs();
	  Multi::clean_boatshapes();

	  Mobile::clean_attributes();
	  cmdlevels2.clear();
	  clean_spells();
	  clean_skills();
	  Network::clean_packethooks();
	}



	void set_watch_vars()
	{
	  Clib::ConfigFile cf;
	  Clib::ConfigElem elem;
	  if ( Clib::FileExists( "config/watch.cfg" ) )
	  {
		cf.open( "config/watch.cfg" );
		cf.readraw( elem );
	  }
	  else if ( config.loglevel > 1 )
        INFO_PRINT << "File config/watch.cfg not found, skipping.\n";

	  watch.combat = elem.remove_bool( "COMBAT", false );
	  watch.profile_scripts = elem.remove_bool( "ProfileScripts", false );
	}
  }
}