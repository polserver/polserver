/** @file
 *
 * @par History
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "poltest.h"

#include "../clib/logfacility.h"
#include "../clib/passert.h"

#include "../plib/maptile.h"
#include "../plib/testenv.h"

#include "realms.h"

#include "item/item.h"
#include "los.h"
#include "mobile/npc.h"
#include "polcfg.h"
#include "globals/uvars.h"
#include "skilladv.h"
#include "dynproperties.h"
#include "realms/realm.h"

#include "uobject.h"
#include "../clib/timer.h"

namespace Pol {
  namespace Plib {
    //void pol_walk_test();
    //void pol_drop_test();
    //void pol_los_test();
    //void pol_test_multiwalk();
    //void display_test_results();
    //void create_test_environment( );
    void inc_failures();
    void inc_successes( );
  }
  namespace Multi {
    void read_multidefs( );
  }
  namespace Core {
    extern bool static_debug_on;

	extern Mobile::NPC* test_banker;
	extern Items::Item* test_guard_door;
	extern Mobile::NPC* test_water_elemental;
	extern Mobile::NPC* test_seaserpent;

	void map_test()
	{
	  Plib::MAPTILE_CELL cell = gamestate.main_realm->getmaptile( 1453, 1794 );
      INFO_PRINT << cell.landtile << " " << cell.z << "\n";
	}

	void test_skilladv( unsigned int raw, unsigned short base )
	{
      INFO_PRINT << "Raw " << raw << ", base " << base << ": ";
	  if ( raw_to_base( raw ) != base )
	  {
        INFO_PRINT << "raw_to_base failed!\n";
        Plib::inc_failures( );
	  }
	  if ( base_to_raw( base ) != raw )
	  {
        INFO_PRINT << "base_to_raw failed!\n";
        Plib::inc_failures( );
	  }
      INFO_PRINT << "Passed\n";
      Plib::inc_successes( );
	}

	void skilladv_test()
	{
      INFO_PRINT << "Skill advancement tests:\n";
	  test_skilladv( 20, 1 );
	  test_skilladv( 512, 25 );
	  test_skilladv( 1024, 50 );
	  test_skilladv( 2048, 100 );
	  test_skilladv( 3072, 150 );
	  test_skilladv( 4096, 200 );
	  test_skilladv( 5120, 225 );
	  test_skilladv( 6144, 250 );
	  test_skilladv( 7168, 275 );
	  test_skilladv( 8192, 300 );
	  test_skilladv( 16384, 400 );
	  test_skilladv( 32768, 500 );
	  test_skilladv( 65536, 600 );
	  test_skilladv( 131072, 700 );
	  test_skilladv( 262144, 800 );
	  test_skilladv( 524288, 900 );
	  test_skilladv( 1048576, 1000 );
	  test_skilladv( 2097152, 1100 );
	  test_skilladv( 4194304, 1200 );
	  test_skilladv( 8388608, 1300 );
	  test_skilladv( 16777216, 1400 );
	  test_skilladv( 33554432, 1500 );
	  test_skilladv( 67108864, 1600 );
	  test_skilladv( 134217728, 1700 );
	  test_skilladv( 268435456, 1800 );
	  test_skilladv( 536870912, 1900 );

	  for ( unsigned short base = 0; base < 2000; ++base )
	  {
        INFO_PRINT << "Base " << base << ": Raw=";
		unsigned int raw = base_to_raw( base );
        INFO_PRINT << raw << ": ";
		unsigned short calcbase = raw_to_base( raw );
		if ( calcbase != base )
		{
          INFO_PRINT << "Failed (" << calcbase << ")\n";
          Plib::inc_failures( );
		}
		else
		{
          INFO_PRINT << "Passed\n";
          Plib::inc_successes( );
		}
	  }

	  for ( unsigned int raw = 0; raw < 2500; ++raw )
	  {
        INFO_PRINT << "Raw " << raw << ": Base=";
		unsigned short base = raw_to_base( raw );
        INFO_PRINT << base << ": ";
		unsigned int calcraw = base_to_raw( base );
		if ( calcraw != raw )
		{
          INFO_PRINT << "Failed (" << calcraw << ")\n";
          Plib::inc_failures( );
		}
		else
		{
          INFO_PRINT << "Passed\n";
          Plib::inc_successes( );
		}
	  }
	}

	void run_pol_tests()
	{
      class Test : public DynamicPropsHolder
      {
      public:
            DYN_PROPERTY(armod, s16, PROP_AR_MOD, 0);
            DYN_PROPERTY(max_items, u32, PROP_MAX_ITEMS_MOD, 0);
            DYN_PROPERTY(itemname, std::string, PROP_NAME_SUFFIX, "");
      };
      Test h;
      INFO_PRINT << "size " << h.estimateSizeDynProps() << "\n";
      INFO_PRINT << "ar " << h.armod() << " " << h.has_armod() << "\n";
      h.armod(10);
      INFO_PRINT << "ar " << h.armod()<<" " << h.has_armod() << "\n";
      h.armod(0);
      INFO_PRINT << "ar " << h.armod()<<" " << h.has_armod() << "\n";
      INFO_PRINT << "size " << h.estimateSizeDynProps() << "\n";
      INFO_PRINT << "name " << h.itemname() << " " << h.has_itemname() << "\n";
      h.itemname("hello world");
      INFO_PRINT << "name " << h.itemname() << " " << h.has_itemname() << "\n";
      h.itemname("");
      INFO_PRINT << "name " << h.itemname() << " " << h.has_itemname() << "\n";
      INFO_PRINT << "size " << h.estimateSizeDynProps() << "\n";
      return;
   //   Plib::create_test_environment();

	  //Plib::pol_drop_test();
   //   Plib::pol_walk_test( );
   //   Plib::pol_los_test( );
   //   Plib::pol_test_multiwalk( );

   //   Plib::display_test_results( );
	}


	/* TO ADD:
	  (,5909,1395,30)  (walking through the floor?)
	  (2193,1408,-2) -- no LOS to (2191, 1412, -2) ?
	  */


#ifdef POLTEST
	Item* Item::create( u32 objtype, u32 serial )
	{
	  passert( objtype <= config.max_tile_id );
	  return new Item(objtype, CLASS_ITEM);
	}

	UMulti* UMulti::create(  u32 objtype, u32 serial )
	{
	  UMulti* multi = NULL;
	  if ( objtype <= config.max_tile_id)
		return NULL;

	  if (defs[objtype] != NULL)
	  {
		multi = new UMulti( objtype );
	  }
	  else
	  {
        ERROR_PRINT << "Tried to create multi " << objtype << " but no definition exists in MULTI.MUL\n";
		return NULL;
	  }

	  /*
		  if (serial)
		  multi->serial = UseItemSerialNumber( serial );
		  else
		  multi->serial = GetNewItemSerialNumber();

		  multi->serial_ext = ctBEu32( multi->serial );
		  */

	  return multi;
	}


	Items items;
	Multis multis;

	int main( int arc, char *argv[] )
	{
	  Plib::systemstate.config.uo_datafile_root = "C:/Games/AoS2D/";
      INFO_PRINT << "Opening UO data files..\n";
	  open_uo_data_files();
	  read_uo_data();
	  Multi::read_multidefs();

	  run_pol_tests();
	  clear_tiledata();
	  return 0;
	}
#endif
  }
}
